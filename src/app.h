#pragma once

#include <memory>
#include "app_base.h"
#include "camera.h"
#include "shader.h"

class App : public AppBase
{
public:
    App(int width, int height, const char* title);

    ~App();

    void on_start() override;
    void on_update() override;
    void on_render() override;

    void on_resize(int w, int h) override;
    void on_mouse_move(double x, double y) override;
    void on_mouse_click(int button, double x, double y) override;
    void on_mouse_release(int button, double x, double y) override;
    void on_mouse_scroll(double dx, double dy) override;

private:
    unsigned int m_vbo{0}, m_vao{0};
    int m_supersampling{1};
    glm::vec3 m_light_pos{0.0f, 5.0f, 0.0f};
    glm::vec3 m_color{1.0f, 1.0f, 1.0f};
    std::unique_ptr<OrbitalCamera> m_camera{nullptr};
    std::unique_ptr<ShaderProgram> m_shader{nullptr};
};