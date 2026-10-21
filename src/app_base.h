#pragma once

#include "debug.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

class AppBase
{
public:
    AppBase(int width, int height, const char* title);

    inline virtual ~AppBase()
    {
        glfwTerminate();
    }

    inline virtual void on_start() = 0;
    inline virtual void on_update() = 0;
    inline virtual void on_render() = 0;

    inline virtual void on_resize(int width, int height)
    {
        glViewport(0, 0, width, height);
    }
    inline virtual void on_mouse_move(double x, double y)
    {}
    inline virtual void on_mouse_click(int button, double x, double y)
    {}
    inline virtual void on_mouse_release(int button, double x, double y)
    {}
    inline virtual void on_mouse_scroll(double dx, double dy)
    {}
    inline virtual void on_key_press(int key)
    {}
    inline virtual void on_key_release(int key)
    {}

    void run();


protected:
    GLFWwindow* m_window{nullptr};

private:
    static void resize_callback(GLFWwindow* window, int w, int h);
    static void mouse_move_callback(GLFWwindow* window, double x, double y);
    static void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_scroll_callback(GLFWwindow* window, double dx, double dy);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};