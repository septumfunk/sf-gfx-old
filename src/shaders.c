#include <sf/numerics.h>
#include <sf/fs.h>
#include "sf/shaders.h"

sf_result sf_load_shader(GLuint *out, const GLenum type, const sf_str path) {
    sf_result res;
    const sf_str spath = sf_str_fmt("%s.%s", path.c_str, type == GL_FRAGMENT_SHADER ? "frag" : "vert");
    uint8_t *sbuffer = nullptr;

    *out = glCreateShader(type);

    const long s = sf_file_size(spath);
    if (s <= 0) {
        res = sf_err(sf_str_fmt("Failed to find vertex shader '%s'", spath.c_str));
        goto cleanup;
    }

    sbuffer = sf_malloc((size_t)s + 1);
    res = sf_load_file(sbuffer, spath);
    if (!res.ok) goto cleanup;
    sbuffer[s] = '\0';

    glShaderSource(*out, 1, (const GLchar **)&sbuffer, nullptr);
    glCompileShader(*out);

    int success;
    glGetShaderiv(*out, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(*out, 512, nullptr, log);
        res = sf_err(sf_str_fmt("Failed to compile shader '%s': %s", spath.c_str, log));
    }

cleanup:
    sf_str_free(spath);
    if (sbuffer) free(sbuffer);
    return res;
}

sf_result sf_shader_new(sf_shader *out, const sf_str path) {
    memset(out, 0, sizeof(sf_shader));

    sf_result res = sf_load_shader(&out->vertex, GL_VERTEX_SHADER, path);
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
        glGetProgramInfoLog(out->program, 512, nullptr, log);

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

sf_result sf_get_uniform(sf_shader *shader, GLint *out, const sf_str name) {
    if (sf_map_exists(&shader->uniforms, name)) {
        *out = sf_map_get(&shader->uniforms, GLint, name);
        return sf_ok();
    }

    const GLint loc = glGetUniformLocation(shader->program, name.c_str);
    if (loc < 0)
        return sf_err(sf_str_fmt("Uniform '%s' not found.", name.c_str));
    sf_map_insert(&shader->uniforms, name, &loc);
    *out = loc;

    return sf_ok();
}

sf_result sf_shader_uniform_float(sf_shader *shader, const sf_str name, const float value) {
    GLint uf;
    const sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform1f(uf, value);

    return sf_ok();
}

sf_result sf_shader_uniform_vec2(sf_shader *shader, const sf_str name, const sf_vec2 value) {
    GLint uf;
    const sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform2f(uf, value.x, value.y);

    return sf_ok();
}

sf_result sf_shader_uniform_vec3(sf_shader *shader, const sf_str name, const sf_vec3 value) {
    GLint uf;
    const sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniform3f(uf, value.x, value.y, value.z);

    return sf_ok();
}

sf_result sf_shader_uniform_mat4(sf_shader *shader, const sf_str name, mat4 value) {
    GLint uf;
    const sf_result res = sf_get_uniform(shader, &uf, name);
    if (!res.ok)
        return res;
    glUniformMatrix4fv(uf, 1, false, (const GLfloat *)value);

    return sf_ok();
}

void sf_transform_model(mat4 out, const sf_transform transform) {
    glm_mat4_identity(out);
    glm_scale(out, (vec3){transform.scale.x, transform.scale.y, transform.scale.z});
    glm_rotate(out, transform.rotation.x, (vec3){1, 0, 0});
    glm_rotate(out, transform.rotation.y, (vec3){0, 1, 0});
    glm_rotate(out, transform.rotation.z, (vec3){0, 0, 1});
    glm_translate(out, (vec3){transform.position.x, transform.position.y, transform.position.z});
}

void sf_transform_view(mat4 out, const sf_transform transform) {
    sf_transform_model(out, transform);
    glm_mat4_inv(out, out);
}
