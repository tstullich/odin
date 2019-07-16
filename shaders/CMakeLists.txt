# New shaders need to be specified here in order for CMake to pickup changes
set(SHADERS shader.comp
            shader.frag
            shader.vert)

add_custom_target(
    CompileShaders ALL DEPENDS ${SHADERS}
)

# Command to also compile shaders during build stage
add_custom_command(TARGET CompileShaders POST_BUILD
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})