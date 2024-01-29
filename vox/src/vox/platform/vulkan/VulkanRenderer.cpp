#include "voxpch.h"
#include "vox/platform/vulkan/VulkanBase.h"
#include "vox/platform/vulkan/VulkanRenderer.h"

#include "vox/core/Application.h"

namespace vox {
    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        ZoneScopedVulkan;

        std::optional<spdlog::level::level_enum> level;
        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = spdlog::level::warn;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = spdlog::level::err;
            break;
        }

        if (level.has_value()) {
            spdlog::log(level.value(), "Vulkan: {}", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    static uint32_t ScoreDevice(VkPhysicalDevice device,
                                const std::vector<VulkanExtension>& requestedExtensions) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        uint32_t extensionCount = 0;
        if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) !=
            VK_SUCCESS) {
            spdlog::info("Failed to enumerate extensions for device {} - skipping",
                         properties.deviceName);

            return 0;
        }

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

        uint32_t score = 0;
        if (features.geometryShader) {
            score += 8000;
        }

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 5000;
        }

        for (const auto& requested : requestedExtensions) {
            bool found = false;
            for (const auto& extension : extensions) {
                if (requested.Name == extension.extensionName) {
                    found = true;
                    break;
                }
            }

            if (found && !requested.Required) {
                score += 1000;
            } else if (requested.Required && !found) {
                return 0;
            }
        }

        return score;
    }

    static std::vector<VulkanExtension> s_InstanceExtensions = {
        { "VK_KHR_get_physical_device_properties2", false }, { "VK_EXT_debug_utils", false }
    };

    static std::vector<VulkanExtension> s_DeviceExtensions = {
        { "VK_KHR_swapchain", true }, { "VK_KHR_portability_subset", false }
    };

    static std::vector<VulkanExtension> s_InstanceLayers = { { "VK_LAYER_KHRONOS_validation",
                                                               false } };

    constexpr uint32_t VersionMajor = 1;
    constexpr uint32_t VersionMinor = 2;
    constexpr uint32_t VersionPatch = 0;

    static VulkanRenderer* s_Instance;
    VulkanRenderer* VulkanRenderer::GetRenderer() { return s_Instance; }

    VkPhysicalDevice VulkanRenderer::ChoosePhysicalDevice(VkInstance instance) {
        ZoneScopedVulkan;

        uint32_t deviceCount = 0;
        if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate physical devices!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        uint32_t selectedScore = 0;
        VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;

        for (VkPhysicalDevice device : devices) {
            uint32_t score = ScoreDevice(device, s_DeviceExtensions);
            if (score > selectedScore) {
                selectedScore = score;
                selectedDevice = device;
            }
        }

        return selectedDevice;
    }

    VulkanRenderer::VulkanRenderer() {
        ZoneScopedVulkan;
        spdlog::info("Initializing Vulkan renderer");

        if (s_Instance != nullptr) {
            throw std::runtime_error("A Vulkan renderer has already been created!");
        }

        s_Instance = this;
        if (volkInitialize() != VK_SUCCESS) {
            throw std::runtime_error("Failed to load Vulkan!");
        }

        CreateInstance();
        CreateDebugMessenger();

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        auto window = Application::Get().GetWindow();
        if (window) {
            surface = (VkSurfaceKHR)window->CreateVulkanSurface(m_Instance);
        }

        CreateDevice(surface);
        if (m_Device) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(m_Device->GetPhysicalDevice(), &properties);

            spdlog::info("Chose Vulkan device: {}", properties.deviceName);
        } else {
            throw std::runtime_error("No viable Vulkan device found!");
        }

        // temporary - we dont have a swapchain yet
        vkDestroySurfaceKHR(m_Instance, surface, nullptr);

        m_ProfilerContext = nullptr;

        spdlog::info("Vulkan renderer initialized");
    }

    VulkanRenderer::~VulkanRenderer() {
        ZoneScopedVulkan;

        m_Device.Reset();

        if (m_DebugMessenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanRenderer::Query(Info& info) {
        ZoneScopedVulkan;

        info.RendererAPI = API::Vulkan;
        info.Major = VersionMajor;
        info.Minor = VersionMinor;
        info.Patch = VersionPatch;
    }

    template <typename T>
    static void CompareVulkanExtensions(const std::vector<T>& availableExtensions,
                                        const std::vector<VulkanExtension>& requestedExtensions,
                                        std::vector<const char*>& result,
                                        std::string (*converter)(const T&)) {
        ZoneScopedVulkan;
        result.clear();

        std::vector<std::string> availableNames;
        for (const auto& extension : availableExtensions) {
            availableNames.push_back(converter(extension));
        }

        for (const auto& requested : requestedExtensions) {
            bool found = false;
            for (const auto& available : availableNames) {
                if (requested.Name == available) {
                    found = true;
                    break;
                }
            }

            if (found) {
                result.push_back(requested.Name.c_str());
            } else if (requested.Required) {
                throw std::runtime_error(requested.Name + " is not present!");
            }
        }
    }

    void VulkanRenderer::CreateInstance() {
        ZoneScopedVulkan;

        std::vector<VulkanExtension> requested(s_InstanceExtensions);
        std::vector<const char*> extensions, layers;

        {
            ZoneScopedNC("Parse extensions and layers", VulkanProfilerColor);

            {
                ZoneScopedNC("Query window surface extensions", VulkanProfilerColor);

                std::vector<std::string> windowExtensions;
                auto window = Application::Get().GetWindow();
                window->GetRequiredVulkanExtensions(windowExtensions);

                for (const auto& name : windowExtensions) {
                    requested.push_back({ name, true });
                }
            }

            std::vector<VkExtensionProperties> availableExtensions;
            std::vector<VkLayerProperties> availableLayers;

            {
                ZoneScopedNC("Query Vulkan for layers and extensions", VulkanProfilerColor);

                uint32_t count;
                if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) !=
                    VK_SUCCESS) {
                    throw std::runtime_error("Failed to enumerate instance extensions!");
                }

                availableExtensions.resize(count);
                vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

                if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to enumerate instance layers!");
                }

                availableLayers.resize(count);
                vkEnumerateInstanceLayerProperties(&count, availableLayers.data());
            }

            CompareVulkanExtensions<VkExtensionProperties>(
                availableExtensions, requested, extensions,
                [](const auto& extension) { return std::string(extension.extensionName); });

            CompareVulkanExtensions<VkLayerProperties>(
                availableLayers, s_InstanceLayers, layers,
                [](const auto& layer) { return std::string(layer.layerName); });
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_MAKE_VERSION(VersionMajor, VersionMinor, VersionPatch);
        appInfo.pApplicationName = appInfo.pEngineName = "Vox";
        appInfo.applicationVersion = appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledExtensionCount = (uint32_t)extensions.size();
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledLayerCount = (uint32_t)layers.size();

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }

        volkLoadInstance(m_Instance);
    }

    void VulkanRenderer::CreateDebugMessenger() {
        ZoneScopedVulkan;

        PFN_vkCreateDebugUtilsMessengerEXT createDebugMessenger = nullptr;
#ifdef VOX_DEBUG
        createDebugMessenger = vkCreateDebugUtilsMessengerEXT;
#endif

        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            m_DebugMessenger = VK_NULL_HANDLE;
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.pfnUserCallback = VulkanValidationCallback;
        createInfo.pUserData = this;

        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        if (createDebugMessenger(m_Instance, &createInfo, nullptr, &m_DebugMessenger) !=
            VK_SUCCESS) {
            m_DebugMessenger = VK_NULL_HANDLE;
            spdlog::error(
                "Failed to create Vulkan debug messenger - validation errors will not show");
        }
    }

    void VulkanRenderer::CreateDevice(VkSurfaceKHR surface) {
        ZoneScopedVulkan;

        VkPhysicalDevice selectedDevice = ChoosePhysicalDevice(m_Instance);
        if (selectedDevice == VK_NULL_HANDLE) {
            return;
        }

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(selectedDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(selectedDevice, nullptr, &extensionCount,
                                             availableExtensions.data());

        std::vector<const char*> extensions;
        CompareVulkanExtensions<VkExtensionProperties>(
            availableExtensions, s_DeviceExtensions, extensions,
            [](const auto& extension) { return std::string(extension.extensionName); });

        m_Device = Ref<VulkanDevice>::Create(selectedDevice, surface, extensions);
        volkLoadDevice(m_Device->GetDevice());
    }
} // namespace vox