#version 430 core

out vec2 UV;

#include "UniformBufferObjects/UI.glsl"

void main() {
    UV = vec2((gl_VertexID >> 1) & 1, gl_VertexID & 1);
    
    gl_Position = vec4(NDCX + UV.x * NDCW,
                       NDCY + UV.y * NDCH,
                       0.0, 1.0);
}
