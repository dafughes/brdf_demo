#include "app_base.h"

AppBase::AppBase(int width, int height, const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, width, height);

#ifndef NDEBUG
    // Setup debugging
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glfwSetFramebufferSizeCallback(m_window, resize_callback);
    glfwSetCursorPosCallback(m_window, mouse_move_callback);
    glfwSetMouseButtonCallback(m_window, mouse_click_callback);
    glfwSetScrollCallback(m_window, mouse_scroll_callback);
    glfwSetKeyCallback(m_window, key_callback);
}


void AppBase::run()
{
    on_start();
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        on_update();
        on_render();
        glfwSwapBuffers(m_window);
    }
}


void AppBase::resize_callback(GLFWwindow* window, int w, int h)
{
    auto app = static_cast<AppBase*>(glfwGetWindowUserPointer(window));
    app->on_resize(w, h);
}
void AppBase::mouse_move_callback(GLFWwindow* window, double x, double y)
{
    auto app = static_cast<AppBase*>(glfwGetWindowUserPointer(window));
    app->on_mouse_move(x, y);
}
void AppBase::mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto app = static_cast<AppBase*>(glfwGetWindowUserPointer(window));

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)
    {
        app->on_mouse_click(button, x, y);
    }
    else if (action == GLFW_RELEASE)
    {
        app->on_mouse_release(button, x, y);
    }
}
void AppBase::mouse_scroll_callback(GLFWwindow* window, double dx, double dy)
{
    auto app = static_cast<AppBase*>(glfwGetWindowUserPointer(window));
    app->on_mouse_scroll(dx, dy);
}
void AppBase::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppBase*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS)
    {
        app->on_key_press(key);
    }
    else if (action == GLFW_RELEASE)
    {
        app->on_key_release(key);
    }
}
