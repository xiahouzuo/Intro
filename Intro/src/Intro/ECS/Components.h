// ECS/Components.h
#pragma once
#include "Intro/Math/Transform.h" // 你的 Transform 类
#include "Intro/Renderer/Mesh.h"  // 你的 Mesh 类
#include "Intro/Renderer/Model.h"
#include "Intro/Renderer/Cameras/Camera.h"
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

    struct CameraComponent {
        // 相机参数
        float fov = 60.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
        bool isMainCamera = false;

        CameraComponent() = default;
        CameraComponent(float fov, float nearClip = 0.1f, float farClip = 1000.0f)
            : fov(fov), nearClip(nearClip), farClip(farClip) {
        }
    };

    // 简单材质组件
    struct MaterialComponent {
        std::shared_ptr<Material> material;
        bool Transparent = false;
        MaterialComponent(std::shared_ptr<Material> mat = nullptr, bool transparent = false)
            : material(std::move(mat)), Transparent(transparent) {
        }
    };


// 碰撞体类型
    enum class ColliderType {
        None = 0,
        Box,
        Sphere,
        Capsule,
        Mesh  // 用于复杂模型
    };

    // 基础碰撞体组件
    struct ColliderComponent {
        ColliderType type = ColliderType::Box;
        glm::vec3 size = glm::vec3(1.0f);      // 对于Box：长宽高
        float radius = 0.5f;                   // 对于Sphere/Capsule：半径
        float height = 2.0f;                   // 对于Capsule：高度
        glm::vec3 offset = glm::vec3(0.0f);    // 相对于物体中心的偏移

        bool isTrigger = false;                // 是否是触发器
        bool enabled = true;

        ColliderComponent() = default;
        ColliderComponent(ColliderType colliderType) : type(colliderType) {}
    };

    // 刚体组件
    struct RigidbodyComponent {
        float mass = 1.0f;
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 angularVelocity = glm::vec3(0.0f);
        glm::vec3 force = glm::vec3(0.0f);
        glm::vec3 torque = glm::vec3(0.0f);

        float drag = 0.0f;                    // 线性阻力
        float angularDrag = 0.05f;            // 角阻力

        bool useGravity = true;
        bool isKinematic = false;             // 是否由代码控制（不受物理影响）
        bool freezeRotation = false;          // 冻结旋转

        RigidbodyComponent() = default;
    };

    // 物理材质
    struct PhysicsMaterial {
        float bounciness = 0.0f;              // 弹性
        float friction = 0.5f;                // 摩擦力
        float dynamicFriction = 0.3f;         // 动摩擦
    };

} // namespace Intro