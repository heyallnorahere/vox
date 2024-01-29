#pragma once

#include "vox/renderer/CommandQueue.h"

namespace vox {
    class VulkanQueue;
    class VulkanCommandBuffer : public CommandList {
    public:
        VulkanCommandBuffer(VulkanQueue* queue);
        ~VulkanCommandBuffer();

        virtual bool IsRecording() const override { return m_Recording; }
        virtual GraphicsQueueType::Flags GetUsage() const override { return m_Usage; }

        virtual void* Raw() const override { return m_Buffer; }
        virtual VkCommandBuffer GetBuffer() const { return m_Buffer; }

        virtual void Begin() override;
        virtual void End() override;
        void Reset();

    private:
        VkCommandBuffer m_Buffer;
        VkCommandPool m_Pool;
        VkDevice m_Device;

        GraphicsQueueType::Flags m_Usage;
        bool m_Recording;
    };

    class VulkanQueue : public CommandQueue {
    public:
        VulkanQueue(VkDevice device, uint32_t family, GraphicsQueueType::Flags usage);
        virtual ~VulkanQueue() override;

        virtual GraphicsQueueType::Flags GetUsage() override { return m_Usage; }
        VkDevice GetDevice() const { return m_Device; }
        VkQueue GetQueue() const { return m_Queue; }
        uint32_t GetQueueFamily() const { return m_Family; }
        VkCommandPool GetPool() const { return m_Pool; }

        virtual CommandList& Release() override;
        virtual void Submit(CommandList& commandList, bool wait = false) override;

        virtual void Wait() override;
        virtual void ResetCache() override;

    private:
        struct StoredCommandBuffer {
            std::unique_ptr<VulkanCommandBuffer> CommandBuffer;
            VkFence Fence;
            bool OwnsFence;
        };

        VkDevice m_Device;
        VkQueue m_Queue;
        uint32_t m_Family;
        GraphicsQueueType::Flags m_Usage;

        VkCommandPool m_Pool;
        std::queue<StoredCommandBuffer> m_StoredBuffers;
        std::queue<VkFence> m_AvailableFences;
        std::queue<std::unique_ptr<VulkanCommandBuffer>> m_AllocatedBuffers;
    };
}