#version 430 core

in vec2 UV;

out vec4 FragColor;

#include "UniformBufferObjects/Timers.glsl"

layout(binding = 0) uniform sampler2D Sampler;

void main() {
    vec4 Color = texture(Sampler, UV);
    //Color = texture(Sampler, UV * abs(sin(Timer)) + 0.3);
    //Color.x += fract(Timer);
    //Color.x = 1 - Color.x;
    FragColor = Color;
    //FragColor = vec4(vec3(Color.x), 1.0);
    //FragColor.a = (1.0 + cos(Timer)) * 0.5;
    //FragColor = vec4(UV, 0, 1);
}