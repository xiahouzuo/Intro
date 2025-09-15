#pragma once
#include <glm/glm.hpp>

namespace Intro {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;

		Vertex(glm::vec3 pos = glm::vec3(1.0f),
			glm::vec3 norm = glm::vec3(1.0f),
			glm::vec2 tex = glm::vec2(1.0f))
			:Position(pos), Normal(norm), TexCoords(tex)
		{

		}

	};

}