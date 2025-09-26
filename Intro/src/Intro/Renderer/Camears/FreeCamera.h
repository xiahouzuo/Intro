#pragma once
#pragma once
#include "Camera.h"
#include "glm/glm.hpp"
#include "Intro/Window.h"

namespace Intro {

    class ITR_API FreeCamera : public Camera
    {
    public:
        FreeCamera() = default;
        FreeCamera(const Window& window);

        glm::mat4 GetViewMat() const;
        void OnUpdate(float deltaTime);

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
