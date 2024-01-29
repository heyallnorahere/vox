#include "voxpch.h"
#include "vox/platform/vulkan/VulkanBase.h"
#include "vox/platform/vulkan/VulkanQueue.h"

namespace vox {
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanQueue* queue) {
        ZoneScopedVulkan;

        m_Device = queue->GetDevice();
        m_Pool = queue->GetPool();
        m_Usage = queue->GetUsage();
        m_Recording = false;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = m_Pool;
        
        if (vkAllocateCommandBuffers(m_Device, &allocInfo, &m_Buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer!");
        }
    }

    VulkanCommandBuffer::~VulkanCommandBuffer() {
        ZoneScopedVulkan;

        End();
        vkFreeCommandBuffers(m_Device, m_Pool, 1, &m_Buffer);
    }

    void VulkanCommandBuffer::Begin() {
        ZoneScopedVulkan;
        if (m_Recording) {
            return;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(m_Buffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        m_Recording = true;
    }

    void VulkanCommandBuffer::End() {
        ZoneScopedVulkan;
        if (!m_Recording) {
            return;
        }

        if (vkEndCommandBuffer(m_Buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to stop recording command buffer!");
        }

        m_Recording = false;
    }

    void VulkanCommandBuffer::Reset() {
        ZoneScopedVulkan;
        if (m_Recording) {
            End();
        }

        if (vkResetCommandBuffer(m_Buffer, 0) != VK_SUCCESS) {
            throw std::runtime_error("Failed to reset command buffer!");
        }
    }

    VulkanQueue::VulkanQueue(VkDevice device, uint32_t family, GraphicsQueueType::Flags usage) {
        ZoneScopedVulkan;

        m_Device = device;
        m_Family = family;
        m_Usage = usage;
        vkGetDeviceQueue(device, m_Family, 0, &m_Queue);

        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = family;
        
        if (vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_Pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    VulkanQueue::~VulkanQueue() {
        ZoneScopedVulkan;
        ResetCache();

        while (!m_AvailableFences.empty()) {
            vkDestroyFence(m_Device, m_AvailableFences.front(), nullptr);
            m_AvailableFences.pop();
        }

        while (!m_AllocatedBuffers.empty()) {
            m_AllocatedBuffers.pop();
        }

        vkDestroyCommandPool(m_Device, m_Pool, nullptr);
    }

    CommandList& VulkanQueue::Release() {
        // todo
    }

    void VulkanQueue::Submit(CommandList& commandList, bool wait = false) {
        // todo
    }

    void VulkanQueue::Wait() {
        // todo
    }

    void VulkanQueue::ResetCache() {
        // todo
    }
}