// ECS/Components.h
#pragma once
#include "Intro/Math/Transform.h" // 你的 Transform 类
#include "Intro/Renderer/Mesh.h"  // 你的 Mesh 类
#include "Intro/Assert/Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

namespace Intro {

    // 如果没有包含 Model.h，可以用前向声明（shared_ptr 支持不完整类型）
    class Model;

    // 变换组件：所有实体都可以有
    struct TransformComponent {
        Transform transform; // 直接用你现有的 Transform 类

        TransformComponent(
            const glm::vec3& pos = glm::vec3(0.0f),
            const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            const glm::vec3& scale = glm::vec3(1.0f))
            : transform(pos, rot, scale) {
        }
    };

    // 网格组件：只有需要被渲染的实体才有
    struct MeshComponent {
        std::shared_ptr<Mesh> mesh; // 持有 Mesh 的智能指针

        MeshComponent(std::shared_ptr<Mesh> meshPtr = nullptr)
            : mesh(std::move(meshPtr)) {
        }
    };

    // 模型组件：如果你需要加载外部模型（可选）
    struct ModelComponent {
        std::shared_ptr<Model> model; // 持有 Model 的智能指针

        ModelComponent(std::shared_ptr<Model> modelPtr = nullptr)
            : model(std::move(modelPtr)) {
        }
    };


} // namespace Intro