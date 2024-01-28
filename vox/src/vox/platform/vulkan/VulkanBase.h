#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>

#define TRACY_VK_USE_SYMBOL_TABLE
#include <tracy/TracyVulkan.hpp>

#define ZoneScopedVulkan ZoneScopedC(::vox::VulkanProfilerColor)
namespace vox {
    constexpr uint32_t VulkanProfilerColor = tracy::Color::Red4;

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        ZoneScopedVulkan;

        spdlog::level::level_enum level;
        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = spdlog::level::warn;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = spdlog::level::err;
            break;
        }

        spdlog::log(level, "Vulkan: {}", pCallbackData->pMessage);
        return VK_FALSE;
    }
} // namespace vox
