#include "itrpch.h"
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Intro {

	Texture::Texture(const std::string& filepath)
		:m_FilePath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
	{

		if (!std::filesystem::exists(filepath)) {
			std::cout << "纹理文件不存在: " << filepath << std::endl;
			throw std::runtime_error("纹理文件不存在: " + filepath);
		}

		glGenTextures(1, &m_TextureID);
		
		stbi_set_flip_vertically_on_load(true);  // 关键修复：翻转纹理坐标
		unsigned char* imageData = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BPP, 0);
		if (imageData)
		{
			GLenum format;
			if (m_BPP == 1)
				format = GL_RED;
			else if (m_BPP == 3)
				format = GL_RGB;
			else if (m_BPP == 4)
				format = GL_RGBA;


			glBindTexture(GL_TEXTURE_2D, m_TextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, imageData);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			std::cout << "Failed to load texture at path:" << filepath << std::endl;
		}

		stbi_image_free(imageData);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_TextureID);
	}

	void Texture::Bind(unsigned int slot) const
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureID);
	}

	void Texture::UnBind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}


}