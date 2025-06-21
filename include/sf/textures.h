#ifndef TEXTURES_H
#define TEXTURES_H

#include <sf/dynamic.h>
#include <sf/numerics.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/// A wrapper around an OpenGL texture.
typedef struct {
    GLuint gl_handle;
    sf_vec2 dimensions;
} sf_texture;
/// Load a texture from a file and upload it to the gpu.
EXPORT sf_result sf_texture_load(sf_texture *out, sf_str path);
static inline sf_result sf_texture_cload(sf_texture *out, const char *path) { return sf_texture_load(out, sf_ref(path)); }
/// Free a texture's resources.
EXPORT void sf_texture_delete(sf_texture *texture);

#endif // TEXTURES_H
