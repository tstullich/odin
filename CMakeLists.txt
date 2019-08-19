cmake_minimum_required(VERSION 3.13.4)
project(odin VERSION 0.1 LANGUAGES CXX)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION true)

find_package(Boost 1.69 COMPONENTS program_options REQUIRED)
find_package(glfw3 3.2 REQUIRED)
find_package(Vulkan REQUIRED)

# External dependencies from git submodules
set(GLM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/extern/glm)
set(STB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/extern/stb)
set(TINY_OBJ_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/extern/tinyobjloader)

set(ODIN_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(ODIN_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

# Adding different compiler flags for debug and release
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

include_directories(${Boost_INCLUDE_DIR})
include_directories(${VULKAN_INCLUDE_DIR})
include_directories(${STB_INCLUDE_DIR})
include_directories(${TINY_OBJ_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${ODIN_INCLUDE_DIR})

add_subdirectory(shaders)
add_subdirectory(src)