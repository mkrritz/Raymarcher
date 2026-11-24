#include "pch.h"

#include "mkr/Shader.h"

ShaderConfig Programs[] = {
    { "Quad.vs", nullptr, "DrawTexture.fs", nullptr, nullptr, nullptr },
    { "QuadForRayMarching.vs", nullptr, "Raymarcher.fs",  nullptr, nullptr, nullptr },
    { nullptr, nullptr, nullptr, "View.cs", nullptr, nullptr },
};

static_assert(sizeof(Programs) / sizeof(Programs[0]) == (size_t)Shaders::Count,
              "Mismatch between Programs and 'enum struct Shaders'.");
