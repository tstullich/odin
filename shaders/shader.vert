#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out dvec2 inPosition;
layout(location = 2) out vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inPosition;
}