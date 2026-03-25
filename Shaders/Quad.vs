#version 430 core

out vec2 UV;

#include "UniformBufferObjects/UI.glsl"
layout(std140, binding = 0) uniform UIUniformBufferObject {
    UIUBO UI;
};

void main() {
    UV = vec2((gl_VertexID >> 1) & 1, gl_VertexID & 1);
    
    gl_Position = vec4(UI.Rect.x + UV.x * UI.Rect.z,
                       UI.Rect.y + UV.y * UI.Rect.w,
                       0.0, 1.0);
}
