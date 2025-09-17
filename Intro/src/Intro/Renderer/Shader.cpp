#include "itrpch.h"
#include "Shader.h"
#include <fstream>
#include <iostream>
#include "glm/gtc/packing.inl"

namespace Intro {

	void Shader::Bind() const
	{
		glUseProgram(m_ShaderID);
	}

	void Shader::UnBind() const
	{
		glUseProgram(0);
	}

	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& value) const
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::CompileShader(const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		std::string VertexCode, FragmentCode;
		std::ifstream VertexShaderFile, FragmentShaderFile;

		VertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		FragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			VertexShaderFile.open(vertexShaderPath);
			FragmentShaderFile.open(fragmentShaderPath);
			std::stringstream VertexShaderStream, FragmentShaderStream;

			VertexShaderStream << VertexShaderFile.rdbuf();
			FragmentShaderStream << FragmentShaderFile.rdbuf();

			VertexShaderFile.close();
			FragmentShaderFile.close();

			VertexCode = VertexShaderStream.str();
			FragmentCode = FragmentShaderStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
			// ��Ӹ���ϸ�Ĵ�����Ϣ
			std::cout << "Vertex shader path: " << vertexShaderPath << std::endl;
			std::cout << "Fragment shader path: " << fragmentShaderPath << std::endl;
			return; // ��Ҫ���ļ���ȡʧ��ʱֱ�ӷ��أ������������մ���
		}

		// ����ļ������Ƿ�Ϊ��
		if (VertexCode.empty() || FragmentCode.empty()) {
			std::cout << "ERROR::SHADER::FILE_IS_EMPTY" << std::endl;
			return;
		}

		const char* VertexShaderCode = VertexCode.c_str();
		const char* FragmentShaderCode = FragmentCode.c_str();


		unsigned int vertexID, fragmentID;
		int success;
		char infoLog[512];

		//������ɫ��
		vertexID = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexID, 1, &VertexShaderCode, NULL);
		glCompileShader(vertexID);
		//����������
		glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		//Ƭ����ɫ��
		fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentID, 1, &FragmentShaderCode, NULL);
		glCompileShader(fragmentID);
		//����������
		glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		m_ShaderID = glCreateProgram();
		glAttachShader(m_ShaderID, vertexID);
		glAttachShader(m_ShaderID, fragmentID);
		glLinkProgram(m_ShaderID);
		glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_ShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertexID);
		glDeleteShader(fragmentID);
	}

	int Shader::GetUniformLocation(const std::string& name) const
	{
		return glGetUniformLocation(m_ShaderID, name.c_str());
	}
}