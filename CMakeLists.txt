cmake_minimum_required (VERSION 3.13.4)
project (odin)
enable_language(C)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(${VULKAN_INCLUDE_DIR})

find_package(glfw3 3.2 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan ${VULKAN_SDK/lib})