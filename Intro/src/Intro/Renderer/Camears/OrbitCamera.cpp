#include "itrpch.h"
#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/MouseButtonCodes.h"

namespace Intro {

    OrbitCamera::OrbitCamera(const Window& window)
        : Camera(window)
    {
        // 初始化距离与位置
        Distance = glm::length(Position - Target);
        m_LastMouseX = Input::GetMouseX();
        m_LastMouseY = Input::GetMouseY();
    }

    glm::mat4 OrbitCamera::GetViewMat() const
    {
        return glm::lookAt(Position, Target, Up);
    }

    void OrbitCamera::OnUpdate(float /*deltaTime*/)
    {
        if (Input::IsMouseButtonPressed(ITR_MOUSE_BUTTON_1))
        {
            float currentX = Input::GetMouseX();
            float currentY = Input::GetMouseY();

            if (m_FirstMouse) {
                m_LastMouseX = currentX;
                m_LastMouseY = currentY;
                m_FirstMouse = false;
            }

            float xoffset = currentX - m_LastMouseX;
            float yoffset = m_LastMouseY - currentY; // inverted Y

            m_LastMouseX = currentX;
            m_LastMouseY = currentY;

            RotateCamera(xoffset, yoffset);
        }
        else {
            // 避免下一次按下时跳变
            m_FirstMouse = true;
            m_LastMouseX = Input::GetMouseX();
            m_LastMouseY = Input::GetMouseY();
        }

        // 可扩展：滚轮缩放、平移 (中键拖拽) 等
    }

    void OrbitCamera::RotateCamera(float xoffset, float yoffset)
    {
        xoffset *= m_MouseSensitivity;
        yoffset *= m_MouseSensitivity;

        glm::vec3 forward = glm::normalize(Target - Position);

        // 绕世界Y轴旋转（水平）
        {
            glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset), Up);
            forward = glm::vec3(rotateY * glm::vec4(forward, 0.0f));
        }

        // 绕右轴旋转（俯仰）
        {
            glm::vec3 right = glm::normalize(glm::cross(forward, Up));
            glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset), right);
            forward = glm::vec3(rotateX * glm::vec4(forward, 0.0f));
        }

        // 更新位置，保持距离
        Position = Target - forward * Distance;
    }

} // namespace Intro
