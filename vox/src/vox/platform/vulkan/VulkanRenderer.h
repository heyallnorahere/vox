#pragma once

#include "vox/renderer/Renderer.h"
#include "vox/platform/vulkan/VulkanDevice.h"

namespace vox {
    class VulkanRenderer : public Renderer {
    public:
        static VulkanRenderer* GetRenderer();
        static VkPhysicalDevice ChoosePhysicalDevice(VkInstance instance);

        VulkanRenderer();
        virtual ~VulkanRenderer() override;

        virtual void Query(Info& info) override;

        VkInstance GetInstance() { return m_Instance; }
        Ref<VulkanDevice> GetDevice() { return m_Device; }
        TracyVkCtx GetProfilerContext() { return m_ProfilerContext; }

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateDevice();

        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        Ref<VulkanDevice> m_Device;
        TracyVkCtx m_ProfilerContext;
    };
} // namespace vox