
#include "pch.h"

#include "mkr/Assert.h"
#include "mkr/Window.h"
#include "mkr/Camera.h"
#include "mkr/Shader.h"
#include "mkr/Memory.h"

static GLuint Sampler(GLuint wrap, GLuint filter) {
    GLuint s;
    glGenSamplers(1, &s);
    glSamplerParameteri(s, GL_TEXTURE_WRAP_S, wrap);
    glSamplerParameteri(s, GL_TEXTURE_WRAP_T, wrap);
    glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, filter);
    glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, filter); 
    return s;
}

void CameraInit(Heap* const heap) {
    Camera* camera = (Camera*)HeapAllocate(heap, sizeof(Camera));
    camera->sampler_clamp_nearest  = Sampler(GL_CLAMP_TO_EDGE, GL_NEAREST);
    camera->sampler_clamp_linear   = Sampler(GL_CLAMP_TO_EDGE, GL_LINEAR);
    camera->sampler_repeat_linear  = Sampler(GL_REPEAT, GL_LINEAR);
    camera->sampler_repeat_nearest  = Sampler(GL_REPEAT, GL_NEAREST);
    
    camera->fov = 120.0f;
    camera->tan_half_fov = glm::tan(glm::radians(camera->fov * 0.5));
    camera->aspect = 4.0f / 3.0f;
    camera->near_plane = 0.1f;
    camera->far_plane  = 250.0f;
    
    glm::vec4 position = glm::vec4(236.55, 56.72, 79.25, 1.0);
    glm::vec4 forward  = glm::normalize(glm::vec4(-glm::vec3(position), 0));
    camera->up       = glm::normalize(glm::vec4(0, 1, 0, 0));
    camera->right    = glm::normalize(glm::vec4(glm::cross(glm::vec3(forward), glm::vec3(camera->up)), 0));
    
    glm::vec3 f = glm::normalize(glm::vec3(forward));
    camera->pitch = glm::degrees(asin(f.y));
    camera->yaw   = glm::degrees(atan2(f.z, f.x));
    
    glCreateBuffers(1, &camera->position_buffer);
    glNamedBufferStorage(camera->position_buffer, sizeof(glm::vec4), &position[0],
                         GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    camera->position = (glm::vec4*)glMapNamedBufferRange(camera->position_buffer, 0, sizeof(glm::vec4),
                                                         GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glCreateBuffers(1, &camera->forward_buffer);
    glNamedBufferStorage(camera->forward_buffer, sizeof(glm::vec4), &forward[0],
                         GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    camera->forward = (glm::vec4*)glMapNamedBufferRange(camera->forward_buffer, 0, sizeof(glm::vec4),
                                                         GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glCreateBuffers(1, &camera->inverse_view_buffer);
    glNamedBufferStorage(camera->inverse_view_buffer, sizeof(glm::mat4), nullptr,
                         GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
    glCreateBuffers(1, &camera->ubo_handle);
    glNamedBufferStorage(camera->ubo_handle,
                         sizeof(UniformBufferObjectCamera),
                         nullptr,
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    camera->ubo = (UniformBufferObjectCamera*)glMapNamedBufferRange(
                  camera->ubo_handle,
                  0,
                  sizeof(UniformBufferObjectCamera),
                  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
    camera->ubo->Attributes.x = camera->aspect;
    camera->ubo->Attributes.y = camera->tan_half_fov;
    camera->ubo->Attributes.z = camera->far_plane;
}
