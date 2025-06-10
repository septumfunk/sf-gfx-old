#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <sf/result.h>
#include <sf/files.h>
#include "sf/graphics.h"
#include "cglm/affine-pre.h"
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "sf/numerics.h"

#define CLEAN_BIND true

sf_camera sf_camera_new(float fov, mat4 projection) {
    sf_camera cam = {};
    cam.fov = fov;
    memcpy(&cam.projection, projection, sizeof(mat4));
    glGenFramebuffers(1, &cam.framebuffer);
    return cam;
}

void sf_camera_free(sf_camera *camera) {
    glDeleteFramebuffers(1, &camera->framebuffer);
}

void sf_cb_err(int error_code, const char *error_string) {
    fprintf(stderr, "OpenGL Error %d: '%s.'", error_code, error_string);
}

void sf_cb_key(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
    sf_window *win = glfwGetWindowUserPointer(window);
    switch (action) {
        case GLFW_RELEASE: win->keyboard[key] = SF_KEY_RELEASED; break;
        case GLFW_PRESS: win->keyboard[key] = SF_KEY_PRESSED; break;
    }
}

void sf_cb_char(GLFWwindow* window, unsigned int codepoint) {
    sf_window *win = glfwGetWindowUserPointer(window);
    if (codepoint <= 127) {
        if (win->kb_p == UINT8_MAX - 1)
            memcpy(win->keyboard_string, win->keyboard_string + 1, UINT8_MAX - 2);
        win->keyboard_string[win->kb_p] = (char)codepoint;
        if (win->kb_p < UINT8_MAX - 1) win->kb_p++;
    }
}

sf_result sf_window_new(sf_window **out, const sf_str title, const sf_vec2 size, sf_camera *camera) {
    *out = calloc(1, sizeof(sf_window));
    memcpy(*out, &(sf_window) {
        .title = sf_str_dup(title),
        .size = size,
        .mouse_position = { 0, 0 },
        .camera = camera,
    }, sizeof(sf_window));
    sf_window *win = *out;

    //TODO: GLFW Init
    if (!glfwInit()) {
        glfwTerminate();
        return sf_err(sf_lit("GLFW Failed to initialize."));
    }
    if (!(win->handle = glfwCreateWindow((int)size.x, (int)size.y, title.c_str, nullptr, nullptr)))
        return sf_err(sf_lit("GLFW Failed to open the window."));

    glfwSetWindowUserPointer(win->handle, win); // Point to myself
    glfwSetErrorCallback(sf_cb_err);
    glfwSetKeyCallback(win->handle, sf_cb_key);
    glfwSetCharCallback(win->handle, sf_cb_char);

    glfwMakeContextCurrent(win->handle);
    if (!gladLoadGL())
        return sf_err(sf_lit("GLAD Failed to initialize!"));

    glViewport(0, 0, (int)size.x, (int)size.y);

    return sf_ok();
}

void sf_window_free(sf_window *window) {
    sf_str_free(window->title);
    //TODO: GLFW Cleanup
}

sf_str sf_key_string(sf_window *window) {
    sf_str str = sf_str_cdup(window->keyboard_string);
    memset(window->keyboard_string, 0, 64);
    window->kb_p = 0;
    return str;
}

bool sf_window_loop(sf_window *window) {
    //TODO: Prepare for frame.
    glfwPollEvents();
    return !glfwWindowShouldClose(window->handle);
}

void sf_window_swap(sf_window *window) {
    glfwSwapBuffers(window->handle);
    for (int i = 0; i < SF_KEY_COUNT; ++i) {
        if (window->keyboard[i] == SF_KEY_PRESSED)
            window->keyboard[i] = SF_KEY_DOWN;
        if (window->keyboard[i] == SF_KEY_RELEASED)
            window->keyboard[i] = 0;
    }
}

void sf_window_set_title(sf_window *window, const sf_str title) {
    sf_str_free(window->title);
    window->title = sf_str_dup(title);
    glfwSetWindowTitle(window->handle, title.c_str);
}

void sf_window_set_size(sf_window *window, sf_vec2 size) {
    window->size = size;
    glfwSetWindowSize(window->handle, (int)size.x, (int)size.y);
}

sf_result sf_load_shader(GLuint *out, GLenum type, sf_str path) {
    sf_result res = sf_ok();
    sf_str spath = sf_str_fmt("%s.%s", path.c_str, type == GL_FRAGMENT_SHADER ? "frag" : "vert");
    uint8_t *sbuffer = nullptr;

    *out = glCreateShader(type);

    long s = sf_file_size(spath);
    if (s <= 0) {
        res = sf_err(sf_str_fmt("Failed to find vertex shader '%s'", spath.c_str));
        goto cleanup;
    }

    sbuffer = malloc((size_t)s + 1);
    res = sf_load_file(sbuffer, spath);
    if (!res.ok) goto cleanup;
    sbuffer[s] = '\0';

    glShaderSource(*out, 1, (const GLchar **)&sbuffer, NULL);
    glCompileShader(*out);

    int success;
    glGetShaderiv(*out, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(*out, 512, NULL, log);
        res = sf_err(sf_str_fmt("Failed to compile shader '%s': %s", spath.c_str, log));
        goto cleanup;
    }

cleanup:
    sf_str_free(spath);
    if (sbuffer) free(sbuffer);
    return res;
}

sf_result sf_shader_new(sf_shader *out, sf_str path) {
    sf_result res = sf_ok();

    memset(out, 0, sizeof(sf_shader));

    res = sf_load_shader(&out->vertex, GL_VERTEX_SHADER, path);
    if (!res.ok)
        goto result;
    res = sf_load_shader(&out->fragment, GL_FRAGMENT_SHADER, path);
    if (!res.ok) {
        glDeleteShader(out->vertex);
        goto result;
    }

    out->program = glCreateProgram();
    glAttachShader(out->program, out->vertex);
    glAttachShader(out->program, out->fragment);
    glLinkProgram(out->program);

    int success;
    glGetProgramiv(out->program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(out->program, 512, NULL, log);

        glDeleteShader(out->vertex);
        glDeleteShader(out->fragment);
        res = sf_err(sf_str_fmt("Failed to link shader '%s': %s", path, log));
        goto result;
    }

    out->uniforms = sf_map_new();
    out->path = sf_str_dup(path);

result:
    return res;
}

void sf_shader_free(sf_shader *shader) {
    sf_str_free(shader->path);

    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);

    glDeleteProgram(shader->program);

    sf_map_delete(&shader->uniforms);
}

sf_result sf_get_uniform(sf_shader *shader, GLint *out, sf_str name) {
    if (sf_map_exists(&shader->uniforms, name)) {
        *out = sf_map_get(&shader->uniforms, GLint, name);
        return sf_ok();
    }

    GLint loc = glGetUniformLocation(shader->program, name.c_str);
    if (loc < 0)
        return sf_err(sf_str_fmt("Uniform '%s' not found.", name.c_str));
    sf_map_insert(&shader->uniforms, name, &loc);
    *out = loc;

    return sf_ok();
}

sf_result sf_shader_uniform_float(sf_shader *shader, sf_str name, float value) {
    GLint uf;
    sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform1f(uf, value);

    return sf_ok();
}

sf_result sf_shader_uniform_vec2(sf_shader *shader, sf_str name, sf_vec2 value) {
    GLint uf;
    sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform2f(uf, value.x, value.y);

    return sf_ok();
}

sf_result sf_shader_uniform_vec3(sf_shader *shader, sf_str name, sf_vec3 value) {
    GLint uf;
    sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform3f(uf, value.x, value.y, value.z);

    return sf_ok();
}

sf_result sf_shader_uniform_mat4(sf_shader *shader, sf_str name, mat4 value) {
    GLint uf;
    sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniformMatrix4fv(uf, 1, false, (const GLfloat *)value);

    return sf_ok();
}

sf_mesh sf_mesh_new() {
    sf_mesh mesh = {
        .vertices = sf_vec_new(sf_vertex),
        .flags = sf_MESH_ACTIVE | sf_MESH_VISIBLE,
    };

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBindVertexArray(mesh.vao);
    // Vertex Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
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

    mesh->flags &= ~sf_MESH_ACTIVE;
    mesh->flags &= ~sf_MESH_VISIBLE;
}

void sf_mesh_update(sf_mesh *mesh) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, (int64_t)(mesh->vertices.count * mesh->vertices.element_size), mesh->vertices.data, GL_DYNAMIC_DRAW);
}

sf_result sf_mesh_draw(sf_mesh *mesh, sf_shader *shader, sf_camera *camera, sf_transform transform) {
    sf_shader_bind(shader);

    sf_result res;
    res = sf_shader_uniform_mat4(shader, sf_lit("m_projection"), camera->projection);
    if (!res.ok)
        return res;

    mat4 campos;
    sf_transform_model(campos, camera->transform);
    res = sf_shader_uniform_mat4(shader, sf_lit("m_campos"), campos);
    if (!res.ok)
        return res;

    mat4 model;
    sf_transform_model(model, transform);
    res = sf_shader_uniform_mat4(shader, sf_lit("m_model"), model);
    if (!res.ok)
        return res;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glDrawArrays(GL_TRIANGLES, 0, (int)mesh->vertices.count);
    if (CLEAN_BIND)
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    return sf_ok();
}

void sf_transform_model(mat4 out, const sf_transform transform) {
    glm_mat4_identity(out);
    glm_scale(out, (vec3){transform.scale.x, transform.scale.y, transform.scale.z});
    glm_rotate_at(out, (vec3){0, 0, 0}, transform.rotation.x, (vec3){1, 0, 0});
    glm_rotate_at(out, (vec3){0, 0, 0}, transform.rotation.x, (vec3){1, 0, 0});
    glm_rotate_at(out, (vec3){0, 0, 0}, transform.rotation.y, (vec3){0, 1, 0});
    glm_rotate_at(out, (vec3){0, 0, 0}, transform.rotation.z, (vec3){0, 0, 1});
    glm_translate(out, (vec3){transform.position.x, transform.position.y, transform.position.z});
}
