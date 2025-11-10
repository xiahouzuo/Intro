// Frustum.cpp
#include "itrpch.h"
#include "Frustum.h"
#include "Intro/Log.h"

namespace Intro {

    // Frustum.cpp - 修改 UpdateFromMatrix 函数中的角点部分
    void Frustum::UpdateFromMatrix(const glm::mat4& viewProjectionMatrix) {
        glm::mat4 matrix = glm::transpose(viewProjectionMatrix);

        // 提取平面（保持不变）
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

        // 修正：使用正确的NDC坐标（OpenGL标准：z范围[-1,1]）
        std::array<glm::vec4, Corner_Count> ndcCorners = {
            // 近平面 (z = -1)
            glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),  // 左下近
            glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),  // 右下近  
            glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f),  // 左上近
            glm::vec4(1.0f,  1.0f, -1.0f, 1.0f),  // 右上近
            // 远平面 (z = 1)
            glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),   // 左下远
            glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),   // 右下远
            glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f),   // 左上远
            glm::vec4(1.0f,  1.0f, 1.0f, 1.0f)    // 右上远
        };

        // 变换到世界空间
        glm::mat4 inverseMatrix = glm::inverse(viewProjectionMatrix);

        for (size_t i = 0; i < Corner_Count; ++i) {
            glm::vec4 worldCorner = inverseMatrix * ndcCorners[i];

            // 确保正确的透视除法
            if (std::abs(worldCorner.w) > 1e-6f) {
                m_Corners[i] = glm::vec3(worldCorner) / worldCorner.w;
            }
            else {
                m_Corners[i] = glm::vec3(worldCorner);
                ITR_WARN("Zero w component in frustum corner calculation");
            }
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