#version 460 core

uniform vec2 screen_size;
uniform float fov;
uniform float camera_radius;
uniform mat4 view;
uniform vec3 light_pos;
uniform vec3 light_intensity;


out vec4 FragColor;

const float hit_threshold = 0.0001f;
const float max_distance = 1e6f;
const int max_iterations = 1000;
const float eps = 0.0001f;
uniform int supersampling;

const vec2 supersampling_offsets[] = vec2[](
    vec2(0), // 1 X
    vec2(-0.25, 0.25), // 4 X
    vec2(0.25, 0.25),
    vec2(0.25, -0.25),
    vec2(-0.25, -0.25),
    vec2(-0.375, -0.375), // 16 X
    vec2(-0.125, -0.375),
    vec2(0.125, -0.375),
    vec2(0.375, -0.375),
    vec2(-0.375, -0.125),
    vec2(-0.125, -0.125),
    vec2(0.125, -0.125),
    vec2(0.375, -0.125),
    vec2(-0.375, 0.125),
    vec2(-0.125, 0.125),
    vec2(0.125, 0.125),
    vec2(0.375, 0.125),
    vec2(-0.375, 0.375),
    vec2(-0.125, 0.375),
    vec2(0.125, 0.375),
    vec2(0.375, 0.375));

// `light` should not be not normalized since it contains light distance information
// Defined in an another file!
vec3 brdf(vec3 pos, vec3 view, vec3 light, vec3 normal, vec3 tangent, vec3 bitangent);

// from ChatGPT
float radialGrain(float r)
{
    return sin(r * 400.0) * 0.50 + sin(r * 830.0) * 0.25 + sin(r * 1730.0) * 0.12;
}

float radialGrainDerivative(float r)
{
    return cos(r * 400.0) * 200.0 + cos(r * 830.0) * 207.5 + cos(r * 1730.0) * 207.6;
}

vec3 radialBrushedNormal(vec3 p, vec3 N, vec3 center, vec3 axis, float frequency, float strength)
{
    axis = normalize(axis);

    // Position relative to brushing center.
    vec3 q = p - center;

    // Project onto radial plane.
    vec3 radial = q - axis * dot(q, axis);
    float r = length(radial);

    // Avoid singularity at center.
    if (r < 0.0001)
        return N;

    radial /= r;

    // Scale the radial coordinate.
    float scaledR = r * frequency;

    // Evaluate grain derivative.
    float slope = radialGrainDerivative(scaledR) * frequency * strength;

    // Apply perturbation.
    return normalize(N - radial * slope);
}


float smoothmin(float a, float b, float k)
{
    // k *= 1.0;
    float r = exp2(-a / k) + exp2(-b / k);
    return -k * log2(r);
}

float smoothplane(vec3 pos, float height)
{
    return sqrt(pos.y * pos.y + 0.0000000001) - height;
}

// SDF stuff from Inigo Quilez (https://iquilezles.org/articles/distfunctions/)
float plane(vec3 pos, float height)
{
    return abs(pos.y) - height;
}

float sphere(vec3 pos, float radius)
{
    return length(pos) - radius;
}

float cylinder(vec3 p, float r, float h)
{
    vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(r, h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float scene_sdf(vec3 pos)
{
    // Repetition
    vec3 spacing = vec3(0.21f, 0.0f, 0.21f);
    vec3 limit = vec3(20, 0, 20);
    vec3 q = pos - spacing * clamp(round(pos / spacing), -limit, limit);

    // const float s = sphere(q, 0.1f);
    const float s = cylinder(q, 0.1, 0.1);
    return min(plane(pos, 0.0f), s);
    // return smoothmin(smoothplane(pos, 0.0f), s, 0.01f);
}

// vec3 scene_normal_analytic

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

void tbn(vec3 normal, out vec3 tangent, out vec3 bitangent)
{
    vec3 up = abs(normal.y) < 0.999 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);

    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
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
        vec3 spacing = vec3(0.21f, 0.0f, 0.21f);
        vec3 limit = vec3(20, 0, 20);
        vec3 q = spacing * clamp(round(hit / spacing), -limit, limit);
        normal = radialBrushedNormal(q, normal, hit, vec3(0, 1, 0), 5.0, 0.00001);

        vec3 light_hit;
        vec3 light_dir = light_pos - hit;
        if (!march_ray(hit + eps * normal, normalize(light_dir), light_hit))
        {
            vec3 tangent;
            vec3 bitangent;
            tbn(normal, tangent, bitangent);

            const float constant = 1.0f;
            const float linear = 0.08f;
            const float quadratic = 0.03f;

            const float d = length(light_dir);
            const vec3 intensity = light_intensity / (constant + linear * d + quadratic * d * d);

            return brdf(hit, normalize(origin - hit), normalize(light_dir), normal, tangent, bitangent) * intensity *
                dot(normalize(light_dir), normal);
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


void main()
{
    const float camera_near = 1.0f / (2.0f * tan(fov / 2.0f));

    // Center at (0,0)
    const vec2 frag_coord = (gl_FragCoord.xy - screen_size / 2.0f);


    vec3 ray_origin = vec3(0, 0, camera_radius);
    ray_origin = (view * vec4(-ray_origin, 1.0f)).xyz;

    vec3 color = vec3(0);

    int start, end;
    if (supersampling == 1)
    {
        start = 0;
        end = 1;
    }
    else if (supersampling == 4)
    {
        start = 1;
        end = 5;
    }
    else
    {
        start = 5;
        end = 21;
    }

    for (int i = start; i < end; i++)
    {
        vec3 ray_direction = create_ray(frag_coord, supersampling_offsets[i], camera_near, screen_size.x);
        ray_direction = (view * vec4(ray_direction, 1.0f)).xyz;

        color += march(ray_origin, ray_direction);
    }

    FragColor = vec4(color / float(supersampling), 1.0f);
}