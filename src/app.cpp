#include "app.h"
#include "util.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>


App::App(int width, int height, const char* title) : AppBase(width, height, title)
{}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void App::on_start()
{
    float quad[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f};

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m_camera = std::make_unique<OrbitalCamera>(glm::radians(45.0f), 0.0f, 5.0f, 80.0f);

    // Compile shaders
    Shader vs(read_file("./shaders/vs.glsl"), GL_VERTEX_SHADER);
    Shader fs(read_file("./shaders/fs.glsl"), GL_FRAGMENT_SHADER);
    auto shaders = {vs, fs};
    m_shader = std::make_unique<ShaderProgram>(shaders);

    m_shader->use();
    m_shader->set("fov", m_camera->fov());
    m_shader->set("view", m_camera->view_matrix());
    m_shader->set("supersampling", m_supersampling);

    m_shader->set("camera_radius", m_camera->radius());

    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    m_shader->set("screen_size", glm::vec2((float)w, (float)h));

    glfwSetWindowUserPointer(m_window, this);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(
        m_window,
        true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void App::on_resize(int w, int h)
{
    AppBase::on_resize(w, h);
    m_shader->set("screen_size", glm::vec2((float)w, (float)h));
}

void App::on_mouse_move(double x, double y)
{
    m_camera->update(x, y);
    m_shader->set("view", m_camera->view_matrix());
}

void App::on_mouse_click(int button, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_1)
        m_camera->start_drag(x, y);
}

void App::on_mouse_release(int button, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_1)
        m_camera->stop_drag();
}

void App::on_update()
{}

void App::on_render()
{
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void App::on_mouse_scroll(double dx, double dy)
{
    m_camera->zoom(dy > 0.0);
    m_shader->set("camera_radius", m_camera->radius());
}
