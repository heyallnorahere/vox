#include "voxpch.h"
#include "vox/platform/vulkan/VulkanBase.h"
#include "vox/platform/vulkan/VulkanDevice.h"

#include "vox/platform/vulkan/VulkanRenderer.h"

namespace vox {
    bool VulkanDevice::ConvertQueueType(GraphicsQueueType::QueueType queueType,
                                        VkQueueFlagBits& flag) {
        ZoneScopedVulkan;
        switch (queueType) {
        case GraphicsQueueType::Graphics:
            flag = VK_QUEUE_GRAPHICS_BIT;
            break;
        case GraphicsQueueType::Transfer:
            flag = VK_QUEUE_TRANSFER_BIT;
            break;
        case GraphicsQueueType::Compute:
            flag = VK_QUEUE_COMPUTE_BIT;
            break;
        default:
            return false;
        }

        return true;
    }

    bool VulkanDevice::FindQueueFamilies(
        const std::unordered_set<GraphicsQueueType::QueueType>& requested,
        std::unordered_map<GraphicsQueueType::QueueType, uint32_t>& found,
        VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        ZoneScopedVulkan;

        uint32_t familyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

        // little bit of spaghetti code - sorry
        found.clear();
        for (uint32_t i = 0; i < familyCount; i++) {
            const auto& properties = families[i];
            for (auto type : requested) {
                if (found.find(type) != found.end()) {
                    continue;
                }

                VkQueueFlagBits queueFlag;
                bool isQueue = false;
                if (!ConvertQueueType(type, queueFlag)) {
                    switch (type) {
                    case GraphicsQueueType::Present: // present support is not a flag bit - you need
                                                     // to query it with the surface
                        if (surface != VK_NULL_HANDLE) {
                            VkBool32 supported;
                            if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                                                     &supported) != VK_SUCCESS) {
                                supported = VK_FALSE;
                            }

                            if (supported != VK_FALSE) { // weird vulkan boolean
                                isQueue = true;
                            }
                        }

                        break;
                    }
                } else if ((properties.queueFlags & queueFlag) == queueFlag) {
                    isQueue = true;
                }

                if (isQueue) {
                    found[type] = i;
                }
            }

            if (found.size() == requested.size()) {
                return true;
            }
        }

        return false;
    }

    VulkanDevice::VulkanDevice(VkPhysicalDevice device, VkSurfaceKHR surface,
                               const std::vector<const char*>& requestedExtensions) {
        ZoneScopedVulkan;
        std::unordered_set<GraphicsQueueType::QueueType> requestedFamilies = {
            GraphicsQueueType::Graphics, GraphicsQueueType::Transfer, GraphicsQueueType::Compute
        };

        if (surface != VK_NULL_HANDLE) {
            requestedFamilies.insert(GraphicsQueueType::Present);
        }

        m_PhysicalDevice = device;
        std::unordered_map<GraphicsQueueType::QueueType, uint32_t> queueFamilies;
        if (!FindQueueFamilies(requestedFamilies, queueFamilies, m_PhysicalDevice, surface)) {
            spdlog::warn("Not all device queue families were found!");
        }

        std::unordered_set<uint32_t> createdQueues;
        for (const auto& [type, index] : queueFamilies) {
            createdQueues.insert(index);
        }

        float priority = 1.f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;

        for (uint32_t index : createdQueues) {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueCount = 1;
            queueInfo.queueFamilyIndex = index;
            queueInfo.pQueuePriorities = &priority;

            queueCreateInfo.push_back(queueInfo);
        }

        Renderer::Info rendererInfo;
        VulkanRenderer::GetRenderer()->Query(rendererInfo);
        uint32_t vulkanVersion =
            VK_MAKE_VERSION(rendererInfo.Major, rendererInfo.Minor, rendererInfo.Patch);

        VkPhysicalDeviceVulkan13Features features13{};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        VkPhysicalDeviceVulkan12Features features12{};
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.pNext = vulkanVersion >= VK_API_VERSION_1_3 ? &features13 : nullptr;

        VkPhysicalDeviceVulkan11Features features11{};
        features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        features11.pNext = vulkanVersion >= VK_API_VERSION_1_2 ? &features12 : nullptr;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &features11;

        bool useFeatures2 = vulkanVersion >= VK_API_VERSION_1_1;
        VkPhysicalDeviceFeatures features;

        if (useFeatures2) {
            vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);
        } else {
            vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &features);
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = useFeatures2 ? &features2 : nullptr;
        createInfo.pEnabledFeatures = useFeatures2 ? nullptr : &features;
        createInfo.enabledExtensionCount = (uint32_t)requestedExtensions.size();
        createInfo.ppEnabledExtensionNames = requestedExtensions.data();
        createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfo.size();
        createInfo.pQueueCreateInfos = queueCreateInfo.data();

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan device!");
        }

        std::unordered_map<uint32_t, GraphicsQueueType::Flags> familyUsage;
        for (const auto& [usage, family] : queueFamilies) {
            familyUsage[family] |= usage;
        }

        for (const auto& [family, usage] : familyUsage) {
            m_Queues[family] = std::make_unique<VulkanQueue>(m_Device, family, usage);
        }
    }

    VulkanDevice::~VulkanDevice() {
        m_Queues.clear();
        vkDestroyDevice(m_Device, nullptr);
    }

    bool VulkanDevice::HasQueue(GraphicsQueueType::Flags flags) {
        ZoneScopedVulkan;

        for (const auto& [usage, queue] : m_Queues) {
            if ((usage & flags) == flags) {
                return true;
            }
        }

        return false;
    }

    CommandQueue& VulkanDevice::GetQueue(GraphicsQueueType::Flags flags) {
        ZoneScopedVulkan;

        for (const auto& [usage, queue] : m_Queues) {
            if ((usage & flags) == flags) {
                return *queue;
            }
        }

        throw std::runtime_error("No such queue meets these requirements!");
    }
} // namespace vox