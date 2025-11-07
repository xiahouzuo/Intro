#include "itrpch.h"
#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/MouseButtonCodes.h"
#include "Intro/KeyCodes.h"
#include "Intro/Application.h" // 新增：用于窗口状态判断

namespace Intro {

    OrbitCamera::OrbitCamera(const Window& window)
        : Camera(window)
    {
        Distance = glm::length(Position - Target);
        // 新增：根据初始位置计算初始角度（参考FreeCamera的角度初始化）
        glm::vec3 direction = glm::normalize(Position - Target);
        m_Pitch = glm::degrees(glm::asin(direction.y));
        m_Yaw = glm::degrees(glm::atan(direction.z, direction.x));

        m_LastMouseX = Input::GetMouseX();
        m_LastMouseY = Input::GetMouseY();
    }

    glm::mat4 OrbitCamera::GetViewMat() const
    {
        return glm::lookAt(Position, Target, Up);
    }

    void OrbitCamera::OnUpdate(float deltaTime)
    {
        Application& app = Application::Get(); // 新增：获取应用实例

        // 鼠标旋转控制（参考FreeCamera的交互条件）
        bool mouseButtonDown = Input::IsMouseButtonPressed(ITR_MOUSE_BUTTON_1);
        bool allowMouseControl = mouseButtonDown && app.IsViewportHovered() && !app.IsUsingGizmo();

        if (allowMouseControl)
        {
            float currentX = Input::GetMouseX();
            float currentY = Input::GetMouseY();

            if (m_FirstMouse) {
                m_LastMouseX = currentX;
                m_LastMouseY = currentY;
                m_FirstMouse = false;
            }

            float xoffset = currentX - m_LastMouseX;
            float yoffset = m_LastMouseY - currentY; // Y轴反转（和FreeCamera一致）

            m_LastMouseX = currentX;
            m_LastMouseY = currentY;

            RotateCamera(xoffset, yoffset);
        }
        else
        {
            // 重置鼠标状态（参考FreeCamera）
            m_FirstMouse = true;
            m_LastMouseX = Input::GetMouseX();
            m_LastMouseY = Input::GetMouseY();
        }

        // 新增：键盘控制（参考FreeCamera的键盘交互逻辑）
        bool allowKeyboardControl = app.IsViewportFocused() && !app.IsUsingGizmo();
        if (allowKeyboardControl)
        {
            float zoomVelocity = m_ZoomSpeed * deltaTime;
            float targetVelocity = m_TargetMovementSpeed * deltaTime;

            // 左Shift加速（和FreeCamera一致）
            if (Input::IsKeyPressed(ITR_KEY_LEFT_SHIFT)) {
                zoomVelocity *= 2.0f;
                targetVelocity *= 2.0f;
            }

            // 缩放控制（W/S调整距离）
            if (Input::IsKeyPressed(ITR_KEY_W))
                SetDistance(Distance - zoomVelocity);
            if (Input::IsKeyPressed(ITR_KEY_S))
                SetDistance(Distance + zoomVelocity);

            // 目标移动控制（参考FreeCamera的WASDQE逻辑）
            glm::vec3 forward = glm::normalize(Target - Position);
            glm::vec3 right = glm::normalize(glm::cross(forward, Up));

            if (Input::IsKeyPressed(ITR_KEY_A))
                Target -= right * targetVelocity;
            if (Input::IsKeyPressed(ITR_KEY_D))
                Target += right * targetVelocity;
            if (Input::IsKeyPressed(ITR_KEY_Q))
                Target -= Up * targetVelocity;
            if (Input::IsKeyPressed(ITR_KEY_E))
                Target += Up * targetVelocity;
            if (Input::IsKeyPressed(ITR_KEY_R))
                Target += forward * targetVelocity;
            if (Input::IsKeyPressed(ITR_KEY_F))
                Target -= forward * targetVelocity;
        }
    }

    void OrbitCamera::RotateCamera(float xoffset, float yoffset)
    {
        xoffset *= m_MouseSensitivity;
        yoffset *= m_MouseSensitivity;

        // 更新角度（参考FreeCamera的角度计算）
        m_Yaw += xoffset;
        m_Pitch += yoffset;

        // 限制俯仰角（避免翻转，和FreeCamera一致）
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        // 根据角度计算相机位置（参考FreeCamera的向量更新逻辑）
        glm::vec3 direction;
        direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction.y = sin(glm::radians(m_Pitch));
        direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction = glm::normalize(direction);

        Position = Target + direction * Distance; // 围绕目标旋转
    }

} // namespace Intro