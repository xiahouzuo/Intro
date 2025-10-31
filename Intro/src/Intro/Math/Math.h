#pragma once
#include "Intro/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Intro{

    class ITR_API Math {
    public:
        // 常量
        static constexpr float PI = 3.14159265358979323846f;
        static constexpr float DEG2RAD = PI / 180.0f;
        static constexpr float RAD2DEG = 180.0f / PI;
        static constexpr float EPSILON = 1e-6f;

        // 变换分解
        static bool DecomposeTransform(const glm::mat4& transform,
            glm::vec3& translation,
            glm::quat& rotation,
            glm::vec3& scale);

        // 四元数和欧拉角转换
        static glm::vec3 GetEulerAngles(const glm::quat& quat);
        static glm::quat EulerToQuaternion(const glm::vec3& euler);

        // 角度转换
        static float ToRadians(float degrees);
        static float ToDegrees(float radians);

        // 插值函数
        static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
        static glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t);

        // 数学运算
        static float Clamp(float value, float min, float max);
        static glm::vec3 Clamp(const glm::vec3& value, const glm::vec3& min, const glm::vec3& max);
        static float Lerp(float a, float b, float t);
        static float SmoothStep(float edge0, float edge1, float x);

        // 向量运算
        static float Distance(const glm::vec3& a, const glm::vec3& b);
        static float DistanceSquared(const glm::vec3& a, const glm::vec3& b);
        static glm::vec3 Normalize(const glm::vec3& v);
        static float Dot(const glm::vec3& a, const glm::vec3& b);
        static glm::vec3 Cross(const glm::vec3& a, const glm::vec3& b);
    };

}