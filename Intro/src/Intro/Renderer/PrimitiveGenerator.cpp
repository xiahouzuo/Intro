#include "itrpch.h"
#include "PrimitiveGenerator.h"
#include "glm/gtc/constants.hpp"
#include <unordered_map>
#include <glad/glad.h>

namespace Intro {
    static std::unordered_map<Vertex, unsigned int> s_VertexMap;

    std::shared_ptr<Mesh> PrimitiveGenerator::CreateCube() {
        s_VertexMap.clear();  // ��չ�ϣ��
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // ������ 8 �������λ�á����ߡ�UV�����涨�壩
        const std::vector<Vertex> cubeVertices = {
            // ǰ�棨Z=1��
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            // ���棨Z=-1��
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            // ���棨X=-1��
            {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            // ���棨X=1��
            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            // ���棨Y=1��
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            // ���棨Y=-1��
            {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        };

        // �������������������ÿ���� 2 �������Σ��� 12 ����
        const unsigned int cubeIndices[] = {
            0,1,2, 0,2,3,  // ǰ��
            4,5,6, 4,6,7,  // ����
            8,9,10, 8,10,11,  // ����
            12,13,14, 12,14,15,  // ����
            16,17,18, 16,18,19,  // ����
            20,21,22, 20,22,23   // ����
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

        // 1. ���ɼ��㶥�㣨�����������㣩
        Vertex topPole = { {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.0f} };  // �ϼ��㣨Y=1��
        Vertex bottomPole = { {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 1.0f} };  // �¼��㣨Y=-1��
        AddVertex(vertices, indices, topPole);
        AddVertex(vertices, indices, bottomPole);

        // 2. ���ɾ�γ�����񶥵�
        for (uint32_t lat = 1; lat < latitudeSegments; lat++) {
            float theta = static_cast<float>(lat) / latitudeSegments * glm::pi<float>();  // γ�Ƚǣ�0���У�
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
                float phi = static_cast<float>(lon) / longitudeSegments * 2.0f * glm::pi<float>();  // ���Ƚǣ�0��2�У�
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                // �������� �� �ѿ������꣨λ�ã�
                glm::vec3 pos = {
                    sinTheta * cosPhi,
                    cosTheta,
                    sinTheta * sinPhi
                };
                // ���� = λ�ã�������淨��ָ�������⣩
                glm::vec3 normal = glm::normalize(pos);
                // UV ���꣨���ȡ�U��γ�ȡ�V��
                glm::vec2 uv = {
                    static_cast<float>(lon) / longitudeSegments,
                    static_cast<float>(lat) / latitudeSegments
                };

                AddVertex(vertices, indices, { pos, normal, uv });
            }
        }

        // 3. �������������Ӽ���������
        // �ϼ������һ����������ӣ��������ȣ�
        for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
            uint32_t nextLon = (lon + 1) % longitudeSegments;
            // �ϼ��㣨���� 0���� ��ǰ���ȶ��� �� ��һ���ȶ���
            indices.push_back(0);
            indices.push_back(2 + lon);
            indices.push_back(2 + nextLon);
        }

        // �м���������ӣ��ı��Ρ����������Σ�
        for (uint32_t lat = 1; lat < latitudeSegments - 1; lat++) {
            for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
                uint32_t nextLon = (lon + 1) % longitudeSegments;
                // ��ǰ�������� = 2 + (lat-1)*longitudeSegments + lon
                uint32_t v1 = 2 + (lat - 1) * longitudeSegments + lon;
                uint32_t v2 = 2 + lat * longitudeSegments + lon;
                uint32_t v3 = 2 + lat * longitudeSegments + nextLon;
                uint32_t v4 = 2 + (lat - 1) * longitudeSegments + nextLon;

                // ����������
                indices.push_back(v1); indices.push_back(v2); indices.push_back(v3);
                indices.push_back(v1); indices.push_back(v3); indices.push_back(v4);
            }
        }

        // �¼��������һ����������ӣ��������ȣ�
        uint32_t lastRowStart = 2 + (latitudeSegments - 2) * longitudeSegments;
        for (uint32_t lon = 0; lon < longitudeSegments; lon++) {
            uint32_t nextLon = (lon + 1) % longitudeSegments;
            // �¼��㣨���� 1���� ��һ���ȶ��� �� ��ǰ���ȶ���
            indices.push_back(1);
            indices.push_back(lastRowStart + nextLon);
            indices.push_back(lastRowStart + lon);
        }

        // ���� Mesh���������Բ����� Cube һ�£�
        return std::make_shared<Mesh>(vertices, indices);
    }

    std::shared_ptr<Mesh> PrimitiveGenerator::CreatePlane(float size, uint32_t divisions) {
        s_VertexMap.clear();
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float halfSize = size / 2.0f;
        float step = size / divisions;  // ÿ��Ĵ�С

        // �������񶥵�
        for (uint32_t z = 0; z <= divisions; z++) {
            for (uint32_t x = 0; x <= divisions; x++) {
                // λ�ã�XYƽ�棬Z=0��
                glm::vec3 pos = {
                    -halfSize + x * step,
                    0.0f,
                    -halfSize + z * step
                };
                // ���ߣ�Y�����ϣ�
                glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
                // UV ���꣨0��1 ӳ�䣩
                glm::vec2 uv = {
                    static_cast<float>(x) / divisions,
                    static_cast<float>(z) / divisions
                };

                AddVertex(vertices, indices, { pos, normal, uv });
            }
        }

        // ����������ÿ���������������Σ�
        for (uint32_t z = 0; z < divisions; z++) {
            for (uint32_t x = 0; x < divisions; x++) {
                // ��ǰ���ӵ��ĸ���������
                uint32_t v1 = z * (divisions + 1) + x;
                uint32_t v2 = (z + 1) * (divisions + 1) + x;
                uint32_t v3 = (z + 1) * (divisions + 1) + (x + 1);
                uint32_t v4 = z * (divisions + 1) + (x + 1);

                // ����������
                indices.push_back(v1); indices.push_back(v2); indices.push_back(v3);
                indices.push_back(v1); indices.push_back(v3); indices.push_back(v4);
            }
        }

        // ���� Mesh
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