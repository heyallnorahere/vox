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

        VkInstance m_Instance;
        TracyVkCtx m_ProfilerContext;
    };
} // namespace vox