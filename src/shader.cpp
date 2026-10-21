#include "shader.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>


Shader::Shader(const std::string& src, GLenum type)
{
    m_id = glCreateShader(type);
    const auto src_c = src.c_str();
    glShaderSource(m_id, 1, &src_c, nullptr);
    glCompileShader(m_id);

    int success;
    char info[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(m_id, 512, NULL, info);
        std::cerr << "Shader compile error!\n" << info << "\n";
    }
}

Shader::~Shader()
{
    glDeleteShader(m_id);
}

unsigned int Shader::id() const
{
    return m_id;
}


ShaderProgram::ShaderProgram(std::initializer_list<Shader> shaders)
{
    m_id = glCreateProgram();
    for (const auto& shader : shaders)
    {
        glAttachShader(m_id, shader.id());
    }

    glLinkProgram(m_id);

    int success;
    char info[512];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_id, 512, NULL, info);
        std::cerr << "Shader link error!\n" << info << "\n";
    }
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_id);
}

void ShaderProgram::use()
{
    glUseProgram(m_id);
}

void ShaderProgram::set(const std::string& name, glm::mat4 mat)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::set(const std::string& name, glm::vec3 vec)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void ShaderProgram::set(const std::string& name, glm::vec2 vec)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(vec));
}


void ShaderProgram::set(const std::string& name, float value)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(loc, value);
}

void ShaderProgram::set(const std::string& name, int value)
{
    auto loc = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(loc, value);
}
