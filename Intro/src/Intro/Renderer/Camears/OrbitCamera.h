#pragma once
#include "Camera.h"
#include "glm/glm.hpp"
#include "Intro/Window.h"

namespace Intro {

    class ITR_API OrbitCamera : public Camera
    {
    public:
        OrbitCamera() = default;
        OrbitCamera(const Window& window);

        // ʵ�ֻ��ി�麯��
        glm::mat4 GetViewMat() const override;
        glm::vec3 GetPosition() const override { return Position; } // ������ʵ�ֻ���ӿ�
        void OnUpdate(float deltaTime) override;

        // Ŀ��;������
        void SetTarget(const glm::vec3& t) { Target = t; }
        const glm::vec3& GetTarget() const { return Target; }
        void SetDistance(float d) { Distance = glm::max(0.1f, d); } // ������С����
        float GetDistance() const { return Distance; }

        // �����Ⱥ��ٶ����ã��ο�FreeCamera�ĵ��Žӿڣ�
        void SetMouseSensitivity(float s) { m_MouseSensitivity = s; }
        void SetZoomSpeed(float s) { m_ZoomSpeed = s; }
        void SetTargetMovementSpeed(float s) { m_TargetMovementSpeed = s; }

    private:
        void RotateCamera(float xoffset, float yoffset);

    private:
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 Target = glm::vec3(0.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

        float Distance = 5.0f;
        float m_MouseSensitivity = 0.1f;
        float m_ZoomSpeed = 5.0f; // �����������ٶ�
        float m_TargetMovementSpeed = 5.0f; // ������Ŀ���ƶ��ٶ�

        // �������Ƕȿ��ƣ��ο�FreeCamera��Yaw/Pitch��
        float m_Yaw = 90.0f;
        float m_Pitch = 0.0f;

        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
    };

} // namespace Intro