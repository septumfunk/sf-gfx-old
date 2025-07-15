#ifndef TEXTURES_H
#define TEXTURES_H

#include <sf/dynamic.h>
#include <sf/numerics.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef enum {
    SF_TEXTURE_RGB,
    SF_TEXTURE_RGBA,
    SF_TEXTURE_DEPTH_STENCIL,
} sf_texture_type;

/// A wrapper around an OpenGL texture.
typedef struct {
    sf_texture_type type;
    GLuint handle;
    sf_vec2 dimensions;
} sf_texture;
/// Create an empty OpenGL texture.
EXPORT sf_texture sf_texture_new(sf_texture_type type, sf_vec2 dimensions);
/// Load a texture from a file and upload it to the gpu.
EXPORT sf_result sf_texture_load(sf_texture *out, sf_str path);
static inline sf_result sf_texture_cload(sf_texture *out, const char *path) { return sf_texture_load(out, sf_ref(path)); }
/// Resize a texture without first deleting it.
EXPORT void sf_texture_resize(sf_texture *texture, sf_vec2 dimensions);
/// Free a texture's resources.
EXPORT void sf_texture_delete(sf_texture *texture);

#endif // TEXTURES_H
