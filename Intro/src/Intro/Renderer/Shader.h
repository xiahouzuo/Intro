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
		void Bind() const;
		void UnBind() const;

		unsigned int GetShaderID() const { return m_ShaderID; }

		void SetUniformMat4(const std::string& name, const glm::mat4& value) const;
	private:
		void CompileShader(const char* vertexShaderPath, const char* fragmentShaderPath);

		int GetUniformLocation(const std::string& name) const;
	private:
		unsigned int m_ShaderID;
	};

}