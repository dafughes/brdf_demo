#pragma once

#include <string>
#include <utility>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& src, GLenum type);
    ~Shader();
    unsigned int id() const;


private:
    unsigned int m_id{0};
};

class ShaderProgram
{
public:
    ShaderProgram(std::initializer_list<Shader> shaders);

    ~ShaderProgram();
    void use();

    void set(const std::string& name, glm::mat4 mat);
    void set(const std::string& name, glm::vec3 vec);
    void set(const std::string& name, glm::vec2 vec);
    void set(const std::string& name, float value);
    void set(const std::string& name, int value);

private:
    unsigned int m_id{0};
};