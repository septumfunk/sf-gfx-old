#include <sf/dynamic.h>
#include "sf/window.h"

#include "sf/shaders.h"

void sf_cb_err(const int error_code, const char *error_string) {
    fprintf(stderr, "OpenGL Error %d: '%s.'", error_code, error_string);
}

void sf_cb_key(GLFWwindow* window, const int key, [[maybe_unused]] int scancode, const int action, [[maybe_unused]] int mods) {
    sf_window *win = glfwGetWindowUserPointer(window);
    switch (action) {
        case GLFW_RELEASE: win->keyboard[key] = SF_KEY_RELEASED; break;
        case GLFW_PRESS: win->keyboard[key] = SF_KEY_PRESSED; break;
        default: break;
    }
}

void sf_cb_char(GLFWwindow* window, const unsigned int codepoint) {
    sf_window *win = glfwGetWindowUserPointer(window);
    if (codepoint <= 127) {
        if (win->kb_p == UINT8_MAX - 1)
            memcpy(win->keyboard_string, win->keyboard_string + 1, UINT8_MAX - 2);
        win->keyboard_string[win->kb_p] = (char)codepoint;
        if (win->kb_p < UINT8_MAX - 1) win->kb_p++;
    }
}

void APIENTRY sf_gl_dbglog(GLuint source, GLuint type, GLuint id, GLuint severity,
    [[maybe_unused]] GLsizei length, const GLchar *message, [[maybe_unused]] const void *userParam) {
    printf("[OpenGL] (Source %u) (Type %u) (ID %u), (Severity %u) \"%s\"\n", source, type, id, severity, message);
}

sf_result sf_window_new(sf_window **out, const sf_str title, const sf_vec2 size) {
    *out = sf_calloc(1, sizeof(sf_window));
    memcpy(*out, &(sf_window) {
        .title = sf_str_dup(title),
        .size = size,
        .mouse_position = { 0, 0 },
    }, sizeof(sf_window));
    sf_window *win = *out;

    //TODO: GLFW Init
    if (!glfwInit()) {
        glfwTerminate();
        return sf_err(sf_lit("GLFW Failed to initialize."));
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (!((win->handle = glfwCreateWindow((int)size.x, (int)size.y, title.c_str, nullptr, nullptr))))
        return sf_err(sf_lit("GLFW Failed to open the window."));

    glfwSetWindowUserPointer(win->handle, win); // Point to myself
    glfwSetErrorCallback(sf_cb_err);
    glfwSetKeyCallback(win->handle, sf_cb_key);
    glfwSetCharCallback(win->handle, sf_cb_char);

    glfwMakeContextCurrent(win->handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return sf_err(sf_lit("GLAD Failed to initialize!"));

    glViewport(0, 0, (int)size.x, (int)size.y);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(sf_gl_dbglog, nullptr);
    glEnable(GL_DEPTH_TEST);

    glfwShowWindow(win->handle);
    return sf_ok();
}

void sf_window_free(const sf_window *window) {
    sf_str_free(window->title);
    //TODO: GLFW Cleanup
}

sf_str sf_key_string(sf_window *window) {
    const sf_str str = sf_str_cdup(window->keyboard_string);
    memset(window->keyboard_string, 0, 64);
    window->kb_p = 0;
    return str;
}

bool sf_window_loop(const sf_window *window) {
    //TODO: Prepare for frame.
    sf_opengl_log();
    glfwPollEvents();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return !glfwWindowShouldClose(window->handle);
}

void sf_window_draw(sf_window *window, [[maybe_unused]] sf_camera *camera) {
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

void sf_window_set_size(sf_window *window, const sf_vec2 size) {
    window->size = size;
    glfwSetWindowSize(window->handle, (int)size.x, (int)size.y);
}
