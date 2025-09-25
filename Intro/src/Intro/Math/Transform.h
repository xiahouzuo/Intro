#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


struct Transform
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	Transform(const glm::vec3& pos, const glm::quat& rotation, const glm::vec3& scale)
		:position(pos), rotation(rotation), scale(scale)
	{

	}

	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		glm::mat4 rotMat = glm::mat4_cast(rotation);

		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);

		return transMat * rotMat * scaleMat;
	}

	void SetRotationEuler(const glm::vec3& eulerDegrees)
	{
		rotation = glm::quat(glm::radians(eulerDegrees));
	}

	glm::vec3 GetRotationEuler() const
	{
		return glm::degrees(glm::eulerAngles(rotation));
	}

};
