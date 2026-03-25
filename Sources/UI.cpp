
#include "pch.h"

#include "mkr/Assert.h"
#include "mkr/Memory.h"
#include "mkr/UI.h"
#include "mkr/Input.h"
#include "mkr/Shader.h"
#include "mkr/Camera.h"
#include "mkr/Window.h"

void RenderQuadToScreen(const Shader* const shaders[],
                        const Camera* const camera,
                        const UI* const ui,
                        const GLuint texture) {
    const Shader* shader = shaders[static_cast<uint32_t>(Shaders::DrawTexture)];
    
    glBindSampler(0, camera->sampler_clamp_linear);
    
    glUseProgram(shader->ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ui->ubo_handle[static_cast<uint32_t>(Quads::SCREEN)]);
    glBindTextureUnit(0, texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTextureUnit(0, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
}

void UIInit(Heap* const heap) {
    UI* ui = (UI*)HeapAllocate(heap, sizeof(UI));
    
    glGenVertexArrays(1, &ui->vao);
    
    {
        Quad& quad = ui->quads[static_cast<uint32_t>(Quads::SCREEN)];
        
        glCreateTextures(GL_TEXTURE_2D, 1, &quad.texture);
        glTextureStorage2D(quad.texture, 1, GL_RGBA8, WINDOW_WIDTH, WINDOW_HEIGHT);
        glCreateTextures(GL_TEXTURE_2D, 1, &quad.depthbuffer);
        glTextureStorage2D(quad.depthbuffer, 1, GL_DEPTH_COMPONENT24, WINDOW_WIDTH, WINDOW_HEIGHT);
        glCreateFramebuffers(1, &quad.framebuffer);
        glNamedFramebufferTexture(quad.framebuffer, GL_COLOR_ATTACHMENT0, quad.texture, 0);
        glNamedFramebufferTexture(quad.framebuffer, GL_DEPTH_ATTACHMENT, quad.depthbuffer, 0);
        
        if(glCheckNamedFramebufferStatus(quad.framebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            ASSERT(true, "Framebuffer Error");
        }
    }
    
    UniformBufferObjectUI buffer[static_cast<uint32_t>(Quads::Count)];
    buffer[static_cast<uint32_t>(Quads::SCREEN)].rect = glm::vec4(-1.0f, -1.0f, 2.0f, 2.0f);
    buffer[static_cast<uint32_t>(Quads::RAYMARCH)].rect = glm::vec4(-0.95f, -0.95f, 1.9f, 1.9f);
    glCreateBuffers(static_cast<uint32_t>(Quads::Count), ui->ubo_handle);
    for (uint32_t index = 0; index < static_cast<uint32_t>(Quads::Count); index++) {
        glNamedBufferStorage(ui->ubo_handle[index],
                             sizeof(UniformBufferObjectUI),
                             &buffer[index],
                             0);
    }
    
}

