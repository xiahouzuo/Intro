#pragma once
#include "Intro/Core.h"
#include "Vertex.h"
#include "Texture.h"
#include "Shader.h"
#include "glad/glad.h"
//std
#include <memory>
#include <vector>

namespace Intro {

	class ITR_API Mesh
	{
	public:

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures)
			:Vertices(vertices), Indices(indices), m_Textures(textures)
		{
			SetupMesh();
		}

		~Mesh() {
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &IBO);
		}

		void Draw(Shader& shader) const;

		const std::vector<Vertex>& GetVertices() const { return Vertices; }
		const std::vector<unsigned int>& GetIndices() const { return Indices; }
		const std::vector<std::shared_ptr<Texture>>& GetTextures() const { return m_Textures; }
	private:
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		unsigned int VAO, VBO, IBO;
		std::vector<std::shared_ptr<Texture>> m_Textures;

		void SetupMesh();
	};

}