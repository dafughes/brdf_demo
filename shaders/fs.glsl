#version 460 core

uniform vec2 screen_size;
uniform float fov;
uniform float camera_radius;
uniform mat4 view;

out vec4 FragColor;

const vec3 light_pos = vec3(5.0f, 3.0f, 0.0f);
const float hit_threshold = 0.0001f;
const float max_distance = 1e6f;
const int max_iterations = 1000;
const float eps = 0.0001f;


// SDF stuff from Inigo Quilez (https://iquilezles.org/articles/distfunctions/)
float plane(vec3 pos, float height)
{
    return abs(pos.y) - height;
}

float sphere(vec3 pos, float radius)
{
    return length(pos) - radius;
}

float scene_sdf(vec3 pos)
{
    // Repetition
    vec3 spacing = vec3(0.3f, 0.0f, 0.3f);
    vec3 limit = vec3(100, 0, 100);
    vec3 q = pos - spacing * clamp(round(pos / spacing), -limit, limit);

    const float s = sphere(q, 0.1f);
    return min(plane(pos, 0.0f), s);
}

// Central difference
vec3 scene_normal(vec3 pos)
{
    const vec3 hx = vec3(eps, 0.0f, 0.0f);
    const vec3 hy = vec3(0.0f, eps, 0.0f);
    const vec3 hz = vec3(0.0f, 0.0f, eps);

    return normalize(vec3(
        scene_sdf(pos + hx) - scene_sdf(pos - hx),
        scene_sdf(pos + hy) - scene_sdf(pos - hy),
        scene_sdf(pos + hz) - scene_sdf(pos - hz)));
}


bool march_ray(vec3 origin, vec3 direction, out vec3 hit)
{
    float distance = 0.0f;

    for (int i = 0; i < max_iterations; i++)
    {
        const float d = scene_sdf(origin + direction * distance);

        if (d < hit_threshold)
        {
            hit = origin + direction * (distance + d - eps);
            return true;
        }
        else if (distance > max_distance)
        {
            break;
        }

        distance += d;
    }

    return false;
}

vec3 march(vec3 origin, vec3 direction)
{
    vec3 hit;
    if (march_ray(origin, direction, hit))
    {
        vec3 normal = scene_normal(hit);

        vec3 light_hit;
        vec3 light_dir = normalize(light_pos - hit);

        if (!march_ray(hit + eps * normal, light_dir, light_hit))
        {
            float diff = max(0.0f, dot(light_dir, normal));
            return vec3(diff);
        }
    }
    return vec3(0);
}

// frag_coord:                      pixel coord in [(-w/2,-h/2), (w/2, h/2)]
// offset_from_pixel_center:        [-0.5, 0.5]
// camera_near:                     distance of camera origin to "screen", based on fov
vec3 create_ray(vec2 frag_coord, vec2 offset_from_pixel_center, float camera_near, float screen_width)
{
    const vec2 pixel = (frag_coord + offset_from_pixel_center) / screen_width;
    return normalize(vec3(pixel.xy, -camera_near));
}

vec2 offsets[] = vec2[](vec2(-0.25, 0.25), vec2(0.25, 0.25), vec2(0.25, -0.25), vec2(-0.25, -0.25));

void main()
{
    const float camera_near = 1.0f / (2.0f * tan(fov / 2.0f));

    // Center at (0,0)
    const vec2 frag_coord = (gl_FragCoord.xy - screen_size / 2.0f);

    vec3 ray_origin = vec3(0, 0, camera_radius);
    ray_origin = (view * vec4(-ray_origin, 1.0f)).xyz;

    vec3 color = vec3(0);

    for (int i = 0; i < 4; i++)
    {
        vec3 ray_direction = create_ray(frag_coord, offsets[i], camera_near, screen_size.x);
        ray_direction = (view * vec4(ray_direction, 1.0f)).xyz;

        color += march(ray_origin, ray_direction);
    }


    FragColor = vec4(color / 4.0f, 1.0f);
}