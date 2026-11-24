#ifndef SHADER_HPP
#define SHADER_HPP

#include <Focus.h>

enum struct Shaders {
    DrawTexture,
    Raymarch,
    View,
    Count // Not really a shader.
};

extern ShaderConfig Programs[];

#endif
