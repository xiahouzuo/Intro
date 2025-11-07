// Frustum.h
#pragma once
#include "Intro/Core.h"
#include <glm/glm.hpp>
#include <array>

namespace Intro {

    class ITR_API Frustum {
    public:
        // 修改枚举定义，避免命名冲突
        enum PlaneEnum {
            Plane_Near = 0,
            Plane_Far,
            Plane_Left,
            Plane_Right,
            Plane_Top,
            Plane_Bottom,
            Plane_Count
        };

        enum CornerEnum {
            Corner_NearTopLeft = 0,
            Corner_NearTopRight,
            Corner_NearBottomLeft,
            Corner_NearBottomRight,
            Corner_FarTopLeft,
            Corner_FarTopRight,
            Corner_FarBottomLeft,
            Corner_FarBottomRight,
            Corner_Count
        };

        Frustum() = default;

        // 从视图投影矩阵更新视锥体
        void UpdateFromMatrix(const glm::mat4& viewProjectionMatrix);

        // 检查点是否在视锥体内
        bool ContainsPoint(const glm::vec3& point) const;

        // 检查球体是否在视锥体内
        bool ContainsSphere(const glm::vec3& center, float radius) const;

        // 获取视锥体的8个角点
        const std::array<glm::vec3, Corner_Count>& GetCorners() const { return m_Corners; }

        // 获取视锥体平面
        const std::array<glm::vec4, Plane_Count>& GetPlanes() const { return m_Planes; }

    private:
        void NormalizePlane(glm::vec4& plane);

    private:
        std::array<glm::vec4, Plane_Count> m_Planes;
        std::array<glm::vec3, Corner_Count> m_Corners;
    };

} // namespace Intro