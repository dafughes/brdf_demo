#version 460 core

uniform float n_u;
uniform float n_v;
uniform vec3 r_s;
uniform vec3 r_d;

#define PI 3.1415926538


vec3 brdf(vec3 pos, vec3 view, vec3 light, vec3 normal, vec3 tangent, vec3 bitangent)
{
    const vec3 h = normalize(view + light);

    // Specular part
    const float norm_s = sqrt((n_u + 1.0) * (n_v + 1.0)) / (8.0 * PI);
    const float exponent =
        (n_u * pow(dot(h, tangent), 2.0) + n_v * pow(dot(h, bitangent), 2.0)) / (1.0 - pow(dot(h, normal), 2.0));

    const vec3 f = r_s + (1.0 - r_s) * pow(1.0 - dot(h, light), 5);

    const vec3 specular =
        norm_s * pow(dot(normal, h), exponent) * f / (dot(h, light) * max(dot(normal, light), dot(normal, view)));

    // Diffuse part
    const vec3 norm_d = (28.0 * r_d) / (23.0 * PI);
    const vec3 diffuse =
        norm_d * (1.0 - r_s) * (1.0 - pow(1.0 - dot(normal, view), 5)) * (1.0 - pow(1.0 - dot(normal, light), 5));

    return specular + diffuse;
}
