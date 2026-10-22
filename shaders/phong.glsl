#version 460 core

uniform float shininess;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

// light is not normalized
vec3 brdf(vec3 view, vec3 light, vec3 normal)
{
    const float constant = 1.0f;
    const float linear = 0.08f;
    const float quadratic = 0.03f;

    const float d = length(light);
    const float intensity = 1.0f / (constant + linear * d + quadratic * d * d);
    light = normalize(light);

    return intensity *
        (ambient + max(0.0f, dot(light, normal)) * diffuse +
         pow(max(0.0f, dot(reflect(-light, normal), view)), shininess) * specular);
}