#include "debug.h"
#include "util.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

double scroll_delta = 0.0f;


class Shader
{
public:
    Shader(const std::string& src, GLenum type);
    ~Shader();
    unsigned int id() const;


private:
    unsigned int m_id{0};
};

class Program
{
public:
    Program(std::initializer_list<Shader> shaders);

    ~Program();
    void use();

    void set(const std::string& name, glm::mat4 mat);
    void set(const std::string& name, glm::vec3 vec);
    void set(const std::string& name, glm::vec2 vec);
    void set(const std::string& name, float value);

private:
    unsigned int m_id{0};
};

class OrbitalCamera
{
public:
    OrbitalCamera(float pitch, float yaw, float radius = 1.0f, float fov = glm::radians(90.0f)) :
        m_pitch(pitch),
        m_yaw(yaw),
        m_fov(fov),
        m_radius(radius),
        m_is_dragged(false),
        m_pitch_start(pitch),
        m_yaw_start(yaw),
        m_x_start(0),
        m_y_start(0)
    {}

    glm::mat4 view_matrix() const
    {
        return glm::eulerAngleYXZ(m_yaw, m_pitch, 0.0f);
    }

    float fov() const
    {
        return m_fov;
    }

    void set_fov(float fov)
    {
        m_fov = fov;
    }

    float radius() const
    {
        return m_radius;
    }

    void set_radius(float r)
    {
        m_radius = std::clamp(r, 0.1f, 50.0f);
    }


    bool is_dragging() const
    {
        return m_is_dragged;
    }

    void start_drag(float x, float y)
    {
        m_is_dragged = true;
        m_x_start = x;
        m_y_start = y;
        m_pitch_start = m_pitch;
        m_yaw_start = m_yaw;
    }

    void stop_drag()
    {
        m_is_dragged = false;
    }

    void update(float x, float y)
    {
        const auto sensitivity = 0.002f;
        auto deltax = x - m_x_start;
        auto deltay = y - m_y_start;
        m_yaw = m_yaw_start + deltax * sensitivity;
        m_pitch = m_pitch_start + deltay * sensitivity;

        m_pitch = std::clamp(m_pitch, glm::radians(-89.0f), glm::radians(89.0f));
    }

private:
    float m_pitch, m_yaw, m_fov, m_radius;
    bool m_is_dragged;
    float m_pitch_start, m_yaw_start;
    float m_x_start, m_y_start;
};


unsigned int compile_shader(const std::string& src, GLenum type);

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

    glfwSetScrollCallback(window, [](GLFWwindow*, double, double y) { scroll_delta = y; });

#ifndef NDEBUG
    // Init debugging
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

    OrbitalCamera camera(0.0f, 0.0f, 5.0f, 80.0f);

    {
        // Compile shaders
        Shader vs(read_file("./shaders/vs.glsl"), GL_VERTEX_SHADER);
        Shader fs(read_file("./shaders/fs.glsl"), GL_FRAGMENT_SHADER);
        Program program({vs, fs});

        program.use();


        program.set("fov", camera.fov());
        program.set("view", camera.view_matrix());


        while (!glfwWindowShouldClose(window))
        {
            scroll_delta = 0.0f;
            glfwPollEvents();

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


            camera.set_radius(camera.radius() * (1.0f + scroll_delta * -0.2f));

            program.set("camera_radius", camera.radius());

            program.set("view", camera.view_matrix());

            // Update window size uniform
            int w, h;
            glfwGetWindowSize(window, &w, &h);
            program.set("screen_size", glm::vec2((float)w, (float)h));

            glDrawArrays(GL_TRIANGLES, 0, 6);
            glfwSwapBuffers(window);
        }
    }

    glfwTerminate();
    return 0;
}


Shader::Shader(const std::string& src, GLenum type)
{
    m_id = glCreateShader(type);
    const auto src_c = src.c_str();
    glShaderSource(m_id, 1, &src_c, nullptr);
    glCompileShader(m_id);

    int success;
    char info[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(m_id, 512, NULL, info);
        std::cerr << "Shader compile error!\n" << info << "\n";
    }
}

Shader::~Shader()
{
    glDeleteShader(m_id);
}

unsigned int Shader::id() const
{
    return m_id;
}


Program::Program(std::initializer_list<Shader> shaders)
{
    m_id = glCreateProgram();
    for (const auto& shader : shaders)
    {
        glAttachShader(m_id, shader.id());
    }

    glLinkProgram(m_id);

    int success;
    char info[512];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_id, 512, NULL, info);
        std::cerr << "Shader link error!\n" << info << "\n";
    }
}

Program::~Program()
{
    glDeleteProgram(m_id);
}

void Program::use()
{
    glUseProgram(m_id);
}

void Program::set(const std::string& name, glm::mat4 mat)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Program::set(const std::string& name, glm::vec3 vec)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void Program::set(const std::string& name, glm::vec2 vec)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(vec));
}


void Program::set(const std::string& name, float value)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(loc, value);
}
