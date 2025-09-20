#pragma once


#include "Intro/Core.h"
#include <string>
#include <glad/glad.h>

namespace Intro {

	class ITR_API Texture
	{
	public:
		Texture(const std::string& filepath);
		~Texture();

		void Bind(unsigned int slot = 0) const;
		void UnBind() const;

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline unsigned int GetID() const { return m_TextureID; }
		inline const std::string& GetType() const { return m_Type; }
		inline void SetType(const std::string& type) { m_Type = type; }

	private:
		unsigned int m_TextureID;
		std::string m_FilePath;
		unsigned char* m_LocalBuffer;
		int m_Width, m_Height, m_BPP;
		std::string m_Type;
	};

}