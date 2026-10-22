#include "app.h"
#include "util.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const glm::vec3 vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

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

    Shader vs(read_file("shaders/vs.glsl"), GL_VERTEX_SHADER);
    Shader fs(read_file("shaders/fs.glsl"), GL_FRAGMENT_SHADER);
    Shader phong(read_file("shaders/phong.glsl"), GL_FRAGMENT_SHADER);

    m_supersampling = Select<int>("Supersampling", 1, 4, 16);

    auto phong_components = {vs, fs, phong};
    m_brdfs = Select<std::unique_ptr<BRDF>>(
        "BRDF",
        std::make_unique<Phong>(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), 1.0f, phong_components));


    m_brdfs.selected()->shader().use();

    m_brdfs.selected()->shader().set("fov", m_camera->fov());
    m_brdfs.selected()->shader().set("view", m_camera->view_matrix());
    m_brdfs.selected()->shader().set("camera_radius", m_camera->radius());
    m_brdfs.selected()->shader().set("light_pos", m_light_pos);
    m_brdfs.selected()->shader().set("light_intensity", m_light_intensity);
    m_brdfs.selected()->shader().set("supersampling", m_supersampling.selected());
    m_brdfs.selected()->update_uniforms();


    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    m_brdfs.selected()->shader().set("screen_size", glm::vec2((float)w, (float)h));

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
    m_brdfs.selected()->shader().set("screen_size", glm::vec2((float)w, (float)h));
}

void App::on_mouse_move(double x, double y)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        m_camera->update(x, y);
        m_brdfs.selected()->shader().set("view", m_camera->view_matrix());
    }
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
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene settings");

    if (m_supersampling.update())
    {
        m_brdfs.selected()->shader().set("supersampling", m_supersampling.selected());
    }

    if (m_brdfs.update())
    {
        std::cout << m_brdfs.selected() << "\n";
    }

    ImGui::SeparatorText("Light");


    if (ImGui::SliderFloat3("Light position", glm::value_ptr(m_light_pos), -30.0f, 30.0f))
    {
        m_brdfs.selected()->shader().set("light_pos", m_light_pos);
    }

    if (ImGui::SliderFloat3("Light intensity", glm::value_ptr(m_light_intensity), 0.0f, 10.0f))
    {
        m_brdfs.selected()->shader().set("light_intensity", m_light_intensity);
    }

    ImGui::SeparatorText(m_brdfs.selected()->name());

    m_brdfs.selected()->update_ui();

    ImGui::SeparatorText("Misc");


    const auto fps = ImGui::GetIO().Framerate;
    ImGui::LabelText("Framerate", "%.2f FPS", fps);

    ImGui::End();
}

void App::on_render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::on_mouse_scroll(double dx, double dy)
{
    m_camera->zoom(dy > 0.0);
    m_brdfs.selected()->shader().set("camera_radius", m_camera->radius());
}
