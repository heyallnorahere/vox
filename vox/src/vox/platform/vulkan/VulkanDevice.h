#pragma once

#include "vox/renderer/GraphicsDevice.h"
#include "vox/platform/vulkan/VulkanQueue.h"

namespace vox {
    class VulkanDevice : public GraphicsDevice {
    public:
        static bool ConvertQueueType(GraphicsQueueType::QueueType queueType, VkQueueFlagBits& flag);
        static bool FindQueueFamilies(
            const std::unordered_set<GraphicsQueueType::QueueType>& requested,
            std::unordered_map<GraphicsQueueType::QueueType, uint32_t>& found,
            VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE);

        VulkanDevice(VkPhysicalDevice device, VkSurfaceKHR surface,
                     const std::vector<const char*>& requestedExtensions);

        virtual ~VulkanDevice() override;

        VkDevice GetDevice() const { return m_Device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }

        virtual bool HasQueue(GraphicsQueueType::Flags flags) override;
        virtual CommandQueue& GetQueue(GraphicsQueueType::Flags flags) override;

    private:
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;
        std::unordered_map<GraphicsQueueType::Flags, std::unique_ptr<VulkanQueue>> m_Queues;
    };
} // namespace vox