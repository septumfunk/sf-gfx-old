#include "sf/camera.h"

sf_camera sf_camera_new(const float fov, mat4 projection) {
    sf_camera cam = {};
    cam.fov = fov;
    cam.transform.scale = (sf_vec3){1, 1, 1};
    memcpy(&cam.projection, projection, sizeof(mat4));
    glGenFramebuffers(1, &cam.framebuffer);
    return cam;
}

void sf_camera_free(const sf_camera *camera) {
    glDeleteFramebuffers(1, &camera->framebuffer);
}
