#version 460 core

uniform float shininess;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

vec3 brdf(vec3 pos, vec3 view, vec3 light, vec3 normal, vec3 tangent, vec3 bitangent)
{
    return (
        ambient + max(0.0f, dot(light, normal)) * diffuse +
        pow(max(0.0f, dot(reflect(-light, normal), view)), shininess) * specular);
}