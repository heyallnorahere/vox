#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>

#define TRACY_VK_USE_SYMBOL_TABLE
#include <tracy/TracyVulkan.hpp>

#define ZoneScopedVulkan ZoneScopedC(::vox::VulkanProfilerColor)
namespace vox {
    constexpr uint32_t VulkanProfilerColor = tracy::Color::Red4;

    struct VulkanExtension {
        std::string Name;
        bool Required;
    };
} // namespace vox
