#pragma once
#include "Intro/Scripting/IScript.h"
#include "Intro/ECS/Components.h"
#include "Intro/Input.h"
#include <glm/glm.hpp>

namespace Intro {

    // 旋转脚本示例
    class RotatorScript : public IScript {
    public:
        void OnUpdate(float deltaTime) override {
            if (!GetGameObject().IsValid()) return;

            auto& transform = GetGameObject().GetComponent<TransformComponent>();
            transform.transform.rotation = glm::angleAxis(
                glm::radians(m_Angle),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
            m_Angle += m_RotationSpeed * deltaTime;
        }

        void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }

    private:
        float m_Angle = 0.0f;
        float m_RotationSpeed = 45.0f; // 度/秒
    };
}