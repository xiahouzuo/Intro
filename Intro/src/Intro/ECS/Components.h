// ECS/Components.h
#pragma once
#include "Intro/Math/Transform.h" // 你的 Transform 类
#include "Intro/Renderer/Mesh.h"  // 你的 Mesh 类
#include "Intro/Renderer/Model.h"
#include "Intro/Renderer/Material.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

namespace Intro {

    
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

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {
        }
    };

    enum class LightType {
        Directional = 0,
        Point = 1,
        Spot = 2
    };

    struct LightComponent {
        LightType Type = LightType::Directional;
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;

        // Directional specific
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);

        // Point/Spot specific
        float Range = 10.0f;

        // Spot specific (degrees)
        float SpotAngle = 30.0f;
        float InnerSpotAngle = 20.0f;

        bool CastShadows = false;
        LightComponent() = default;
    };

    // 简单材质组件：持有 Material 智能指针（后面实现 Material 类）
    struct MaterialComponent {
        std::shared_ptr<Material> material;
        bool Transparent = false;
        MaterialComponent(std::shared_ptr<Material> mat = nullptr, bool transparent = false)
            : material(std::move(mat)), Transparent(transparent) {
        }
    };

} // namespace Intro