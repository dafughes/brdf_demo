#include "app.h"
#include "util.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstddef>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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
    Shader ash(read_file("shaders/ashikhmin_shirley.glsl"), GL_FRAGMENT_SHADER);

    m_supersampling = Select<int>("Supersampling", 1, 4, 16);

    auto phong_components = {vs, fs, phong};
    auto ash_components = {vs, fs, ash};
    m_brdfs = Select<std::unique_ptr<BRDF>>(
        "BRDF",
        std::make_unique<Phong>(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), 1.0f, phong_components),
        std::make_unique<AshikhminShirley>(glm::vec3(1.0f), glm::vec3(1.0f), 1.0f, 1.0f, ash_components));


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
        m_brdfs.selected()->shader().use();
        m_brdfs.selected()->shader().set("fov", m_camera->fov());
        m_brdfs.selected()->shader().set("view", m_camera->view_matrix());
        m_brdfs.selected()->shader().set("camera_radius", m_camera->radius());
        m_brdfs.selected()->shader().set("light_pos", m_light_pos);
        m_brdfs.selected()->shader().set("light_intensity", m_light_intensity);
        m_brdfs.selected()->shader().set("supersampling", m_supersampling.selected());
        m_brdfs.selected()->update_uniforms();
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

    static float scale = 1.0;
    if (ImGui::SliderFloat("Picture scale", &scale, 1.0, 5.0))
    {}

    if (ImGui::Button("Take picture"))
    {
        unsigned int fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);


        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);

        const auto old_w = w;
        const auto old_h = h;

        w *= scale;
        h *= scale;
        m_brdfs.selected()->shader().set("screen_size", glm::vec2((float)w, (float)h));
        glViewport(0, 0, w, h);

        std::cout << "Taking a (" << w << ", " << h << ")picture...\n";

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not complete\n";
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Extract texture data
        std::vector<unsigned char> data(static_cast<std::size_t>(w) * h * 3);
        GLint old_pack_alignment;
        glGetIntegerv(GL_PACK_ALIGNMENT, &old_pack_alignment);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glPixelStorei(GL_PACK_ALIGNMENT, old_pack_alignment);

        // TODO: There is a bug when taking a fullscreen picture

        // Save to file
        stbi_flip_vertically_on_write(1);
        // stbi_write_bmp("out.bmp", w, h, 3, data.data());
        stbi_write_png("out.png", w, h, 3, data.data(), w * 3);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);

        m_brdfs.selected()->shader().set("screen_size", glm::vec2((float)old_w, (float)old_h));
        glViewport(0, 0, old_w, old_h);
    }

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
