#pragma once
#include "Intro/Core.h"
#include "glm/glm.hpp"
#include "Intro/Window.h"

namespace Intro {

	class ITR_API Camera
	{
	private:
		float m_LastMouseX = 0.0f;
		float m_LastMouseY = 0.0f;

		float m_MouseSensitivity = 0.1f;

	private:
		void RotateCamera(float xoffset, float yoffset);

	public:
		glm::vec3 Position = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 Target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
		
		float Fov = 45.0f;
		float AspectRatio;
		float NearClip = 0.1f, FarClip = 100.0f;

	public:
		Camera(const Window& window);
		glm::mat4 GetViewMat() const;
		glm::mat4 GetProjectionMat() const;

		void OnUpdate(float deltaTime);

	};

}