# TODO Find a better way to list source files
set(SOURCE_FILES
    main.cpp
    vk/instance.cpp
    vk/device_manager.cpp
    vk/swapchain.cpp
    vk/render_pass.cpp
    vk/descriptor_set.cpp
    vk/pipeline.cpp
    vk/shader_module.cpp
    vk/command_pool.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan ${VULKAN_SDK/lib})

install(TARGETS odin DESTINATION ${ODIN_INSTALL_BIN_DIR})