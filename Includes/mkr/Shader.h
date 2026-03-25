#ifndef SHADER_HPP
#define SHADER_HPP

/* Notes
    When adding/removing a shader we update two things.
        1). 'enum struct Shaders' in 'Shader.h'
        2). 'ShaderConfig SHADERS_CONFIG[]' in 'Shader.cpp'
        These two must be in the same order!
        That way we can index to shaders through Shaders::Name,
        and Name will really be that one because
        it's in that index at SHADERS_CONFIG
*/

enum struct Shaders {
    DrawTexture,
    Raymarch,
    View,
    Count // Not really a shader.
};

struct Shader {
    uint32_t slot;
    GLuint ID;
    #define MAX_FILE_NAME 32
    char vs_name[MAX_FILE_NAME];
    char gs_name[MAX_FILE_NAME];
    char fs_name[MAX_FILE_NAME];
    char cs_name[MAX_FILE_NAME];
    char tes_name[MAX_FILE_NAME];
    char tcs_name[MAX_FILE_NAME];
    #undef MAX_FILE_NAME
    time_t vs_time;
    time_t gs_time;
    time_t fs_time;
    time_t cs_time;
    time_t tes_time;
    time_t tcs_time;
};

#endif
