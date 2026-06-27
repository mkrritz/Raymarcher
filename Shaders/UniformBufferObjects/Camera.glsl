struct CameraUBO {
    vec4 ATTRIBUTES;
};

layout(std140, binding = 1) uniform CameraUniformBufferObject {
    CameraUBO CAMERA;
};

const float Aspect     = CAMERA.ATTRIBUTES.x;
const float TanHalfFOV = CAMERA.ATTRIBUTES.y;
const float FarPlane   = CAMERA.ATTRIBUTES.z;
//const float UNUSED   = CAMERA.ATTRIBUTES.w;