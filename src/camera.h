#pragma once

#include <glm/glm.hpp>

class OrbitalCamera
{
public:
    OrbitalCamera(float pitch, float yaw, float radius, float fov);

    glm::mat4 view_matrix() const;
    float fov() const;
    void set_fov(float fov);
    float radius() const;
    void set_radius(float r);
    bool is_dragging() const;
    void start_drag(float x, float y);
    void stop_drag();
    void update(float x, float y);

private:
    float m_pitch, m_yaw, m_fov, m_radius;
    bool m_is_dragged;
    float m_pitch_start, m_yaw_start;
    float m_x_start, m_y_start;
};