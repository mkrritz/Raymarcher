struct TimersUBO {
    vec4 TIMERS;
};

layout(std140, binding = 0) uniform SceneUniformBufferObject {
    TimersUBO SCENE;
};

const float Timer = SCENE.TIMERS.x;