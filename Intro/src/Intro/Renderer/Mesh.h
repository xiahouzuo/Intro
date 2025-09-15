#pragma once
#include "Intro/Core.h"
#include "Vertex.h"

//std
#include <vector>

namespace Intro {

	class ITR_API Mesh
	{
	public:
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		unsigned int VAO, VBO, IBO;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
			:Vertices(vertices), Indices(indices)
		{
			SetupMesh();
		}

		void Draw() const;

	private:

		void SetupMesh();
	};

}