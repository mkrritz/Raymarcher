
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
void UpdateCamera(const Shader* const [],
                  Camera* const,
                  GLuint,
                  float,
                  float);
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

const uint32_t EVENT_CHANGE_SAMPLER = 1 << 0;
const uint32_t EVENT_CHANGE_VOLUME_SAMPLER = 1 << 1;

static uint32_t events = 0;
static uint32_t using_sampler= 0;
static uint32_t using_volume_sampler = 0;

static void Render(const Shader* const shaders[],
                   const Camera* const camera,
                   const UI* const ui,
                   GLuint scene_handle,
                   float TotalTime,
                   GLuint map,
                   GLuint volume) {
    const Shader* shader = shaders[static_cast<uint32_t>(Shaders::Raymarch)];
    
    switch(using_sampler) {
        case 0: {
            glBindSampler(4, camera->sampler_clamp_linear);
            break;
        }
        case 1: {
            glBindSampler(4, camera->sampler_clamp_nearest);
            break;
        }
    }
    
    switch(using_volume_sampler) {
        case 0: {
            glBindSampler(5, camera->sampler_clamp_linear);
            break;
        }
        case 1: {
            glBindSampler(5, camera->sampler_clamp_nearest);
            break;
        }
    }
    
    glUseProgram(shader->ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ui->ubo_handle[static_cast<uint32_t>(Quads::RAYMARCH)]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, camera->ubo_handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, scene_handle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, camera->position_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, camera->inverse_view_buffer);
    glBindTextureUnit(4, map);
    glBindTextureUnit(5, volume);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTextureUnit(4, 0);
    glBindTextureUnit(5, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
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
    
    if (input->letters & InputM::P) {
        if (input->toggles & InputM::Toggle_P) {
            events |= EVENT_CHANGE_SAMPLER;
        }
        input->toggles &= ~InputM::Toggle_P;
    } else input->toggles |= InputM::Toggle_P;
    
    if (input->letters & InputM::O) {
        if (input->toggles & InputM::Toggle_O) {
            events |= EVENT_CHANGE_VOLUME_SAMPLER;
        }
        input->toggles &= ~InputM::Toggle_O;
    } else input->toggles |= InputM::Toggle_O;
}

int main() {
    const int VOLUME_SIZE = 256;
    const int TOTAL_VOXELS = VOLUME_SIZE * VOLUME_SIZE * VOLUME_SIZE;
    float* volumeData = new float[TOTAL_VOXELS];
    srand(42);
    
    #if 0 // Random noise volume
    const float DENSITY_THRESHOLD = 0.2f; // 0.1 = 10% solid, 0.9 = 90% solid
    for (int i = 0; i < TOTAL_VOXELS; i++) {
        // Generate 0 to 1, then subtract threshold
        // Results in [-DENSITY_THRESHOLD, 1.0 - DENSITY_THRESHOLD]
        float noise = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        volumeData[i] = noise - DENSITY_THRESHOLD;
    }
    #else // Not random noise volume
    auto sdBox = [](glm::vec3 p, glm::vec3 b) {
        glm::vec3 q = glm::abs(p) - b;
        return glm::length(glm::max(q, glm::vec3(0.0f))) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
    };
    const float halfSize = static_cast<float>(VOLUME_SIZE) * 0.5f;
    const glm::vec3 center(halfSize);
    for (int z = 0; z < VOLUME_SIZE; z++) {
        for (int y = 0; y < VOLUME_SIZE; y++) {
            for (int x = 0; x < VOLUME_SIZE; x++) {
                glm::vec3 p(x, y, z);
                float d = sdBox(p - center, glm::vec3(halfSize));
                float s = 1.0f;
                for (int m = 0; m < 2; m++) {
                    glm::vec3 a = glm::mod(p * s, (float)VOLUME_SIZE) - center;
                    s *= 3.0f;
                    glm::vec3 r = glm::abs(1.0f - 3.0f * glm::abs(a) / halfSize);
                    float da = glm::max(r.x, r.y);
                    float db = glm::max(r.y, r.z);
                    float dc = glm::max(r.z, r.x);
                    float c = (glm::min(da, glm::min(db, dc)) - 1.0f) / s * halfSize;
                    d = glm::max(d, c);
                }
                volumeData[x + y * VOLUME_SIZE + z * VOLUME_SIZE * VOLUME_SIZE] = d;
            }
        }
    }
    #endif
    
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
    
    struct UniformBufferObjectScene {
        glm::vec4 Timers;
        // Sun position and whatnot
    };
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
        glCreateTextures(GL_TEXTURE_3D, 1, &volume);
        glTextureStorage3D(volume, 1, GL_R32F, VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE);
        glTextureSubImage3D(volume, 0, 0, 0, 0, VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE, GL_RED, GL_FLOAT, volumeData);
        delete[] volumeData;
    }
    
    glBindVertexArray(ui->vao); // There is only quad rendering happening.
    
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
        
        while(AccumulatorTime >= dt) {
            AccumulatorTime -= dt;
            FixedUpdateCount += 1;
            
            input = NextRecordedInputState();
            
            HandleInput(input, camera, dt);
            
            if (events) {
                if (events & EVENT_CHANGE_SAMPLER) {
                    events &= ~EVENT_CHANGE_SAMPLER;
                    using_sampler = 1 - using_sampler;
                }
                if (events & EVENT_CHANGE_VOLUME_SAMPLER) {
                    events &= ~EVENT_CHANGE_VOLUME_SAMPLER;
                    using_volume_sampler = 1 - using_volume_sampler;
                }
            }
            
            UpdateCamera(shaders, camera, scene_handle, TotalTime, dt);
            
            printf("Camera (%.2f, %.2f, %.2f) \r", camera->position->x, camera->position->y, camera->position->z);
        }
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, quad.framebuffer);
        glClearColor(1, 0, 0, 0); // Red
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Render(shaders, camera, ui, scene_handle, TotalTime, map, volume);
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0, 0, 1, 0); // Blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        RenderQuadToScreen(shaders, camera, ui, quad.texture);
        
        glfwSwapBuffers(window);
    }
    
    return 1;
}


