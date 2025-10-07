// Intro/Intro/src/Intro/Renderer/ShapeGenerator.cpp
#include "itrpch.h"
#include "ShapeGenerator.h"
#include <glm/gtc/constants.hpp>

namespace Intro {

    std::pair<std::vector<Vertex>, std::vector<unsigned int>> ShapeGenerator::GenerateCube(float size)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float h = size / 2.0f;

        // 每个面独立 4 顶点（24 顶点），每顶点都有正确的法线
        vertices = {
            // front (+Z)
            Vertex(glm::vec3(-h, -h,  h), glm::vec3(0,0,1), glm::vec2(0,0)),
            Vertex(glm::vec3(h, -h,  h), glm::vec3(0,0,1), glm::vec2(1,0)),
            Vertex(glm::vec3(h,  h,  h), glm::vec3(0,0,1), glm::vec2(1,1)),
            Vertex(glm::vec3(-h,  h,  h), glm::vec3(0,0,1), glm::vec2(0,1)),

            // back (-Z)
            Vertex(glm::vec3(h, -h, -h), glm::vec3(0,0,-1), glm::vec2(0,0)),
            Vertex(glm::vec3(-h, -h, -h), glm::vec3(0,0,-1), glm::vec2(1,0)),
            Vertex(glm::vec3(-h,  h, -h), glm::vec3(0,0,-1), glm::vec2(1,1)),
            Vertex(glm::vec3(h,  h, -h), glm::vec3(0,0,-1), glm::vec2(0,1)),

            // left (-X)
            Vertex(glm::vec3(-h, -h, -h), glm::vec3(-1,0,0), glm::vec2(0,0)),
            Vertex(glm::vec3(-h, -h,  h), glm::vec3(-1,0,0), glm::vec2(1,0)),
            Vertex(glm::vec3(-h,  h,  h), glm::vec3(-1,0,0), glm::vec2(1,1)),
            Vertex(glm::vec3(-h,  h, -h), glm::vec3(-1,0,0), glm::vec2(0,1)),

            // right (+X)
            Vertex(glm::vec3(h, -h,  h), glm::vec3(1,0,0), glm::vec2(0,0)),
            Vertex(glm::vec3(h, -h, -h), glm::vec3(1,0,0), glm::vec2(1,0)),
            Vertex(glm::vec3(h,  h, -h), glm::vec3(1,0,0), glm::vec2(1,1)),
            Vertex(glm::vec3(h,  h,  h), glm::vec3(1,0,0), glm::vec2(0,1)),

            // top (+Y)
            Vertex(glm::vec3(-h,  h,  h), glm::vec3(0,1,0), glm::vec2(0,0)),
            Vertex(glm::vec3(h,  h,  h), glm::vec3(0,1,0), glm::vec2(1,0)),
            Vertex(glm::vec3(h,  h, -h), glm::vec3(0,1,0), glm::vec2(1,1)),
            Vertex(glm::vec3(-h,  h, -h), glm::vec3(0,1,0), glm::vec2(0,1)),

            // bottom (-Y)
            Vertex(glm::vec3(-h, -h, -h), glm::vec3(0,-1,0), glm::vec2(0,0)),
            Vertex(glm::vec3(h, -h, -h), glm::vec3(0,-1,0), glm::vec2(1,0)),
            Vertex(glm::vec3(h, -h,  h), glm::vec3(0,-1,0), glm::vec2(1,1)),
            Vertex(glm::vec3(-h, -h,  h), glm::vec3(0,-1,0), glm::vec2(0,1)),
        };

        // 每面 2 三角形，保证 CCW（面朝外）
        indices = {
            0,1,2, 2,3,0,       // front
            4,5,6, 6,7,4,       // back
            8,9,10, 10,11,8,    // left
            12,13,14, 14,15,12, // right
            16,17,18, 18,19,16, // top
            20,21,22, 22,23,20  // bottom
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

                // 从上方 (normal = +Y) 看为 CCW
                indices.push_back(topLeft);
                indices.push_back(topRight);
                indices.push_back(bottomRight);

                indices.push_back(topLeft);
                indices.push_back(bottomRight);
                indices.push_back(bottomLeft);
            }
        }

        return { vertices, indices };
    }

}