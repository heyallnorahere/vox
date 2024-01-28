cmake_minimum_required(VERSION 3.21)

add_subdirectory("glfw")
add_subdirectory("spdlog")
add_subdirectory("tracy")

# vulkan libraries
check_platform_support(vulkan VULKAN_SUPPORTED)
if(VULKAN_SUPPORTED)
    # headers
    add_subdirectory("Vulkan-Headers")

    # meta loader
    set(VOLK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/volk")
    add_library(volk STATIC "${VOLK_SOURCE_DIR}/volk.c" "${VOLK_SOURCE_DIR}/volk.h")
    target_link_libraries(volk PUBLIC Vulkan::Headers)
    target_include_directories(volk PUBLIC ${VOLK_SOURCE_DIR})
endif()