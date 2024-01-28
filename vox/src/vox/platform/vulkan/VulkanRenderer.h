#pragma once

#include "vox/renderer/Renderer.h"

namespace vox {
    class VulkanRenderer : public Renderer {
    public:
        static VulkanRenderer* GetRenderer();

        VulkanRenderer();
        virtual ~VulkanRenderer() override;

        virtual void Query(Info& info) override;

        VkInstance GetInstance() { return m_Instance; }
        TracyVkCtx GetProfilerContext() { return m_ProfilerContext; }

    private:
        void CreateInstance();
        void CreateDebugMessenger();

        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        TracyVkCtx m_ProfilerContext;
    };
} // namespace vox