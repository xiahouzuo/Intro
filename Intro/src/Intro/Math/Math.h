#pragma once
#include "Intro/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Intro{

    class ITR_API Math {
    public:
        // ����
        static constexpr float PI = 3.14159265358979323846f;
        static constexpr float DEG2RAD = PI / 180.0f;
        static constexpr float RAD2DEG = 180.0f / PI;
        static constexpr float EPSILON = 1e-6f;

        // �任�ֽ�
        static bool DecomposeTransform(const glm::mat4& transform,
            glm::vec3& translation,
            glm::quat& rotation,
            glm::vec3& scale);

        // ��Ԫ����ŷ����ת��
        static glm::vec3 GetEulerAngles(const glm::quat& quat);
        static glm::quat EulerToQuaternion(const glm::vec3& euler);

        // �Ƕ�ת��
        static float ToRadians(float degrees);
        static float ToDegrees(float radians);

        // ��ֵ����
        static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
        static glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t);

        // ��ѧ����
        static float Clamp(float value, float min, float max);
        static glm::vec3 Clamp(const glm::vec3& value, const glm::vec3& min, const glm::vec3& max);
        static float Lerp(float a, float b, float t);
        static float SmoothStep(float edge0, float edge1, float x);

        // ��������
        static float Distance(const glm::vec3& a, const glm::vec3& b);
        static float DistanceSquared(const glm::vec3& a, const glm::vec3& b);
        static glm::vec3 Normalize(const glm::vec3& v);
        static float Dot(const glm::vec3& a, const glm::vec3& b);
        static glm::vec3 Cross(const glm::vec3& a, const glm::vec3& b);
    };

}