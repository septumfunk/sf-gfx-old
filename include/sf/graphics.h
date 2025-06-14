#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <sf/result.h>
#include <sf/numerics.h>
#include <sf/dynamic.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
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
EXPORT void sf_camera_free(sf_camera *camera);

/// A window with an active OpenGL context and keyboard controls.
typedef struct {
    GLFWwindow *handle;
    sf_str title;
    sf_vec2 size;
    sf_vec2 mouse_position;

    sf_camera *camera;

    int8_t keyboard[GLFW_KEY_LAST + 1];
    uint8_t kb_p;
    char keyboard_string[UINT8_MAX];
} sf_window;

/// Construct and open a new OpenGL window.
[[nodiscard]] EXPORT sf_result sf_window_new(sf_window **out, const sf_str title, const sf_vec2 size);
/// Free a window and its resources.
EXPORT void sf_window_free(sf_window *window);

/// Check is a key is currently pressed down.
static inline bool sf_key_check(sf_window *window, sf_key key)    { return window->keyboard[key] > 0;                }
/// Check if a key was pressed on this frame.
static inline bool sf_key_pressed(sf_window *window, sf_key key)  { return window->keyboard[key] == SF_KEY_PRESSED; }
/// Check if a key was released on this frame.
static inline bool sf_key_released(sf_window *window, sf_key key) { return window->keyboard[key] == SF_KEY_RELEASED; }
/// Get the string of keys pressed since the last time this function was called.
[[nodiscard]] sf_str sf_key_string(sf_window *window);

/// Prepare for a frame, and/or return whether a window should close.
/// Use this in a while loop.
EXPORT bool sf_window_loop(sf_window *window);
/// Swap a window's buffers and finish the frame.
EXPORT void sf_window_draw(sf_window *window, sf_camera *camera);

/// Set the displayed title of a window.
EXPORT void sf_window_set_title(sf_window *window, const sf_str title);
/// Set the displayed size of a window.
EXPORT void sf_window_set_size(sf_window *window, sf_vec2 size);

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
static inline void sf_shader_bind(sf_shader *shader) { glUseProgram(shader->program); }

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
EXPORT void sf_mesh_update(sf_mesh *mesh);
/// Add a single vertex to a mesh's model.
static inline void sf_mesh_add_vertex(sf_mesh *mesh, sf_vertex vertex) {
    sf_vec_push(&mesh->vertices, &vertex);
    sf_mesh_update(mesh);
}
/// Add an array of vertices to a mesh's model.
static inline void sf_mesh_add_vertices(sf_mesh *mesh, sf_vertex *vertices, size_t count) {
    sf_vec_append(&mesh->vertices, vertices, count);
    sf_mesh_update(mesh);
}

/// Draw a mesh to the view of a specific camera.
EXPORT sf_result sf_mesh_draw(sf_mesh *mesh, sf_shader *shader, sf_camera *camera, sf_transform transform);

typedef struct {
    sf_transform *node;
    sf_mesh *mesh;
    sf_shader *shader;
} sf_renderer;

/// Turns an sf_transform into a model matrix.
EXPORT void sf_transform_model(mat4 out, const sf_transform transform);

#endif // GRAPHICS_H
