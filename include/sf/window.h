#ifndef SF_WINDOW_H
#define SF_WINDOW_H

#include <sf/result.h>
#include <sf/numerics.h>
#include <sf/dynamic.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "sf/graphics.h"
#include "sf/input.h"
#include "export.h"

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
[[nodiscard]] EXPORT sf_result sf_window_new(sf_window **out, sf_str title, sf_vec2 size);
/// Free a window and its resources.
EXPORT void sf_window_free(const sf_window *window);

/// Check is a key is currently pressed down.
static inline bool sf_key_check(const sf_window *window, const sf_key key)    { return window->keyboard[key] > 0;                }
/// Check if a key was pressed on this frame.
static inline bool sf_key_pressed(const sf_window *window, const sf_key key)  { return window->keyboard[key] == SF_KEY_PRESSED; }
/// Check if a key was released on this frame.
static inline bool sf_key_released(const sf_window *window, const sf_key key) { return window->keyboard[key] == SF_KEY_RELEASED; }
/// Get the string of keys pressed since the last time this function was called.
[[nodiscard]] sf_str sf_key_string(sf_window *window);

/// Prepare for a frame, and/or return whether a window should close.
/// Use this in a while loop.
EXPORT bool sf_window_loop(const sf_window *window);
/// Swap a window's buffers and finish the frame.
EXPORT void sf_window_draw(sf_window *window, sf_camera *camera);

/// Set the displayed title of a window.
EXPORT void sf_window_set_title(sf_window *window, const sf_str title);
/// Set the displayed size of a window.
EXPORT void sf_window_set_size(sf_window *window, sf_vec2 size);

#endif // SF_WINDOW_H
