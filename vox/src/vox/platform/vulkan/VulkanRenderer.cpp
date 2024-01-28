#include "voxpch.h"
#include "vox/platform/vulkan/VulkanBase.h"
#include "vox/platform/vulkan/VulkanRenderer.h"

#include "vox/core/Application.h"

namespace vox {
    struct VulkanExtension {
        std::string Name;
        bool Required;
    };

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

        m_ProfilerContext = nullptr;

        spdlog::info("Vulkan renderer initialized");
    }

    VulkanRenderer::~VulkanRenderer() {
        ZoneScopedVulkan;

        if (m_DebugMessenger != nullptr) {
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
                const auto& window = Application::Get().GetWindow();
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
            m_DebugMessenger = nullptr;
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
            m_DebugMessenger = nullptr;
            spdlog::error(
                "Failed to create Vulkan debug messenger - validation errors will not show");
        }
    }
} // namespace vox