#pragma once
#include "Mesh.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include "Intro/Core.h"

namespace Intro{

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;

        // 重载 == 运算符（供 Mesh 去重顶点用，可选）
        bool operator==(const Vertex& other) const {
            return Position == other.Position && Normal == other.Normal && TexCoords == other.TexCoords;
        }
    };

	class ITR_API PrimitiveGenerator
	{
	public:
        static std::shared_ptr<Mesh> CreateCube();

        static std::shared_ptr<Mesh> CreateSphere(uint32_t latitudeSegments = 32, uint32_t longitudeSegments = 32);

        static std::shared_ptr<Mesh> CreatePlane(float size = 10.0f, uint32_t divisions = 1);

	private:
        static void AddVertex(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, const Vertex& vertex);
	};


}