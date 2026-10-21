#include "camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

OrbitalCamera::OrbitalCamera(float pitch, float yaw, float radius, float fov) :
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

glm::mat4 OrbitalCamera::view_matrix() const
{
    return glm::eulerAngleYXZ(m_yaw, m_pitch, 0.0f);
}

float OrbitalCamera::fov() const
{
    return m_fov;
}

void OrbitalCamera::set_fov(float fov)
{
    m_fov = fov;
}

float OrbitalCamera::radius() const
{
    return m_radius;
}

void OrbitalCamera::set_radius(float r)
{
    m_radius = std::clamp(r, 0.1f, 50.0f);
}


bool OrbitalCamera::is_dragging() const
{
    return m_is_dragged;
}

void OrbitalCamera::start_drag(float x, float y)
{
    m_is_dragged = true;
    m_x_start = x;
    m_y_start = y;
    m_pitch_start = m_pitch;
    m_yaw_start = m_yaw;
}

void OrbitalCamera::stop_drag()
{
    m_is_dragged = false;
}

void OrbitalCamera::update(float x, float y)
{
    const auto sensitivity = 0.002f;
    auto deltax = x - m_x_start;
    auto deltay = y - m_y_start;
    m_yaw = m_yaw_start + deltax * sensitivity;
    m_pitch = m_pitch_start + deltay * sensitivity;

    m_pitch = std::clamp(m_pitch, glm::radians(-89.0f), glm::radians(89.0f));
}
