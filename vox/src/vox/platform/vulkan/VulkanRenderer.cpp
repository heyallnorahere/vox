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
        if (s_Instance != nullptr) {
            throw std::runtime_error("A Vulkan renderer has already been created!");
        }

        s_Instance = this;
        if (volkInitialize() != VK_SUCCESS) {
            throw std::runtime_error("Failed to load Vulkan!");
        }

        CreateInstance();

        m_ProfilerContext = nullptr;
    }

    VulkanRenderer::~VulkanRenderer() {
        ZoneScopedVulkan;

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

        std::vector<std::string> windowExtensions;
        const auto& window = Application::Get().GetWindow();
        window->GetRequiredVulkanExtensions(windowExtensions);

        std::vector<VulkanExtension> requested(s_InstanceExtensions);
        for (const auto& name : windowExtensions) {
            requested.push_back({ name, true });
        }

        uint32_t count;
        if (vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance extensions!");
        }

        std::vector<VkExtensionProperties> availableExtensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

        if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance layers!");
        }

        std::vector<VkLayerProperties> availableLayers(count);
        vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

        std::vector<const char*> extensions, layers;
        CompareVulkanExtensions<VkExtensionProperties>(
            availableExtensions, requested, extensions,
            [](const auto& extension) { return std::string(extension.extensionName); });

        CompareVulkanExtensions<VkLayerProperties>(
            availableLayers, s_InstanceLayers, layers,
            [](const auto& layer) { return std::string(layer.layerName); });

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
    }
} // namespace vox