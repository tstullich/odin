# TODO Find a better way to list source files
set(SOURCE_FILES
    main.cpp
    renderer/application.cpp
    vk/instance.cpp
    vk/device_manager.cpp
    vk/swapchain.cpp
    vk/render_pass.cpp
    vk/graphics_pipeline.cpp
    vk/shader_module.cpp
    vk/command_pool.cpp
    vk/image.cpp
    vk/texture_image.cpp
    vk/depth_image.cpp
    vk/buffer.cpp
    vk/index_buffer.cpp
    vk/vertex_buffer.cpp
    vk/triangle_buffer.cpp
    vk/uniform_buffer.cpp
    vk/texture_sampler.cpp
    vk/descriptor_set_layout.cpp
    vk/descriptor_pool.cpp
    vk/compute_pipeline.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Used for debugging
target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

target_link_libraries(${PROJECT_NAME}
                        glfw Vulkan::Vulkan
                        ${VULKAN_SDK/lib}
                        ${Boost_LIBRARIES})

install(TARGETS odin DESTINATION ${ODIN_INSTALL_BIN_DIR})