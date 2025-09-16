// Intro/Intro/src/Intro/Renderer/ShapeGenerator.h
#pragma once
#include "Intro/Renderer/Vertex.h"
#include "Intro/Core.h"
#include <vector>
#include <glm/glm.hpp>

namespace Intro {

    class ITR_API ShapeGenerator
    {
    public:
        // ����������
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GenerateCube(float size = 1.0f);

        // ��������
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GenerateSphere(float radius = 1.0f, int sectors = 36, int stacks = 18);

        // ����ƽ��
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GeneratePlane(float width = 1.0f, float height = 1.0f, int xSegments = 1, int ySegments = 1);
    };

}