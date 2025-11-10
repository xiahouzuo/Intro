#pragma once
#pragma once
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Intro/Window.h"

namespace Intro {

    class ITR_API FreeCamera : public Camera
    {
    public:
        FreeCamera() = default;
        FreeCamera(const Window& window);

        glm::mat4 GetViewMat() const;
        glm::vec3 GetPosition() const { return Position; }
        glm::quat GetRotation() const;
        glm::vec3 GetFront() const;
        void OnUpdate(float deltaTime);

        void SetPosition(glm::vec3 position)
        {
            Position = position;
            UpdateCameraVectors();
        }
        void SetRotation(glm::quat rotation)
        {
            glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);

            // 从 forward 向量反推 yaw / pitch
            m_Yaw = glm::degrees(atan2(forward.z, forward.x)) - 90.0f;
            m_Pitch = glm::degrees(asin(forward.y));

            // 限制 pitch 避免 gimbal lock
            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;

            // 从 forward 向量反推 yaw / pitch
            m_Yaw = glm::degrees(atan2(forward.z, forward.x)) - 90.0f;
            m_Pitch = glm::degrees(asin(forward.y));

            // 限制 pitch 避免 gimbal lock
            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;
            UpdateCameraVectors();
        }
        void SetPerspective(float radins, float aspectRatio, float nearClip, float farClip);
        // tuning
        void SetMouseSensitivity(float s) { m_MouseSensitivity = s; }
        void SetMovementSpeed(float s) { m_MovementSpeed = s; }

    private:
        void UpdateCameraVectors();

    private:
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);

        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;

        float m_MouseSensitivity = 0.1f;
        float m_MovementSpeed = 5.0f;

        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
    };

} // namespace Intro
