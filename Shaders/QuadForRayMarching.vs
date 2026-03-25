#version 430 core

out vec2 UV;
out vec3 Direction;

#include "UniformBufferObjects/UI.glsl"
layout(std140, binding = 0) uniform UIUniformBufferObject {
    UIUBO UI;
};

layout(location = 0) uniform float Aspect;
layout(location = 1) uniform float TanHalfFOV;

layout(std430, binding = 7) readonly buffer CameraInverseView {
    mat4 InverseView;
};

void main() {
    UV = vec2((gl_VertexID >> 1) & 1, (gl_VertexID) & 1);
    
    gl_Position = vec4(UI.Rect.x + UV.x * UI.Rect.z,
                       UI.Rect.y + UV.y * UI.Rect.w,
                       0.0, 1.0);
    
    vec2 NDC = (UV * 2.0 - 1.0);
    vec3 ViewSpaceDirection = normalize(vec3(NDC.x * TanHalfFOV * Aspect,
                                             NDC.y * TanHalfFOV,
                                             -1.0));
    Direction = mat3(InverseView) * ViewSpaceDirection;
}
