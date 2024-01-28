cmake_minimum_required(VERSION 3.21)
project(vox CXX C)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    list(APPEND VOX_PLATFORMS windows)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    list(APPEND VOX_PLATFORMS linux)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    list(APPEND VOX_PLATFORMS osx)
endif()

set(DESKTOP_PLATFORMS Windows Linux Darwin)
list(FIND DESKTOP_PLATFORMS ${CMAKE_SYSTEM_NAME} PLATFORM_INDEX)
if(${PLATFORM_INDEX} GREATER_EQUAL 0)
    list(APPEND VOX_PLATFORMS desktop)
endif()

add_subdirectory("vendor")
add_subdirectory("vox")

macro(get_targets DIRECTORY TARGETS)
    get_property(SUBDIRECTORIES DIRECTORY ${DIRECTORY} PROPERTY SUBDIRECTORIES)
    foreach(SUBDIRECTORY ${SUBDIRECTORIES})
        get_targets(${SUBDIRECTORY} ${TARGETS})
    endforeach()

    get_property(DIRECTORY_TARGETS DIRECTORY ${DIRECTORY} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${TARGETS} ${DIRECTORY_TARGETS})
endmacro()

get_targets("${CMAKE_SOURCE_DIR}/vendor" VENDORDEPS)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
foreach(VENDORDEP ${VENDORDEPS})
    set(FOLDER_NAME "vendor")

    get_target_property(CURRENT_FOLDER ${VENDORDEP} FOLDER)
    if(NOT CURRENT_FOLDER STREQUAL "CURRENT_FOLDER-NOTFOUND")
        set(FOLDER_NAME "${FOLDER_NAME}/${CURRENT_FOLDER}")
    endif()

    set_target_properties(${VENDORDEP} PROPERTIES FOLDER ${FOLDER_NAME})
endforeach()