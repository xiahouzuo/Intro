#include "itrpch.h"
#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/MouseButtonCodes.h"

namespace Intro {

    OrbitCamera::OrbitCamera(const Window& window)
        : Camera(window)
    {
        // ��ʼ��������λ��
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
            // ������һ�ΰ���ʱ����
            m_FirstMouse = true;
            m_LastMouseX = Input::GetMouseX();
            m_LastMouseY = Input::GetMouseY();
        }

        // ����չ���������š�ƽ�� (�м���ק) ��
    }

    void OrbitCamera::RotateCamera(float xoffset, float yoffset)
    {
        xoffset *= m_MouseSensitivity;
        yoffset *= m_MouseSensitivity;

        glm::vec3 forward = glm::normalize(Target - Position);

        // ������Y����ת��ˮƽ��
        {
            glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset), Up);
            forward = glm::vec3(rotateY * glm::vec4(forward, 0.0f));
        }

        // ��������ת��������
        {
            glm::vec3 right = glm::normalize(glm::cross(forward, Up));
            glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset), right);
            forward = glm::vec3(rotateX * glm::vec4(forward, 0.0f));
        }

        // ����λ�ã����־���
        Position = Target - forward * Distance;
    }

} // namespace Intro
