#pragma once
#include "Intro/Core.h"
#include <string>
#include "glm/glm.hpp"
#include <glad/glad.h>
#include "Intro/Log.h"

namespace Intro {

	class ITR_API Shader
	{
	public:
		Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
		{ 
			CompileShader(vertexShaderPath, fragmentShaderPath);
		}

		~Shader(){ glDeleteProgram(m_ShaderID); }

		void Bind() const;
		void UnBind() const;

		unsigned int GetShaderID() const { return m_ShaderID; }

		void SetUniformMat4(const std::string& name, const glm::mat4& value) const;
		void SetUniformInt(const std::string& name, int value) const;
		void SetUniformFloat(const std::string& name, float value) const;
		void SetUniformVec3(const std::string& name, const glm::vec3& value) const;

		void CheckShaderCompileStatus(GLuint shader, const std::string& type) {
			GLint success;
			GLchar infoLog[1024];
			if (type != "PROGRAM") {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					ITR_ERROR("SHADER_COMPILATION_ERROR of type {}: {}", type, infoLog);
				}
			}
			else {
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					ITR_ERROR("PROGRAM_LINKING_ERROR of type {}: {}", type, infoLog);
				}
			}
		}

	private:
		void CompileShader(const char* vertexShaderPath, const char* fragmentShaderPath);

		int GetUniformLocation(const std::string& name) const;
	private:
		unsigned int m_ShaderID;
	};

}