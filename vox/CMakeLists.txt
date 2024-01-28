cmake_minimum_required(VERSION 3.21)

set(VOX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE VOX_SOURCE CONFIGURE_DEPENDS "${VOX_SOURCE_DIR}/*")
file(GLOB_RECURSE VOX_PLATFORM_CODE CONFIGURE_DEPENDS "${VOX_SOURCE_DIR}/vox/platform/*")
list(REMOVE_ITEM VOX_SOURCE ${VOX_PLATFORM_CODE})

set(VOX_LIBS spdlog::spdlog Tracy::TracyClient glfw)
set(VOX_INCLUDES ${VOX_SOURCE_DIR})

foreach(PLATFORM ${VOX_PLATFORMS})
    file(GLOB_RECURSE VOX_${PLATFORM}_CODE CONFIGURE_DEPENDS "${VOX_SOURCE_DIR}/vox/platform/${PLATFORM}/*")
    list(APPEND VOX_SOURCE ${VOX_${PLATFORM}_CODE})

    list(APPEND VOX_DEFINITIONS VOX_PLATFORM_${PLATFORM})
endforeach()

check_platform_support(vulkan VULKAN_SUPPORTED)
if(VULKAN_SUPPORTED)
    list(APPEND VOX_LIBS volk)
endif()

add_executable(vox ${VOX_SOURCE})
target_link_libraries(vox PUBLIC ${VOX_LIBS})
target_include_directories(vox PUBLIC ${VOX_INCLUDES})
target_compile_definitions(vox PUBLIC ${VOX_DEFINITIONS} $<$<CONFIG:Debug>:VOX_DEBUG>)

set_target_properties(vox PROPERTIES
    CXX_STANDARD 20
    FOLDER "vox")

set(VOX_PCH "${VOX_SOURCE_DIR}/voxpch.h")
target_precompile_headers(vox PRIVATE ${VOX_PCH})

foreach(VOX_SOURCE_FILE ${VOX_SOURCE})
    cmake_path(GET VOX_SOURCE_FILE PARENT_PATH PARENT_DIRECTORY)
    cmake_path(RELATIVE_PATH PARENT_DIRECTORY BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE SOURCE_FOLDER)

    string(FIND ${SOURCE_FOLDER} "." PERIOD_INDEX)
    if(${PERIOD_INDEX} LESS 0)
        source_group(${SOURCE_FOLDER} FILES ${VOX_SOURCE_FILE})
    endif()
endforeach()

message(STATUS "Using precompiled header: ${VOX_PCH}")
message(STATUS "Linking against external libraries: ${VOX_LIBS}")
message(STATUS "Adding include paths: ${VOX_INCLUDES}")
message(STATUS "Defining macros: ${VOX_DEFINITIONS}")
message(STATUS "Enabled platform IDs: ${VOX_PLATFORMS}")