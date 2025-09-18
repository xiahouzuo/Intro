#include "itrpch.h"
#include "Camera.h"
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Intro/Input.h"
#include "Intro/MouseButtonCodes.h"


namespace Intro{
	Camera::Camera(const Window& window)
	{
		AspectRatio = (float)window.GetWidth() / (float)window.GetHeight();
	}
	glm::mat4 Camera::GetViewMat() const
	{
		return glm::lookAt(Position, Target, Up);
	}

	glm::mat4 Camera::GetProjectionMat() const
	{
		return glm::perspective(glm::radians(Fov), AspectRatio, NearClip, FarClip);
	}

	void Camera::OnUpdate(float deltaTime)
	{

		if (Input::IsMouseButtonPressed(ITR_MOUSE_BUTTON_1))
		{

			float currentX = Input::GetMouseX();
			float currentY = Input::GetMouseY();

			// ����ƫ��������ǰ - ��һ֡��
			float xoffset = currentX - m_LastMouseX;
			float yoffset = m_LastMouseY - currentY;  // Y�ᷴ�򣨴���Y���£����Y���ϣ�

			// ���¡���һ֡���λ�á�Ϊ��ǰλ�ã�����һ֡���㣩
			m_LastMouseX = currentX;
			m_LastMouseY = currentY;

			RotateCamera(xoffset, yoffset);
		}
		else {
			// ���δ��ס�Ҽ������¡���һ֡���λ�á�Ϊ��ǰλ�ã������´ΰ�סʱ���䣩
			m_LastMouseX = Input::GetMouseX();
			m_LastMouseY = Input::GetMouseY();
		}


		//���������ƶ�
	}
	void Camera::RotateCamera(float xoffset, float yoffset)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		glm::vec3 forward = glm::normalize(Target - Position);

		// 1. ��Y����ת�����ң�
		{
			glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset), Up);
			forward = glm::vec3(rotateY * glm::vec4(forward, 1.0f));
		}

		// 2. ��X����ת�����£���ת��Ϊ������������
		{
			glm::vec3 right = glm::cross(forward, Up); // ��������������ֱ��forward��Up��
			glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset), right);
			forward = glm::vec3(rotateX * glm::vec4(forward, 1.0f));
		}

		// �������λ�ã����ֵ�Target�ľ��룩
		float distance = glm::distance(Position, Target);
		Position = Target - forward * distance;
	}
}