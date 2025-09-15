#include "itrpch.h"
#include "Shader.h"

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
    // 编译顶点着色器
    unsigned int vertexshader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSouceCode = VertexShaderSource.c_str();
    glShaderSource(vertexshader, 1, &vertexShaderSouceCode, NULL);
    glCompileShader(vertexshader);
    int success;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success); // 检查顶点着色器编译状态
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexshader, 512, NULL, infoLog);
        ITR_CORE_ERROR("顶点着色器编译失败: {0}", infoLog);
        return; // 编译失败直接返回，避免后续错误
    }

    // 编译片段着色器（修复错误1：检查片段着色器自身的编译状态）
    unsigned int fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentshaderSouceCode = FragmentShaderSource.c_str();
    glShaderSource(fragmentshader, 1, &fragmentshaderSouceCode, NULL);
    glCompileShader(fragmentshader);
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success); // 修正：用glGetShaderiv检查片段着色器
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentshader, 512, NULL, infoLog);
        ITR_CORE_ERROR("片段着色器编译失败: {0}", infoLog);
        glDeleteShader(vertexshader); // 清理已创建的顶点着色器
        return;
    }

    // 创建并链接程序
    m_ShaderID = glCreateProgram();
    glAttachShader(m_ShaderID, vertexshader);
    glAttachShader(m_ShaderID, fragmentshader);
    glLinkProgram(m_ShaderID);

    // 修复错误2：检查程序链接状态
    glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
        ITR_CORE_ERROR("着色器程序链接失败: {0}", infoLog);
        glDeleteShader(vertexshader);
        glDeleteShader(fragmentshader);
        glDeleteProgram(m_ShaderID); // 清理无效程序
        m_ShaderID = 0; // 标记为无效
        return;
    }

    // 链接成功后删除着色器（已无用）
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
}

int Intro::Shader::GetUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(m_ShaderID, name.c_str());
}
