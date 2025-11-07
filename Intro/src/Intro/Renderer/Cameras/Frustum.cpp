// Frustum.cpp
#include "itrpch.h"
#include "Frustum.h"

namespace Intro {

    void Frustum::UpdateFromMatrix(const glm::mat4& viewProjectionMatrix) {
        glm::mat4 matrix = glm::transpose(viewProjectionMatrix);

        // 提取视锥体平面
        m_Planes[Plane_Left] = matrix[3] + matrix[0];
        m_Planes[Plane_Right] = matrix[3] - matrix[0];
        m_Planes[Plane_Bottom] = matrix[3] + matrix[1];
        m_Planes[Plane_Top] = matrix[3] - matrix[1];
        m_Planes[Plane_Near] = matrix[3] + matrix[2];
        m_Planes[Plane_Far] = matrix[3] - matrix[2];

        // 归一化平面
        for (auto& plane : m_Planes) {
            NormalizePlane(plane);
        }

        // 计算视锥体角点
        glm::mat4 inverseMatrix = glm::inverse(viewProjectionMatrix);

        // 定义标准化设备坐标的角点
        std::array<glm::vec4, Corner_Count> ndcCorners = {
            // 近平面
            glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
            glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
            glm::vec4(1.0f,  1.0f, -1.0f, 1.0f),
            glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f),
            // 远平面
            glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
            glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
            glm::vec4(1.0f,  1.0f, 1.0f, 1.0f),
            glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f)
        };

        // 变换到世界空间
        for (size_t i = 0; i < Corner_Count; ++i) {
            glm::vec4 worldCorner = inverseMatrix * ndcCorners[i];
            m_Corners[i] = glm::vec3(worldCorner) / worldCorner.w;
        }
    }

    void Frustum::NormalizePlane(glm::vec4& plane) {
        float length = glm::length(glm::vec3(plane));
        plane /= length;
    }

    bool Frustum::ContainsPoint(const glm::vec3& point) const {
        for (const auto& plane : m_Planes) {
            float distance = glm::dot(glm::vec3(plane), point) + plane.w;
            if (distance < 0.0f) {
                return false;
            }
        }
        return true;
    }

    bool Frustum::ContainsSphere(const glm::vec3& center, float radius) const {
        for (const auto& plane : m_Planes) {
            float distance = glm::dot(glm::vec3(plane), center) + plane.w;
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }

} // namespace Intro