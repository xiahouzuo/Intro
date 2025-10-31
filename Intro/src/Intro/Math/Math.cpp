#include "itrpch.h"
#include "Math.h"

namespace Intro{

    bool Math::DecomposeTransform(const glm::mat4& transform,
        glm::vec3& translation,
        glm::quat& rotation,
        glm::vec3& scale) {
        // 使用GLM的分解函数
        glm::vec3 skew;
        glm::vec4 perspective;
        return glm::decompose(transform, scale, rotation, translation, skew, perspective);
    }

    glm::vec3 Math::GetEulerAngles(const glm::quat& quat) {
        return glm::degrees(glm::eulerAngles(quat));
    }

    glm::quat Math::EulerToQuaternion(const glm::vec3& euler) {
        glm::vec3 radians = glm::radians(euler);
        return glm::quat(radians);
    }

    float Math::ToRadians(float degrees) {
        return degrees * DEG2RAD;
    }

    float Math::ToDegrees(float radians) {
        return radians * RAD2DEG;
    }

    glm::vec3 Math::Lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a * (1.0f - t) + b * t;
    }

    glm::quat Math::Slerp(const glm::quat& a, const glm::quat& b, float t) {
        return glm::slerp(a, b, t);
    }

    float Math::Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    glm::vec3 Math::Clamp(const glm::vec3& value, const glm::vec3& min, const glm::vec3& max) {
        return glm::vec3(
            Clamp(value.x, min.x, max.x),
            Clamp(value.y, min.y, max.y),
            Clamp(value.z, min.z, max.z)
        );
    }

    float Math::Lerp(float a, float b, float t) {
        return a * (1.0f - t) + b * t;
    }

    float Math::SmoothStep(float edge0, float edge1, float x) {
        x = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    float Math::Distance(const glm::vec3& a, const glm::vec3& b) {
        return glm::distance(a, b);
    }

    float Math::DistanceSquared(const glm::vec3& a, const glm::vec3& b) {
        return glm::distance2(a, b);
    }

    glm::vec3 Math::Normalize(const glm::vec3& v) {
        return glm::normalize(v);
    }

    float Math::Dot(const glm::vec3& a, const glm::vec3& b) {
        return glm::dot(a, b);
    }

    glm::vec3 Math::Cross(const glm::vec3& a, const glm::vec3& b) {
        return glm::cross(a, b);
    }

}