#ifndef SF_GRAPHICS_H
#define SF_GRAPHICS_H

#include <sf/result.h>
#include <sf/numerics.h>
#include <sf/dynamic.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "sf/input.h"
#include "export.h"

/// A movable camera.
typedef struct {
    sf_transform transform;
    float fov;
    mat4 projection;
    GLuint framebuffer;
} sf_camera;

/// Create a new camera with its own framebuffer.
EXPORT sf_camera sf_camera_new(float fov, mat4 projection);
/// Delete a camera and its framebuffer.
EXPORT void sf_camera_free(const sf_camera *camera);

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
/// Set a shader's vector2 uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_vec2(sf_shader *shader, sf_str name, sf_vec2 value);
/// Set a shader's vector3 uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_vec3(sf_shader *shader, sf_str name, sf_vec3 value);
/// Set a shader's matrix uniform to the desired value by name.
[[nodiscard]] EXPORT sf_result sf_shader_uniform_mat4(sf_shader *shader, sf_str name, mat4 value);

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

#define sf_rgbagl(rgba) ((sf_glcolor){ rgba.r/255.0f, rgba.g/255.0f, rgba.b/255.0f, rgba.a/255.0f})
#define sf_glrgba(gl) ((sf_rgba){ (uint8_t)(gl.r * 255), (uint8_t)(gl.g * 255), (uint8_t)(gl.b * 255), (uint8_t)(gl.a * 255) })
static inline sf_str sf_glcolor_str(const sf_glcolor gl) {
    return sf_str_fmt("{ %f, %f, %f, %f }",
        (double)gl.r,
        (double)gl.g,
        (double)gl.b,
        (double)gl.a
    );
}

/// Contains vertex data for composing a mesh.
#pragma pack(push, 1)
typedef struct {
    sf_vec3 position;
    sf_vec2 uv;
    sf_rgba color;
} sf_vertex;
#pragma pack(pop)

/// A bitfield containing information about an active mesh.
typedef uint8_t sf_mesh_flags;
#define sf_MESH_ACTIVE (sf_mesh_flags)0b10000000
#define sf_MESH_VISIBLE (sf_mesh_flags)0b01000000

/// A mesh containing data for drawing a 3d model of any variety.
typedef struct {
    GLuint vao, vbo;
    sf_vec vertices; /// Should contain no more than INT_MAX vertices.
    sf_mesh_flags flags;
} sf_mesh;

/// Create a new, empty mesh.
[[nodiscard]] EXPORT sf_mesh sf_mesh_new();
/// Free a mesh and delete all of its vertices.
EXPORT void sf_mesh_free(sf_mesh *mesh);

/// Copy a mesh to vram (Vertex Buffer)
EXPORT void sf_mesh_update(const sf_mesh *mesh);
/// Add a single vertex to a mesh's model.
static inline void sf_mesh_add_vertex(sf_mesh *mesh, const sf_vertex vertex) {
    sf_vec_push(&mesh->vertices, &vertex);
    sf_mesh_update(mesh);
}
/// Add an array of vertices to a mesh's model.
static inline void sf_mesh_add_vertices(sf_mesh *mesh, const sf_vertex *vertices, const size_t count) {
    sf_vec_append(&mesh->vertices, vertices, count);
    sf_mesh_update(mesh);
}

/// Draw a mesh to the view of a specific camera.
EXPORT sf_result sf_mesh_draw(const sf_mesh *mesh, sf_shader *shader, sf_camera *camera, sf_transform transform);

typedef struct {
    sf_transform *node;
    sf_mesh *mesh;
    sf_shader *shader;
} sf_renderer;

/// Turns an sf_transform into a model matrix.
EXPORT void sf_transform_model(mat4 out, sf_transform transform);

#endif // SF_GRAPHICS_H
