
#include "pch.h"
#include <cstdlib>

#include "mkr/Assert.h"
#include "mkr/Input.h"
#include "mkr/Shader.h"
#include "mkr/Globals.h"
#include "mkr/Camera.h"
#include "mkr/Memory.h"
#include "mkr/UI.h"
#include "mkr/Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

GLFWwindow* WindowInit();

//Memory
void HeapsInit(Heap*[]);
//Memory

// Camera
void CameraInit(Heap* const);
// Camera

// Shader
void ShadersInit(Heap* const,
                 Shader*[], bool);
void ReloadShader(Heap* const,
                  uint32_t);
// Shader

// Input
Input* NextRecordedInputState();
// Input

// UI
void UIInit(Heap* const);
void RenderQuadToScreen(const Shader* const [],
                        const Camera* const,
                        const UI* const,
                        const GLuint);
// UI

#define EVENT_CHANGE_SAMPLER 0b100

static void RenderWorld(const Shader* const shaders[],
                        const Camera* const camera,
                        const UI* const ui,
                        GLuint scene_handle,
                        float TotalTime,
                        GLuint map,
                        GLuint volume,
                        uint32_t using_sampler) {
    const Shader* shader;
    
    switch(using_sampler) {                                        // This is fine...
        case 0: {                                                  // This is fine...
            glBindSampler(4, camera->sampler_clamp_linear);        // This is fine...
            break;                                                 // This is fine...
        }                                                          // This is fine...
        case 1: {                                                  // This is fine...
            glBindSampler(4, camera->sampler_clamp_nearest);       // This is fine...
            break;                                                 // This is fine...
        }
    }
    
    shader = shaders[static_cast<uint32_t>(Shaders::Raymarch)];
    glUseProgram(shader->ID);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ui->ubo_handle[static_cast<uint32_t>(Quads::RAYMARCH)]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, camera->ubo_handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, scene_handle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, camera->position_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, camera->inverse_view_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, volume);
    glBindTextureUnit(4, map);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTextureUnit(4, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);
}

static void UpdateCamera(const Shader* const shaders[],
                         Camera* const camera,
                         GLuint scene_handle,
                         float TotalTime,
                         float dt) {
    const Shader* shader;
    
    shader = shaders[static_cast<uint32_t>(Shaders::View)];
    glUseProgram(shader->ID);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, scene_handle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, camera->inverse_view_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, camera->position_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, camera->forward_buffer);
    glDispatchCompute(1, 1, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
}

static void HandleInput(Input* const input,
                        Camera* const camera,
                        float dt) {
    if (input->wheel) {
        camera->fov -= input->wheel;
        camera->tan_half_fov = glm::tan(glm::radians(camera->fov * 0.5));
        input->wheel = 0;
        
        camera->ubo->Attributes.y = camera->tan_half_fov;
    }
    
    glm::vec4 position  = *camera->position;
    glm::vec4 forward   = *camera->forward;
    
    glm::vec4& right    = camera->right;
    glm::vec4& up       = camera->up;
    float& yaw          = camera->yaw;
    float& pitch        = camera->pitch;
    
    float speed = 1.0f;
    
    if (input->mouse & InputM::LEFT_CLICK) {
        float sensitivity = 20.0f;
        yaw   -= (input->x_previous - input->x_current) * dt * sensitivity;
        pitch += (input->y_previous - input->y_current) * dt * sensitivity;
        pitch = glm::clamp(pitch, -85.0f, 85.0f);
        
        input->x_previous = input->x_current;
        input->y_previous = input->y_current;
        
        glm::vec3 new_forward(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                              sin(glm::radians(pitch)),
                              sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        forward = glm::normalize(glm::vec4(new_forward, 0));
        right   = glm::normalize(glm::vec4(glm::cross(glm::vec3(forward), glm::vec3(0,1,0)), 0));
        up      = glm::normalize(glm::vec4(glm::cross(glm::vec3(right), glm::vec3(forward)), 0));
        
        speed = 0.1f;
    }
    
    if (input->special & InputM::N2) {
        speed *= 10.0f;
    } else if (input->special & InputM::N3) {
        speed *= 100.0f;
    }
    
    if (input->letters & InputM::W) position += speed * forward * dt;
    if (input->letters & InputM::S) position -= speed * forward * dt;
    if (input->letters & InputM::A) position -= speed * right * dt;
    if (input->letters & InputM::D) position += speed * right * dt;
    if (input->letters & InputM::E) position += speed * up * dt;
    if (input->letters & InputM::Q) position -= speed * up * dt;
    
    *camera->position = position;
    *camera->forward  = forward;
}

struct UniformBufferObjectScene {
    glm::vec4 Timers;
    // Sun position and whatnot
};

int main() {
    GLFWwindow* window;
    window = WindowInit();
    
    Input* input = NextRecordedInputState();
    
    Heap* heaps[static_cast<uint32_t>(Heaps::Count)] = {nullptr};
    HeapsInit(heaps);
    
    Shader* shaders[static_cast<uint32_t>(Shaders::Count)] = {nullptr};
    ShadersInit(heaps[static_cast<uint32_t>(Heaps::Shader)], shaders, true);
    
    CameraInit(heaps[static_cast<uint32_t>(Heaps::Camera)]);
    Camera* const camera = (Camera*)heaps[static_cast<uint32_t>(Heaps::Camera)]->memory;
    
    UIInit(heaps[static_cast<uint32_t>(Heaps::UI)]);
    UI* const ui = (UI*)heaps[static_cast<uint32_t>(Heaps::UI)]->memory;
    
    Quad& quad = ui->quads[static_cast<uint32_t>(Quads::SCREEN)];
    
    GLuint scene_handle;
    UniformBufferObjectScene* scene;
    
    glCreateBuffers(1, &scene_handle);
    glNamedBufferStorage(scene_handle,
                         sizeof(UniformBufferObjectScene),
                         nullptr,
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    scene = (UniformBufferObjectScene*)glMapNamedBufferRange(
            scene_handle,
            0,
            sizeof(UniformBufferObjectScene),
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
    glBindVertexArray(ui->vao); // There is only quad rendering happening.
    
    GLuint map;
    {
        int W, H, C;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* img = stbi_load("Map\\map.png", &W, &H, &C, 1);
        stbi_set_flip_vertically_on_load(false);

        glCreateTextures(GL_TEXTURE_2D, 1, &map);
        glTextureStorage2D(map, 1, GL_R8, W, H);
        glTextureSubImage2D(
            map,
            0,
            0, 0,
            W, H,
            GL_RED,
            GL_UNSIGNED_BYTE,
            img
        );
        stbi_image_free(img);
    }
    
    GLuint volume;
    {
        const int VOLUME_SIZE = 256;
        const int TOTAL_VOXELS = VOLUME_SIZE * VOLUME_SIZE * VOLUME_SIZE;
        // 64MB for 32-bit float SDF values
        const int FLOAT_COUNT = TOTAL_VOXELS;
        
        float* volumeData = new float[FLOAT_COUNT];
        srand(42);
        for (int i = 0; i < FLOAT_COUNT; i++) {
            // Store random distance in range [0, 200]
            volumeData[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.0f;
        }
        
        glCreateBuffers(1, &volume);
        glNamedBufferStorage(volume, FLOAT_COUNT * sizeof(float), volumeData, 0);
        delete[] volumeData;
    }
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glPointSize(5);
    glLineWidth(2);
    glfwSwapInterval(1);
    
    const float dt = 1.0f/60.0f;
    float CurrectTime, LastTime, DeltaTime;
    float AccumulatorTime, TotalTime, EveryTwoSeconds;
    LastTime = glfwGetTime();
    CurrectTime = DeltaTime = TotalTime = EveryTwoSeconds = 0;
    AccumulatorTime = dt;
    uint32_t UpdateCount= 0;
    uint32_t FixedUpdateCount= 0;
    
    uint32_t events = 0;
    uint32_t using_sampler = 0;
    
    while (!(input->special & InputM::ESC)) {
        CurrectTime = glfwGetTime();
        DeltaTime = CurrectTime - LastTime;
        LastTime = CurrectTime;
        AccumulatorTime += DeltaTime;
        TotalTime += DeltaTime;
        EveryTwoSeconds += DeltaTime;
        
        scene->Timers.x = TotalTime;
        
        UpdateCount += 1;
        glfwPollEvents();
        
        while (shader_slots_to_reload) {
            uint32_t slot = __builtin_ctzll(shader_slots_to_reload);
            shader_slots_to_reload &= ~(1ULL << slot);
            ReloadShader(heaps[static_cast<uint32_t>(Heaps::Shader)], slot);
        }
        
        input = NextRecordedInputState();
        
        while(AccumulatorTime >= dt) {
            AccumulatorTime -= dt;
            FixedUpdateCount += 1;
            
            HandleInput(input, camera, dt);
            
            UpdateCamera(shaders, camera, scene_handle, TotalTime, dt);
            
            printf("Camera (%.2f, %.2f, %.2f) \r", camera->position->x, camera->position->y, camera->position->z);
        }
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, quad.framebuffer);
        glClearColor(1, 0, 0, 0); // Red
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        #if 1
        if (input->letters & InputM::P) {
            if (input->toggles & InputM::Toggle_P) {
                events |= EVENT_CHANGE_SAMPLER;
            }
            input->toggles &= ~InputM::Toggle_P;
        } else input->toggles |= InputM::Toggle_P;
        
        if (events) {
            if (events & EVENT_CHANGE_SAMPLER) {
                events &= ~EVENT_CHANGE_SAMPLER;
                using_sampler = (using_sampler + 1) % 2;
            }
        }
        
        #endif
        
        RenderWorld(shaders, camera, ui, scene_handle, TotalTime, map, volume, using_sampler);
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0, 0, 1, 0); // Blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        RenderQuadToScreen(shaders, camera, ui, quad.texture);
        
        glfwSwapBuffers(window);
    }
    
    return 1;
}


