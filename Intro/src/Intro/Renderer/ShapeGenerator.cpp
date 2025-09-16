// Intro/Intro/src/Intro/Renderer/ShapeGenerator.cpp
#include "itrpch.h"
#include "ShapeGenerator.h"
#include <glm/gtc/constants.hpp>

namespace Intro {

    std::pair<std::vector<Vertex>, std::vector<unsigned int>> ShapeGenerator::GenerateCube(float size)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float halfSize = size / 2.0f;

        // 立方体8个顶点
        vertices = {
            // 前面
            Vertex(glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(halfSize, -halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),

            // 后面
            Vertex(glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(halfSize, -halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),
        };

        // 立方体6个面，每个面2个三角形
        indices = {
            0, 1, 2, 2, 3, 0, // 前面
            4, 5, 6, 6, 7, 4, // 后面
            4, 0, 3, 3, 7, 4, // 左面
            1, 5, 6, 6, 2, 1, // 右面
            3, 2, 6, 6, 7, 3, // 顶面
            4, 5, 1, 1, 0, 4  // 底面
        };

        return { vertices, indices };
    }

    std::pair<std::vector<Vertex>, std::vector<unsigned int>> ShapeGenerator::GenerateSphere(float radius, int sectors, int stacks)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float sectorStep = 2.0f * glm::pi<float>() / sectors;
        float stackStep = glm::pi<float>() / stacks;
        float sectorAngle, stackAngle;

        // 生成顶点
        for (int i = 0; i <= stacks; ++i) {
            stackAngle = glm::pi<float>() / 2.0f - i * stackStep;
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                sectorAngle = j * sectorStep;
                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
                glm::vec2 texCoords = glm::vec2(
                    (float)j / sectors,
                    (float)i / stacks
                );

                vertices.emplace_back(glm::vec3(x, y, z), normal, texCoords);
            }
        }

        // 生成索引
        unsigned int k1, k2;
        for (int i = 0; i < stacks; ++i) {
            k1 = i * (sectors + 1);
            k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != stacks - 1) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        return { vertices, indices };
    }

    std::pair<std::vector<Vertex>, std::vector<unsigned int>> ShapeGenerator::GeneratePlane(float width, float height, int xSegments, int ySegments)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float xStep = width / xSegments;
        float yStep = height / ySegments;
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;

        // 生成顶点
        for (int y = 0; y <= ySegments; ++y) {
            for (int x = 0; x <= xSegments; ++x) {
                float xPos = -halfWidth + x * xStep;
                float zPos = -halfHeight + y * yStep;

                vertices.emplace_back(
                    glm::vec3(xPos, 0.0f, zPos),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec2((float)x / xSegments, (float)y / ySegments)
                );
            }
        }

        // 生成索引
        unsigned int vertexCountX = xSegments + 1;
        for (int y = 0; y < ySegments; ++y) {
            for (int x = 0; x < xSegments; ++x) {
                unsigned int topLeft = y * vertexCountX + x;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (y + 1) * vertexCountX + x;
                unsigned int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        return { vertices, indices };
    }

}