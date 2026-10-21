#include "debug.h"
#include "util.h"
#include "shader.h"
#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>

double g_scroll_delta = 0.0f;
int g_supersampling = 1;

void update_camera(GLFWwindow* window, OrbitalCamera& camera);

int main()
{
    const auto w = 800;
    const auto h = 600;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

    GLFWwindow* window = glfwCreateWindow(w, h, "BRDF Demo", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, w, h);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); });

    glfwSetScrollCallback(window, [](GLFWwindow*, double, double y) { g_scroll_delta = y; });
    glfwSetKeyCallback(window, [](GLFWwindow*, int key, int, int action, int) {
        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            g_supersampling = std::clamp(g_supersampling * 4, 1, 16);
            std::cout << "Supersampling: " << g_supersampling << "\n";
        }
        else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            g_supersampling = std::clamp(g_supersampling / 4, 1, 16);
            std::cout << "Supersampling: " << g_supersampling << "\n";
        }
    });

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

    float quad[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f};

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    OrbitalCamera camera(glm::radians(45.0f), 0.0f, 5.0f, 80.0f);

    {
        // Compile shaders
        Shader vs(read_file("./shaders/vs.glsl"), GL_VERTEX_SHADER);
        Shader fs(read_file("./shaders/fs.glsl"), GL_FRAGMENT_SHADER);
        ShaderProgram shader({vs, fs});

        shader.use();
        shader.set("fov", camera.fov());
        shader.set("view", camera.view_matrix());

        shader.set("supersampling", g_supersampling);

        while (!glfwWindowShouldClose(window))
        {
            g_scroll_delta = 0.0f;
            glfwPollEvents();


            update_camera(window, camera);

            shader.set("supersampling", g_supersampling);
            shader.set("camera_radius", camera.radius());
            shader.set("view", camera.view_matrix());

            // Update window size uniform
            int w, h;
            glfwGetWindowSize(window, &w, &h);
            shader.set("screen_size", glm::vec2((float)w, (float)h));

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glfwSwapBuffers(window);
        }
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}

void update_camera(GLFWwindow* window, OrbitalCamera& camera)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if (!camera.is_dragging())
        {
            camera.start_drag(x, y);
        }
        camera.update(x, y);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
    {
        camera.stop_drag();
    }

    camera.set_radius(camera.radius() * (1.0f + g_scroll_delta * -0.2f));
}
