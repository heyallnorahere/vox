#pragma once

#include "vox/renderer/GraphicsDevice.h"

namespace vox {
    class VulkanDevice : public GraphicsDevice {
    public:
        static bool ConvertQueueType(GraphicsQueueType::QueueType queueType, VkQueueFlagBits& flag);
        static bool FindQueueFamilies(
            const std::unordered_set<GraphicsQueueType::QueueType>& requested,
            std::unordered_map<GraphicsQueueType::QueueType, uint32_t>& found,
            VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE);

        VulkanDevice(VkPhysicalDevice device,
                     const std::vector<const char*>& requestedExtensions,
                     const std::unordered_set<uint32_t>& requestedQueues);

        virtual ~VulkanDevice() override;

        VkDevice GetDevice() const { return m_Device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }

    private:
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;
        std::unordered_map<GraphicsQueueType::QueueType, uint32_t> m_QueueFamilies;
    };
} // namespace vox