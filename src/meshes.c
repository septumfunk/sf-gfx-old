#include "sf/meshes.h"

#include "sf/camera.h"

#define CLEAN_BIND true

sf_mesh sf_mesh_new() {
    sf_mesh mesh = {
        .vertices = sf_vec_new(sf_vertex),
        .flags = SF_MESH_ACTIVE | SF_MESH_VISIBLE,
    };

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    // Vertex Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), nullptr);
    // UV Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    // Vertex Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));

    if (CLEAN_BIND) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    sf_opengl_log();

    return mesh;
}

void sf_mesh_free(sf_mesh *mesh) {
    sf_vec_delete(&mesh->vertices);

    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);

    mesh->flags &= ~SF_MESH_ACTIVE;
    mesh->flags &= ~SF_MESH_VISIBLE;
}

void sf_mesh_update(const sf_mesh *mesh) {
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    int64_t c = (int64_t)(mesh->vertices.count * mesh->vertices.element_size);
    glBufferData(GL_ARRAY_BUFFER, c, mesh->vertices.data, GL_DYNAMIC_DRAW);

    if (CLEAN_BIND) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

sf_result sf_mesh_draw(const sf_mesh *mesh, sf_shader *shader, sf_camera *camera, const sf_transform transform) {
    sf_shader_bind(shader);

    sf_result res = sf_shader_uniform_mat4(shader, sf_lit("m_projection"), camera->projection);
    if (!res.ok)
        return res;

    mat4 campos;
    sf_transform cp = camera->transform;
    cp.position = (sf_vec3){-cp.position.x, -cp.position.y, -cp.position.z};
    sf_transform_model(campos, cp);
    res = sf_shader_uniform_mat4(shader, sf_lit("m_campos"), campos);
    if (!res.ok)
        return res;

    mat4 model;
    sf_transform_model(model, transform);
    res = sf_shader_uniform_mat4(shader, sf_lit("m_model"), model);
    if (!res.ok)
        return res;

    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_TRIANGLES, 0, (int)mesh->vertices.count);
    if (CLEAN_BIND)
        glBindVertexArray(0);

    return sf_ok();
}
