#include "itrpch.h"
#include "Shader.h"
#include "glad/glad.h"
#include "glm/gtc/packing.inl"

void Intro::Shader::Bind() const
{
    glUseProgram(m_ShaderID);
}

void Intro::Shader::UnBind() const
{
    glUseProgram(0);
}

void Intro::Shader::SetUniformMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Intro::Shader::CompileShader(const std::string& VertexShaderSource, const std::string& FragmentShaderSource)
{
    unsigned int vertexshader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSouceCode = VertexShaderSource.c_str();
    glShaderSource(vertexshader, 1, &vertexShaderSouceCode, NULL);
    glCompileShader(vertexshader);

    unsigned int fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentshaderSouceCode = FragmentShaderSource.c_str();
    glShaderSource(fragmentshader, 1, &fragmentshaderSouceCode, NULL);
    glCompileShader(fragmentshader);

    m_ShaderID = glCreateProgram();
    glAttachShader(m_ShaderID, vertexshader);
    glAttachShader(m_ShaderID, fragmentshader);
    glLinkProgram(m_ShaderID);

    glDeleteProgram(vertexshader);
    glDeleteProgram(fragmentshader);
}

int Intro::Shader::GetUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(m_ShaderID, name.c_str());
}
