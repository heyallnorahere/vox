cmake_minimum_required(VERSION 3.21)

set(VOX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE VOX_SOURCE CONFIGURE_DEPENDS "${VOX_SOURCE_DIR}/*")
add_executable(vox ${VOX_SOURCE})

set(VOX_LIBS spdlog::spdlog Tracy::TracyClient)
set(VOX_INCLUES ${VOX_SOURCE_DIR})

target_link_libraries(vox PUBLIC ${VOX_LIBS})
target_include_directories(vox PUBLIC ${VOX_INCLUDES})
set_target_properties(vox PROPERTIES
    CXX_STANDARD 20
    FOLDER "vox")