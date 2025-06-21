#ifndef MESHES_H
#define MESHES_H

#include <sf/numerics.h>
#include "sf/camera.h"
#include "sf/shaders.h"
#include "sf/textures.h"

/// Contains vertex data for composing a mesh.
#pragma pack(push, 1)
typedef struct {
    sf_vec3 position;
    sf_vec2 uv;
    sf_glcolor color;
} sf_vertex;
#pragma pack(pop)

/// A bitfield containing information about an active mesh.
typedef uint8_t sf_mesh_flags;
#define SF_MESH_ACTIVE (sf_mesh_flags)0b10000000
#define SF_MESH_VISIBLE (sf_mesh_flags)0b01000000

/// A mesh containing data for drawing a 3d model of any variety.
typedef struct {
    GLuint vao, vbo, ebo;
    sf_vec vertices, indices; /// Should contain no more than INT_MAX vertices.
    sf_map cache;
    sf_mesh_flags flags;
} sf_mesh;

/// Create a new, empty mesh.
[[nodiscard]] EXPORT sf_mesh sf_mesh_new();
/// Free a mesh and delete all of its vertices.
EXPORT void sf_mesh_delete(sf_mesh *mesh);

/// Copy a mesh to vram (Vertex Buffer)
EXPORT void sf_mesh_update(const sf_mesh *mesh);
/// Add a single vertex to a mesh's model.
EXPORT void sf_mesh_add_vertex(sf_mesh *mesh, sf_vertex vertex);
/// Add an array of vertices to a mesh's model.
EXPORT void sf_mesh_add_vertices(sf_mesh *mesh, const sf_vertex *vertices, size_t count);

/// Draw a mesh to the view of a specific camera.
EXPORT sf_result sf_mesh_draw(const sf_mesh *mesh, sf_shader *shader, sf_camera *camera, sf_transform transform, const sf_texture *texture);

#endif // MESHES_H
