#include "voxpch.h"
#include "vox/renderer/Renderer.h"

#ifdef VOX_PLATFORM_vulkan
#include "vox/platform/vulkan/VulkanBase.h"
#include "vox/platform/vulkan/VulkanRenderer.h"
#endif

namespace vox {
    std::unique_ptr<Renderer> Renderer::Create(API api) {
        Renderer* renderer;
        switch (api) {
#ifdef VOX_PLATFORM_vulkan
        case API::Vulkan:
            renderer = new VulkanRenderer;
            break;
#endif
        default:
            renderer = nullptr;
            break;
        }

        return std::unique_ptr<Renderer>(renderer);
    }
} // namespace vox