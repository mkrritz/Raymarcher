#include "pch.h"

#include "mkr/Shader.h"

extern const ShaderConfig FocusShaderPrograms[] = {
    { "Quad.vs", nullptr, "DrawTexture.fs", nullptr, nullptr, nullptr },
    { "QuadForRayMarching.vs", nullptr, "Raymarcher.fs",  nullptr, nullptr, nullptr },
    { nullptr, nullptr, nullptr, "View.cs", nullptr, nullptr },
};

extern const size_t FocusShaderProgramCount =
    sizeof(FocusShaderPrograms) / sizeof(FocusShaderPrograms[0]);

static_assert(sizeof(FocusShaderPrograms) / sizeof(FocusShaderPrograms[0]) ==
              (size_t)Shaders::Count,
              "The enum and the Focus table must list the same programs.");
