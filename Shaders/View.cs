#version 430 core

layout(local_size_x = 1) in;

layout(std430, binding = 3) writeonly buffer CameraInverseView {
    mat4 InverseView;
};

layout(std430, binding = 6) readonly buffer CameraPosition {
    vec4 Camera;
};

layout(std430, binding = 7) readonly buffer CameraForward {
    vec4 Forward;
};

#include "UniformBufferObjects/Timers.glsl"
layout(std140, binding = 0) uniform SceneUniformBufferObject {
    TimersUBO Scene;
};

void main() {
    vec3 P = Camera.xyz;
    vec3 F = normalize(Forward.xyz);
    vec3 R = normalize(cross(F, vec3(0.0, 1.0, 0.0)));
    vec3 U = normalize(cross(R, F));

    InverseView = mat4(
        R.x, R.y, R.z, 0.0,
        U.x, U.y, U.z, 0.0,
       -F.x,-F.y,-F.z, 0.0,
        P.x, P.y, P.z, 1.0
    );
}

/*
    View = mat4(
        R.x,        U.x,       -F.x,      0.0,
        R.y,        U.y,       -F.y,      0.0,
        R.z,        U.z,       -F.z,      0.0,
       -dot(R, P), -dot(U, P), dot(F, P), 1.0
    );

    InverseView = mat4(
        R.x, R.y+0.3*cos(Scene.Timers.x*0.7), R.z, 0.0,
        U.x, U.y+0.2*sin(Scene.Timers.x), U.z, 0.0,
       -F.x,-F.y-0.4*sin(Scene.Timers.x*1.9),-F.z+0.5*cos(Scene.Timers.x), 0.0,
        P.x, P.y, P.z, 1.0
    );
*/