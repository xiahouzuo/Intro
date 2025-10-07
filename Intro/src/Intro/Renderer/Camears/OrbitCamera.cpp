#include "itrpch.h"
#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/MouseButtonCodes.h"
#include "Intro/KeyCodes.h"
#include "Intro/Application.h" // ���������ڴ���״̬�ж�

namespace Intro {

    OrbitCamera::OrbitCamera(const Window& window)
        : Camera(window)
    {
        Distance = glm::length(Position - Target);
        // ���������ݳ�ʼλ�ü����ʼ�Ƕȣ��ο�FreeCamera�ĽǶȳ�ʼ����
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
        Application& app = Application::Get(); // ��������ȡӦ��ʵ��

        // �����ת���ƣ��ο�FreeCamera�Ľ���������
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
            float yoffset = m_LastMouseY - currentY; // Y�ᷴת����FreeCameraһ�£�

            m_LastMouseX = currentX;
            m_LastMouseY = currentY;

            RotateCamera(xoffset, yoffset);
        }
        else
        {
            // �������״̬���ο�FreeCamera��
            m_FirstMouse = true;
            m_LastMouseX = Input::GetMouseX();
            m_LastMouseY = Input::GetMouseY();
        }

        // ���������̿��ƣ��ο�FreeCamera�ļ��̽����߼���
        bool allowKeyboardControl = app.IsViewportFocused() && !app.IsUsingGizmo();
        if (allowKeyboardControl)
        {
            float zoomVelocity = m_ZoomSpeed * deltaTime;
            float targetVelocity = m_TargetMovementSpeed * deltaTime;

            // ��Shift���٣���FreeCameraһ�£�
            if (Input::IsKeyPressed(ITR_KEY_LEFT_SHIFT)) {
                zoomVelocity *= 2.0f;
                targetVelocity *= 2.0f;
            }

            // ���ſ��ƣ�W/S�������룩
            if (Input::IsKeyPressed(ITR_KEY_W))
                SetDistance(Distance - zoomVelocity);
            if (Input::IsKeyPressed(ITR_KEY_S))
                SetDistance(Distance + zoomVelocity);

            // Ŀ���ƶ����ƣ��ο�FreeCamera��WASDQE�߼���
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

        // ���½Ƕȣ��ο�FreeCamera�ĽǶȼ��㣩
        m_Yaw += xoffset;
        m_Pitch += yoffset;

        // ���Ƹ����ǣ����ⷭת����FreeCameraһ�£�
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        // ���ݽǶȼ������λ�ã��ο�FreeCamera�����������߼���
        glm::vec3 direction;
        direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction.y = sin(glm::radians(m_Pitch));
        direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction = glm::normalize(direction);

        Position = Target + direction * Distance; // Χ��Ŀ����ת
    }

} // namespace Intro