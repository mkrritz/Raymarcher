
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
                        const GLuint,
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
            //glBindSampler(4, camera->sampler_repeat_linear);
            break;
        }
        case 1: {
            glBindSampler(4, camera->sampler_clamp_nearest);
            //glBindSampler(4, camera->sampler_repeat_nearest);
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

// Tree SDF C++ Implementation
static float sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    return glm::length(pa - ba * h) - r;
}

static float sdBranch(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r1, float r2) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    float r = glm::mix(r1, r2, h);
    return glm::length(pa - ba * h) - r;
}

static glm::mat2 rot(float a) {
    float s = sin(a), c = cos(a);
    return glm::mat2(c, -s, s, c);
}

static float TreeSDF(glm::vec3 p) {
    // Redwood Trunk (base is at y=0, top at y=100)
    // Scale diameter down as it reaches max height (4.0 at base to 0.2 at top)
    float d = sdBranch(p, glm::vec3(0.0f), glm::vec3(0.0f, 100.0f, 0.0f), 4.0f, 0.2f);
    
    // Canopy Clusters via 1D domain repetition
    float spacing = 3.0f; // A cluster every 3 meters
    float base_id = glm::floor(p.y / spacing);
    
    // Evaluate current cell and neighbors to prevent clipping when branches cross cell boundaries
    for (int offset = -2; offset <= 1; offset++) {
        float cid = base_id + (float)offset;
        
        // Redwood branches are mainly at the top (y=60 to y=96)
        if(cid < 20.0f || cid > 32.0f) continue; 
        
        glm::vec3 bp = p - glm::vec3(0.0f, cid * spacing, 0.0f);
        
        // Spiral main branches around the trunk using golden angle
        glm::vec2 xz = rot(cid * 2.3999f) * glm::vec2(bp.x, bp.z);
        bp.x = xz.x; bp.z = xz.y;
        
        // Start from trunk surface
        float trunk_rad_at_y = glm::mix(4.0f, 0.2f, glm::clamp((cid * spacing) / 100.0f, 0.0f, 1.0f));
        bp.x -= trunk_rad_at_y;
        
        // Main branch droops downwards slightly
        glm::vec2 xy = rot(-0.1f) * glm::vec2(bp.x, bp.y);
        bp.x = xy.x; bp.y = xy.y;
        
        // Conical tree shape: longer branches at bottom of canopy
        float shape = 1.0f - (cid - 20.0f) / 13.0f; 
        shape = glm::clamp(shape, 0.3f, 1.0f);
        
        // Scale branch lengths to ensure they fit perfectly inside the 8.0m volume bounding radius
        // The max branch horizontal reach is ~2.35 * len. Max trunk radius is 4.0m. 
        // 2.5m * 2.35 = 5.87m + trunk_rad = ~7.5m (safely under 8.0m limit).
        float len = 2.5f * shape;
        
        // First level branches start exactly at a diameter of 1 (radius = 0.5)
        float rad = 0.5f;
        
        glm::vec3 fp = bp;
        float branch_id = cid; // Base ID for the cluster
        
        // FRACTAL SUB-BRANCHES (Infinite detail)
        for(int j = 0; j < 4; j++) {
            float next_rad = rad * 0.6f;
            
            // Generate a unique pseudo-random hash for this specific branch quadrant
            float hash = glm::fract(sin(glm::mod(branch_id, 1000.0f) * 135.233f) * 43758.5453f);
            
            // Random Pruning: 30% chance to skip drawing this branch.
            if(j > 0 && hash <= 0.3f) {
                break; 
            }
            
            d = glm::min(d, sdBranch(fp, glm::vec3(0.0f), glm::vec3(len, 0.0f, 0.0f), rad, next_rad));
            
            // Move to branch tip
            fp.x -= len;
            
            // Track which quadrant we are in BEFORE folding space.
            float qy = (fp.y >= 0.0f) ? 1.0f : 0.0f;
            float qz = (fp.z >= 0.0f) ? 1.0f : 0.0f;
            branch_id = branch_id * 4.0f + qy + qz * 2.0f;
            
            // Fold space in Y and Z to spawn 4 sub-branches per branch
            fp.y = glm::abs(fp.y);
            fp.z = glm::abs(fp.z);
            
            // Wiggle angles slightly based on the parent's hash
            float wiggle_y = (glm::fract(hash * 34.5f) - 0.5f) * 0.15f;
            float wiggle_z = (glm::fract(hash * 78.9f) - 0.5f) * 0.15f;
            
            // Bend the sub-branches outwards
            glm::vec2 fpxy = rot(0.4f + wiggle_y) * glm::vec2(fp.x, fp.y);
            fp.x = fpxy.x; fp.y = fpxy.y;
            
            glm::vec2 fpxz = rot(0.5f + wiggle_z) * glm::vec2(fp.x, fp.z);
            fp.x = fpxz.x; fp.z = fpxz.y;
            
            // Scale down for the next generation
            len *= 0.65f;
            rad = next_rad;
        }
    }
    
    return d;
}

int main() {
    const int VOL_X = 128;
    const int VOL_Y = 1024;
    const int VOL_Z = 128;
    const size_t TOTAL_VOXELS = (size_t)VOL_X * VOL_Y * VOL_Z;
    float* volumeData = new float[TOTAL_VOXELS];
    
    FILE* f = fopen("tree_volume_128.bin", "rb");
    if(f) {
        printf("Loading tree volume from tree_volume_128.bin... (64MB)\n");
        fread(volumeData, sizeof(float), TOTAL_VOXELS, f);
        fclose(f);
    } else {
        printf("Generating tree volume (this may take 5-15 seconds)...\n");
        
        // To speed up generation, pre-calculate bounding cylinders
        for (int z = 0; z < VOL_Z; z++) {
            for (int y = 0; y < VOL_Y; y++) {
                for (int x = 0; x < VOL_X; x++) {
                    // 8 voxels = 1 meter. 
                    // Volume dimensions: 16m x 128m x 16m.
                    // Center of XZ is 8m, 8m. Base of Y is 0m.
                    glm::vec3 p_meters(x / 8.0f, y / 8.0f, z / 8.0f);
                    glm::vec3 p_tree = p_meters - glm::vec3(8.0f, 0.0f, 8.0f);
                    
                    // Bounding optimization: if we are far from the tree, return a rough distance.
                    // The Redwood branches don't reach beyond 8 meters horizontally.
                    float dist_to_center = glm::length(glm::vec2(p_tree.x, p_tree.z));
                    float d;
                    if(dist_to_center > 14.0f || p_tree.y < -5.0f || p_tree.y > 110.0f) {
                        // Very rough bounding cylinder
                        d = glm::max(dist_to_center - 12.0f, glm::abs(p_tree.y - 50.0f) - 60.0f);
                    } else {
                        d = TreeSDF(p_tree);
                    }
                    
                    size_t index = (size_t)x + (size_t)y * VOL_X + (size_t)z * VOL_X * VOL_Y;
                    volumeData[index] = d;
                }
            }
        }
        
        f = fopen("tree_volume_128.bin", "wb");
        if(f) {
            printf("Saving tree volume to tree_volume_128.bin...\n");
            fwrite(volumeData, sizeof(float), TOTAL_VOXELS, f);
            fclose(f);
        }
    }
    
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
        glTextureStorage3D(volume, 1, GL_R32F, VOL_X, VOL_Y, VOL_Z);
        glTextureSubImage3D(volume, 0, 0, 0, 0, VOL_X, VOL_Y, VOL_Z, GL_RED, GL_FLOAT, volumeData);
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
        
        RenderQuadToScreen(shaders, camera, ui, quad.texture, scene_handle);
        
        glfwSwapBuffers(window);
    }
    
    return 1;
}


