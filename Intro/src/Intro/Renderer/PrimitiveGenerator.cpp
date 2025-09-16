#include "itrpch.h"
#include "PrimitiveGenerator.h"
#include "glm/gtc/constants.hpp"
#include <unordered_map>
#include <glad/glad.h>

namespace Intro {
    static std::unordered_map<Vertex, unsigned int> s_VertexMap;

    std::shared_ptr<Mesh> PrimitiveGenerator::CreateCube() {
        s_VertexMap.clear();  // 清空哈希表
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 立方体 8 个顶点的位置、法线、UV（按面定义）
        const std::vector<Vertex> cubeVertices = {
            // 前面（Z=1）
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            // 后面（Z=-1）
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            // 左面（X=-1）
            {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            // 右面（X=1）
            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            // 上面（Y=1）
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            // 下面（Y=-1）
            {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        };

        // 立方体的三角形索引（每个面 2 个三角形，共 12 个）
        const unsigned int cubeIndices[] = {
            0,1,2, 0,2,3,  // 前面
            4,5,6, 4,6,7,  // 后面
            8,9,10, 8,10,11,  // 左面
            12,13,14, 12,14,15,  // 右面
            16,17,18, 16,18,19,  // 上面
            20,21,22, 20,22,23   // 下面
        };

        
        for (unsigned int idx : cubeIndices) {
            AddVertex(vertices, indices, cubeVertices[idx]);
        }

        
        return std::make_shared<Mesh>(vertices, indices);
    }

    std::shared_ptr<Mesh> PrimitiveGenerator::CreateSphere(uint32_t latitudeSegments, uint32_t longitudeSegments) {
        s_VertexMap.clear();
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 1. 生成极点顶点（上下两个极点）
        Vertex topPole = { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.0f} };  // 上极点（Y=1）
        Vertex bottomPole = { {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 1.0f} };  // 下极点（Y=-1）
        AddVertex(vertices, indices, topPole);
        AddVertex(vertices, indices, bottomPole);

        // 2. 生成经纬度网格顶点
        for (uint32_t lat = 1; lat < latitudeSegments; lat++) {
            float theta = static_cast<float>(lat) / latitudeSegments * glm::pi<float>();  // 纬度角（0→π）
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
                float phi = static_cast<float>(lon) / longitudeSegments * 2.0f * glm::pi<float>();  // 经度角（0→2π）
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                // 球面坐标 → 笛卡尔坐标（位置）
                glm::vec3 pos = {
                    sinTheta * cosPhi,
                    cosTheta,
                    sinTheta * sinPhi
                };
                // 法线 = 位置（球体表面法线指向球心外）
                glm::vec3 normal = glm::normalize(pos);
                // UV 坐标（经度→U，纬度→V）
                glm::vec2 uv = {
                    static_cast<float>(lon) / longitudeSegments,
                    static_cast<float>(lat) / latitudeSegments
                };

                AddVertex(vertices, indices, { pos, normal, uv });
            }
        }

        // 3. 生成索引（连接极点与网格）
        // 上极点与第一行网格的连接（三角形扇）
        for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
            uint32_t nextLon = (lon + 1) % longitudeSegments;
            // 上极点（索引 0）→ 当前经度顶点 → 下一经度顶点
            indices.push_back(0);
            indices.push_back(2 + lon);
            indices.push_back(2 + nextLon);
        }

        // 中间网格的连接（四边形→两个三角形）
        for (uint32_t lat = 1; lat < latitudeSegments - 1; lat++) {
            for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
                uint32_t nextLon = (lon + 1) % longitudeSegments;
                // 当前顶点索引 = 2 + (lat-1)*longitudeSegments + lon
                uint32_t v1 = 2 + (lat - 1) * longitudeSegments + lon;
                uint32_t v2 = 2 + lat * longitudeSegments + lon;
                uint32_t v3 = 2 + lat * longitudeSegments + nextLon;
                uint32_t v4 = 2 + (lat - 1) * longitudeSegments + nextLon;

                // 两个三角形
                indices.push_back(v1); indices.push_back(v2); indices.push_back(v3);
                indices.push_back(v1); indices.push_back(v3); indices.push_back(v4);
            }
        }

        // 下极点与最后一行网格的连接（三角形扇）
        uint32_t lastRowStart = 2 + (latitudeSegments - 2) * longitudeSegments;
        for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
            uint32_t nextLon = (lon + 1) % longitudeSegments;
            // 下极点（索引 1）→ 下一经度顶点 → 当前经度顶点
            indices.push_back(1);
            indices.push_back(lastRowStart + nextLon);
            indices.push_back(lastRowStart + lon);
        }

        // 生成 Mesh（顶点属性布局与 Cube 一致）
        return std::make_shared<Mesh>(vertices, indices);
    }

    std::shared_ptr<Mesh> PrimitiveGenerator::CreatePlane(float size, uint32_t divisions) {
        s_VertexMap.clear();
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float halfSize = size / 2.0f;
        float step = size / divisions;  // 每格的大小

        // 生成网格顶点
        for (uint32_t z = 0; z <= divisions; z++) {
            for (uint32_t x = 0; x <= divisions; x++) {
                // 位置（XY平面，Z=0）
                glm::vec3 pos = {
                    -halfSize + x * step,
                    0.0f,
                    -halfSize + z * step
                };
                // 法线（Y轴向上）
                glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
                // UV 坐标（0→1 映射）
                glm::vec2 uv = {
                    static_cast<float>(x) / divisions,
                    static_cast<float>(z) / divisions
                };

                AddVertex(vertices, indices, { pos, normal, uv });
            }
        }

        // 生成索引（每个格子两个三角形）
        for (uint32_t z = 0; z < divisions; z++) {
            for (uint32_t x = 0; x < divisions; x++) {
                // 当前格子的四个顶点索引
                uint32_t v1 = z * (divisions + 1) + x;
                uint32_t v2 = (z + 1) * (divisions + 1) + x;
                uint32_t v3 = (z + 1) * (divisions + 1) + (x + 1);
                uint32_t v4 = z * (divisions + 1) + (x + 1);

                // 两个三角形
                indices.push_back(v1); indices.push_back(v2); indices.push_back(v3);
                indices.push_back(v1); indices.push_back(v3); indices.push_back(v4);
            }
        }

        // 生成 Mesh
        return std::make_shared<Mesh>(vertices, indices);
    }

    void PrimitiveGenerator::AddVertex(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, const Vertex& vertex)
    {
        if (s_VertexMap.find(vertex) != s_VertexMap.end())
        {
            indices.push_back(s_VertexMap[vertex]);
            return;
        }

        s_VertexMap[vertex] = static_cast<unsigned int>(vertices.size());
        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(vertices.size())-1);

    }
}