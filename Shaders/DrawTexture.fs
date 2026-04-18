#version 430 core

in vec2 UV;

out vec4 FragColor;

#include "UniformBufferObjects/Timers.glsl"
layout(std140, binding = 2) uniform SceneUniformBufferObject {
    TimersUBO Scene;
};

layout(binding = 0) uniform sampler2D Sampler;

void main() {
    vec4 Color = texture(Sampler, UV);
    //Color = texture(Sampler, UV * abs(sin(Scene.Timers.x)) + 0.3);
    //Color.x += fract(Scene.Timers.x);
    //Color.x = 1 - Color.x;
    FragColor = Color;
    //FragColor = vec4(vec3(Color.x), 1.0);
    //FragColor.a = (1.0 + cos(Scene.Timers.x)) * 0.5;
    //FragColor = vec4(UV, 0, 1);
}