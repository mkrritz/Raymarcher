
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
    
    camera->CPU.position = glm::vec4(20.65, 42.96, 40.86, 1.0);
    camera->CPU.position = glm::vec4(-65.72, 436.70, 25.99, 1.0);
    camera->CPU.position = glm::vec4(220.69, 90.73, 222.33, 1.0);
    //camera->CPU.position = glm::vec4(799.34, 386.14, 368.42, 1.0);
    camera->CPU.forward  = glm::normalize(glm::vec4(-glm::vec3(camera->CPU.position), 0));
    camera->CPU.up       = glm::normalize(glm::vec4(0, 1, 0, 0));
    camera->CPU.right    = glm::normalize(glm::vec4(glm::cross(glm::vec3(camera->CPU.forward), glm::vec3(camera->CPU.up)), 0));
    
    glm::vec3 f = glm::normalize(glm::vec3(camera->CPU.forward));
    camera->CPU.pitch = glm::degrees(asin(f.y));
    camera->CPU.yaw   = glm::degrees(atan2(f.z, f.x));
    
    glCreateBuffers(1, &camera->position_buffer);
    glNamedBufferStorage(camera->position_buffer, sizeof(glm::vec4), &camera->CPU.position[0],
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    camera->position = (glm::vec4*)glMapNamedBufferRange(camera->position_buffer, 0, sizeof(glm::vec4),
                                                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glCreateBuffers(1, &camera->forward_buffer);
    glNamedBufferStorage(camera->forward_buffer, sizeof(glm::vec4), &camera->CPU.forward[0],
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    camera->forward = (glm::vec4*)glMapNamedBufferRange(camera->forward_buffer, 0, sizeof(glm::vec4),
                                                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glCreateBuffers(1, &camera->inverse_view_buffer);
    glNamedBufferStorage(camera->inverse_view_buffer, sizeof(glm::mat4), nullptr,
                         GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
}
