#pragma once
#include "Intro/Core.h"
#include "Vertex.h"

//std
#include <vector>

namespace Intro {

	class ITR_API Mesh
	{
	public:

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
			:Vertices(vertices), Indices(indices)
		{
			SetupMesh();
		}

		void Draw() const;

		const std::vector<Vertex>& GetVertices() const { return Vertices; }
		const std::vector<unsigned int>& GetIndices() const { return Indices; }
	private:
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		unsigned int VAO, VBO, IBO;

		void SetupMesh();
	};

}