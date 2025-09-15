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
    // ���붥����ɫ��
    unsigned int vertexshader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSouceCode = VertexShaderSource.c_str();
    glShaderSource(vertexshader, 1, &vertexShaderSouceCode, NULL);
    glCompileShader(vertexshader);
    int success;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success); // ��鶥����ɫ������״̬
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexshader, 512, NULL, infoLog);
        ITR_CORE_ERROR("������ɫ������ʧ��: {0}", infoLog);
        return; // ����ʧ��ֱ�ӷ��أ������������
    }

    // ����Ƭ����ɫ�����޸�����1�����Ƭ����ɫ������ı���״̬��
    unsigned int fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentshaderSouceCode = FragmentShaderSource.c_str();
    glShaderSource(fragmentshader, 1, &fragmentshaderSouceCode, NULL);
    glCompileShader(fragmentshader);
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success); // ��������glGetShaderiv���Ƭ����ɫ��
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentshader, 512, NULL, infoLog);
        ITR_CORE_ERROR("Ƭ����ɫ������ʧ��: {0}", infoLog);
        glDeleteShader(vertexshader); // �����Ѵ����Ķ�����ɫ��
        return;
    }

    // ���������ӳ���
    m_ShaderID = glCreateProgram();
    glAttachShader(m_ShaderID, vertexshader);
    glAttachShader(m_ShaderID, fragmentshader);
    glLinkProgram(m_ShaderID);

    // �޸�����2������������״̬
    glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
        ITR_CORE_ERROR("��ɫ����������ʧ��: {0}", infoLog);
        glDeleteShader(vertexshader);
        glDeleteShader(fragmentshader);
        glDeleteProgram(m_ShaderID); // ������Ч����
        m_ShaderID = 0; // ���Ϊ��Ч
        return;
    }

    // ���ӳɹ���ɾ����ɫ���������ã�
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
}

int Intro::Shader::GetUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(m_ShaderID, name.c_str());
}
