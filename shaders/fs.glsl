#version 460 core

uniform vec2 screen_size;
uniform float fov;
uniform float zoom;
uniform mat4 view;

out vec4 FragColor;

float sphere(vec3 pos, float radius)
{
    return length(pos) - radius;
}

// From Inigo Quilez
float box(vec3 pos, vec3 size)
{
    vec3 q = abs(pos) - size;

    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}


float scene_sdf(vec3 pos)
{
    const vec3 sphere_pos = vec3(0, 0, 0);
    const float sphere_radius = 0.1f;

    const vec3 box_pos = vec3(0.1, 0, 0);
    const vec3 box_size = vec3(0.1);
    const float s = sphere(sphere_pos - pos, sphere_radius);
    const float b = box(box_pos - pos, box_size);
    return min(s, b);
}

// Central difference
vec3 scene_normal(vec3 pos)
{
    const float eps = 0.0001f;
    const vec3 hx = vec3(eps, 0.0f, 0.0f);
    const vec3 hy = vec3(0.0f, eps, 0.0f);
    const vec3 hz = vec3(0.0f, 0.0f, eps);

    return normalize(vec3(
        scene_sdf(pos + hx) - scene_sdf(pos - hx),
        scene_sdf(pos + hy) - scene_sdf(pos - hy),
        scene_sdf(pos + hz) - scene_sdf(pos - hz)));
}

// direction is unit
vec3 march(vec3 origin, vec3 direction, float d0)
{
    const float hit_threshold = 0.0001f;
    const float max_distance = 1e3f;
    const int max_iterations = 100;

    float distance = d0;

    for (int i = 0; i < max_iterations; i++)
    {
        const vec3 pos = origin + direction * distance;

        float d = scene_sdf(pos);

        if (d < hit_threshold)
        {
            return scene_normal(pos + direction * d);
        }
        else if (distance > max_distance)
        {
            return vec3(0.0f);
        }

        distance += d;
    }

    return vec3(0.0);
}

void main()
{
    const vec2 pixel = gl_FragCoord.xy - screen_size / 2.0f;
    vec3 direction = normalize(vec3(pixel.x / screen_size.x, pixel.y / screen_size.x, -1.0));

    vec3 origin = vec3(0, 0, zoom);
    direction = (view * vec4(direction, 1.0f)).xyz;
    origin = (view * vec4(origin, 1.0f)).xyz;

    FragColor = vec4(march(origin, direction, 0.0f), 1.0f);
}