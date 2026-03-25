#ifndef UI_HPP
#define UI_HPP

enum struct Quads {
    RAYMARCH,
    SCREEN,
    Count // Not really a quad.
};

struct UniformBufferObjectUI {
    glm::vec4 rect;
};

struct Quad {
    GLuint framebuffer;
    GLuint texture, depthbuffer;
};

struct UI {
    GLuint vao, vbo, ibo;
    Quad quads[static_cast<uint32_t>(Quads::Count)];
    GLuint ubo_handle[static_cast<uint32_t>(Quads::Count)];
};

#endif
