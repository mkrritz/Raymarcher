#ifndef SHADER_HPP
#define SHADER_HPP

#include <Focus.h>

enum struct Shaders {
    DrawTexture,
    Raymarch,
    View,
    Count // Not really a shader.
};

extern const ShaderFileSet FocusShaderPrograms[];
extern const size_t FocusShaderProgramCount;

#endif
