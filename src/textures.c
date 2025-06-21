#include <sf/fs.h>
#include "sf/textures.h"
#include "stb/stb_image.h"

sf_result sf_texture_load(sf_texture *out, const sf_str path) {
    *out = (sf_texture){};

    if (!sf_file_exists(path))
        return sf_err(sf_str_fmt("File '%s' does not exist.", path.c_str));

    int width, height, channels;
    uint8_t *buffer = stbi_load(path.c_str, &width, &height, &channels, 4 /* RGBA */);
    if (!buffer)
        return sf_err(sf_str_fmt("File '%s' could not be loaded.", path.c_str));
    out->dimensions = (sf_vec2){(float)width, (float)height};

    glGenTextures(1, &out->gl_handle);
    glBindTexture(GL_TEXTURE_2D, out->gl_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)out->dimensions.x,
    (int)out->dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return sf_ok();
}

void sf_texture_delete(sf_texture *texture) {
    glDeleteTextures(1, &texture->gl_handle);
    texture->dimensions = (sf_vec2){0, 0};
}
