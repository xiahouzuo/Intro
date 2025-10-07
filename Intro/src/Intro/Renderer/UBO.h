#pragma once
#include "Intro/Core.h"
#include "glad/glad.h"
#include "glm/glm.hpp"

namespace Intro {

	class ITR_API UBO
	{
	public:
		UBO() = default;
		UBO(GLenum target, GLsizeiptr size)
		{
			glGenBuffers(1, &m_BufferID);
			glBindBuffer(target, m_BufferID);
			glBufferData(target, size, nullptr, GL_DYNAMIC_DRAW);
		}

		~UBO() { glDeleteBuffers(1, &m_BufferID); }

		void BindBase(GLenum target, GLuint index) const
		{
			glBindBufferBase(target, index, m_BufferID);
		}

		void UpdateSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) const
		{
			glBindBuffer(target, m_BufferID);
			glBufferSubData(target, offset, size, data);
		}

		GLuint GetID() const { return m_BufferID; }

	private:
		GLuint m_BufferID = 0;
	};

}