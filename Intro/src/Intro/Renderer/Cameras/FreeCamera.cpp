#include "itrpch.h"
#include "FreeCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/Application.h"
#include "Intro/ImGui/ImGuiLayer.h"
#include "Intro/MouseButtonCodes.h"
#include "Intro/KeyCodes.h"

namespace Intro {

    FreeCamera::FreeCamera(const Window& window)
        : Camera(window)
    {
        UpdateCameraVectors();
        m_LastMouseX = Input::GetMouseX();
        m_LastMouseY = Input::GetMouseY();
    }

    glm::mat4 FreeCamera::GetViewMat() const
    {
        return glm::lookAt(Position, Position + m_Front, m_WorldUp);
    }
    glm::vec3 FreeCamera::GetFront() const
    {
        return m_Front;
    }

    void FreeCamera::OnUpdate(float deltaTime)
    {
        Application& app = Application::Get();

        // --- 鼠标视角控制 ---
        bool mouseButtonDown = Input::IsMouseButtonPressed(ITR_MOUSE_BUTTON_1);
        bool allowMouseControl = mouseButtonDown && app.IsViewportHovered() && !app.IsUsingGizmo();

        if (allowMouseControl)
        {
            // 原有的鼠标控制逻辑
            float currentX = Input::GetMouseX();
            float currentY = Input::GetMouseY();

            if (m_FirstMouse) {
                m_LastMouseX = currentX;
                m_LastMouseY = currentY;
                m_FirstMouse = false;
            }

            float xoffset = currentX - m_LastMouseX;
            float yoffset = m_LastMouseY - currentY;

            m_LastMouseX = currentX;
            m_LastMouseY = currentY;

            xoffset *= m_MouseSensitivity;
            yoffset *= m_MouseSensitivity;

            m_Yaw += xoffset;
            m_Pitch += yoffset;

            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;

            UpdateCameraVectors();
        }
        else
        {
            m_FirstMouse = true;
            m_LastMouseX = Input::GetMouseX();
            m_LastMouseY = Input::GetMouseY();
        }

        // --- 键盘移动 ---
        bool allowKeyboardControl = app.IsViewportFocused() && !app.IsUsingGizmo();

        if (allowKeyboardControl)
        {
            float velocity = m_MovementSpeed * deltaTime;
            if (Input::IsKeyPressed(ITR_KEY_W)) Position += m_Front * velocity;
            if (Input::IsKeyPressed(ITR_KEY_S)) Position -= m_Front * velocity;
            if (Input::IsKeyPressed(ITR_KEY_D)) Position += m_Right * velocity;
            if (Input::IsKeyPressed(ITR_KEY_A)) Position -= m_Right * velocity;
            if (Input::IsKeyPressed(ITR_KEY_E)) Position += m_WorldUp * velocity;
            if (Input::IsKeyPressed(ITR_KEY_Q)) Position -= m_WorldUp * velocity;

            if (Input::IsKeyPressed(ITR_KEY_LEFT_SHIFT))
                Position += m_Front * (m_MovementSpeed * 2.0f * deltaTime);
        }
    }

    void FreeCamera::UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Front = glm::normalize(front);

        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
        // m_WorldUp 保持不变
    }

    void FreeCamera::SetPerspective(float radins, float aspectRatio, float nearClip, float farClip)
    {
        this->Fov = radins;
        this->AspectRatio = aspectRatio;
        this->FarClip = farClip;
        this->NearClip = nearClip;
    }

    glm::quat FreeCamera::GetRotation() const
    {
        // 获取视图矩阵，其旋转部分是相机旋转的逆（正交矩阵）
        glm::mat4 viewMatrix = GetViewMat();
        // 提取视图矩阵的旋转部分（3x3矩阵）
        glm::mat3 rotationMatrix = glm::mat3(viewMatrix);
        // 正交矩阵的逆等于转置，转置后得到相机实际的旋转矩阵
        rotationMatrix = glm::transpose(rotationMatrix);
        // 将旋转矩阵转换为四元数
        return glm::quat(rotationMatrix);
    }

} // namespace Intro
