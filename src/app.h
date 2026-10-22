#pragma once

#include <memory>
#include <vector>
#include "app_base.h"
#include "camera.h"
#include "shader.h"
#include "select.h"
#include <glm/gtc/type_ptr.hpp>

class BRDF
{
public:
    BRDF(const char* name, std::initializer_list<Shader> shader_components) : m_name(name), m_shader(shader_components)
    {}


    virtual void update_ui() = 0;
    virtual void update_uniforms() = 0;

    const char* name() const
    {
        return m_name;
    }

    ShaderProgram& shader()
    {
        return m_shader;
    }

protected:
    const char* m_name;
    ShaderProgram m_shader;
};

inline std::ostream& operator<<(std::ostream& os, const std::unique_ptr<BRDF>& brdf)
{
    return os << brdf->name();
}

class Phong : public BRDF
{
public:
    Phong(
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        float shininess,
        std::initializer_list<Shader> shader_components) :
        BRDF("Phong", shader_components),
        m_ambient(ambient),
        m_diffuse(diffuse),
        m_specular(specular),
        m_shininess(shininess)
    {}

    virtual void update_ui() override
    {
        ImGui::ColorEdit3("Ambient", glm::value_ptr(m_ambient));
        ImGui::ColorEdit3("Diffuse", glm::value_ptr(m_diffuse));
        ImGui::ColorEdit3("Specular", glm::value_ptr(m_specular));
        ImGui::SliderFloat("Shininess", &m_shininess, 0.0f, 30.0f);
        update_uniforms();
    }

    virtual void update_uniforms() override
    {
        m_shader.set("ambient", m_ambient);
        m_shader.set("diffuse", m_diffuse);
        m_shader.set("specular", m_specular);
        m_shader.set("shininess", m_shininess);
    }


private:
    glm::vec3 m_ambient, m_diffuse, m_specular;
    float m_shininess;
};

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
    glm::vec3 m_light_pos{0.0f, 5.0f, 0.0f};
    glm::vec3 m_light_intensity{1.0f, 1.0f, 1.0f};

    std::unique_ptr<OrbitalCamera> m_camera{nullptr};

    Select<int> m_supersampling;
    Select<std::unique_ptr<BRDF>> m_brdfs;
};