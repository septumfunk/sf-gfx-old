#ifndef CAMERA_H
#define CAMERA_H

#include <sf/numerics.h>
#include <cglm/cglm.h>
#include "export.h"
#include "glad/glad.h"

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

#endif // CAMERA_H
