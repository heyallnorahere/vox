#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>

#define TRACY_VK_USE_SYMBOL_TABLE
#include <tracy/TracyVulkan.hpp>

namespace vox {
    constexpr uint32_t VulkanProfilerColor = tracy::Color::Red4;
} // namespace vox

#define ZoneScopedVulkan ZoneScopedC(::vox::VulkanProfilerColor);