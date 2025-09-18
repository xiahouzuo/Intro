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

			// 计算偏移量（当前 - 上一帧）
			float xoffset = currentX - m_LastMouseX;
			float yoffset = m_LastMouseY - currentY;  // Y轴反向（窗口Y向下，相机Y向上）

			// 更新“上一帧鼠标位置”为当前位置（供下一帧计算）
			m_LastMouseX = currentX;
			m_LastMouseY = currentY;

			RotateCamera(xoffset, yoffset);
		}
		else {
			// 如果未按住右键，更新“上一帧鼠标位置”为当前位置（避免下次按住时跳变）
			m_LastMouseX = Input::GetMouseX();
			m_LastMouseY = Input::GetMouseY();
		}


		//键盘输入移动
	}
	void Camera::RotateCamera(float xoffset, float yoffset)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		glm::vec3 forward = glm::normalize(Target - Position);

		// 1. 绕Y轴旋转（左右）
		{
			glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset), Up);
			forward = glm::vec3(rotateY * glm::vec4(forward, 1.0f));
		}

		// 2. 绕X轴旋转（上下，旋转轴为“右向量”）
		{
			glm::vec3 right = glm::cross(forward, Up); // 计算右向量（垂直于forward和Up）
			glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset), right);
			forward = glm::vec3(rotateX * glm::vec4(forward, 1.0f));
		}

		// 更新相机位置（保持到Target的距离）
		float distance = glm::distance(Position, Target);
		Position = Target - forward * distance;
	}
}