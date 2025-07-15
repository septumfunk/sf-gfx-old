#ifndef SHADERS_H
#define SHADERS_H

#include <sf/str.h>
#include <sf/dynamic.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "export.h"

/// An OpenGL shader program and its vertex/fragment glsl shaders.
/// Uniform locations are automatically cached as you use them.
typedef struct {
    sf_str path;
    GLuint vertex, fragment, program;
    sf_map uniforms;
} sf_shader;

/// Compile and link shaders into a program.
/// Returns a result if it fails.
[[nodiscard]] EXPORT sf_result sf_shader_new(sf_shader *out, sf_str path);
/// Free a shader and its code/program.
/// Cached uniforms will be reset.
EXPORT void sf_shader_free(sf_shader *shader);

/// Bind to the shader's OpenGL program.
static inline void sf_shader_bind(const sf_shader *shader) { glUseProgram(shader->program); }

/// Set a shader's float uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_float(sf_shader *shader, sf_str name, float value);
/// Set a shader's int uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_int(sf_shader *shader, sf_str name, int value);
/// Set a shader's vector2 uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_vec2(sf_shader *shader, sf_str name, sf_vec2 value);
/// Set a shader's vector3 uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_vec3(sf_shader *shader, sf_str name, sf_vec3 value);
/// Set a shader's matrix uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_mat4(sf_shader *shader, sf_str name, const mat4 value);

/// Log OpenGL errors to the console.
static inline void sf_opengl_log() {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        printf("OpenGL Error: %d\n", err);
    }
}

/// A color defined by its Red, Blue, Green and Alpha
typedef struct {
    uint8_t r, g, b, a;
} sf_rgba;
static inline sf_str sf_rgba_str(const sf_rgba rgba) {
    return sf_str_fmt("{ %d, %d, %d, %d }",
        rgba.r,
        rgba.g,
        rgba.a,
        rgba.b
    );
}

/// A color compatible with OpenGL.
typedef union {
    struct {
        float r;
        float g;
        float b;
        float a;
    };
    float gl[4];
} sf_glcolor;

static inline sf_glcolor sf_rgbagl(const sf_rgba rgba) {
    return (sf_glcolor){ { rgba.r / 255.0f, rgba.g / 255.0f, rgba.b / 255.0f, rgba.a / 255.0f } };
}
static inline sf_rgba sf_glrgba(const sf_glcolor gl) {
    return (sf_rgba){ (uint8_t)(gl.r * 255), (uint8_t)(gl.g * 255), (uint8_t)(gl.b * 255), (uint8_t)(gl.a * 255) };
}
static inline sf_str sf_glcolor_str(const sf_glcolor gl) {
    return sf_str_fmt("{ %f, %f, %f, %f }",
        (double)gl.r,
        (double)gl.g,
        (double)gl.b,
        (double)gl.a
    );
}

#define SF_WHITE (sf_rgba){255, 255, 255, 255}
#define SF_BLACK (sf_rgba){0, 0, 0, 255}

/// Turns an sf_transform into a model matrix.
EXPORT void sf_transform_model(mat4 out, sf_transform transform);
/// Turns an sf_transform into a view matrix.
EXPORT void sf_transform_view(mat4 out, sf_transform transform);

#endif // SHADERS_H
