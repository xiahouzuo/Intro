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
        // 生成立方体
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GenerateCube(float size = 1.0f);

        // 生成球体
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GenerateSphere(float radius = 1.0f, int sectors = 36, int stacks = 18);

        // 生成平面
        static std::pair<std::vector<Vertex>, std::vector<unsigned int>> GeneratePlane(float width = 1.0f, float height = 1.0f, int xSegments = 1, int ySegments = 1);
    };

}