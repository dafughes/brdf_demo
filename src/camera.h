#pragma once

#include <glm/glm.hpp>

class OrbitalCamera
{
public:
    OrbitalCamera(float pitch, float yaw, float radius, float fov);

    glm::mat4 view_matrix() const;
    float fov() const;
    float radius() const;

    void zoom(bool in);
    void set_fov(float fov);
    void start_drag(float x, float y);
    void stop_drag();
    void update(float x, float y);

private:
    float m_zoom_sensitivity, m_drag_sensitivity;

    float m_pitch, m_yaw, m_fov, m_radius;
    bool m_is_dragged;
    float m_pitch_start, m_yaw_start;
    float m_x_start, m_y_start;
};