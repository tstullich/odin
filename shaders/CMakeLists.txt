# New shaders need to be specified here in order for CMake to pickup changes
set(SHADERS shader.frag
            shader.vert)

# TODO Find way to build shaders post compilation and not with
# the main build target
add_custom_target(
    BuildShaders ALL DEPENDS ${SHADERS}
)

# Command to also compile shaders during build stage
add_custom_command(TARGET BuildShaders POST_BUILD
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})