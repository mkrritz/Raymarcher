#ifndef CAMERA_HPP
#define CAMERA_HPP

struct UniformBufferObjectCamera {
    glm::vec4 Attributes;
};

struct Camera {
    GLuint   sampler_clamp_nearest;
    GLuint   sampler_clamp_linear;
    GLuint   sampler_repeat_linear;
    GLuint   sampler_repeat_nearest;
    
    glm::vec4*   position;
    GLuint       position_buffer;
    glm::vec4*   forward;
    GLuint       forward_buffer;
    GLuint       view_buffer;
    GLuint       inverse_view_buffer;
    
    float fov;
    float tan_half_fov;
    float aspect;
    float near_plane;
    float far_plane;
    
    GLuint ubo_handle;
    UniformBufferObjectCamera* ubo;
    
    glm::vec4 up;
    glm::vec4 right;
    float yaw;
    float pitch;
    
};


#endif