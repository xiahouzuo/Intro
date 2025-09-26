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

        glm::mat4 GetViewMat() const;
        void OnUpdate(float deltaTime);

        // ¶îÍâ½Ó¿Ú
        void SetTarget(const glm::vec3& t) { Target = t; }
        const glm::vec3& GetTarget() const { return Target; }
        void SetDistance(float d) { Distance = d; }
        float GetDistance() const { return Distance; }

        // tuning
        void SetMouseSensitivity(float s) { m_MouseSensitivity = s; }

    private:
        void RotateCamera(float xoffset, float yoffset);

    private:
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 Target = glm::vec3(0.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

        float Distance = 5.0f;
        float m_MouseSensitivity = 0.1f;

        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
    };

} // namespace Intro
