#pragma once
#include "Intro/ECS/ECS.h"
#include "Intro/ECS/Components.h"
#include "Intro/Core.h"
#include "Intro/ECS/GameObject.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Intro
{
    // 碰撞信息
    struct CollisionInfo {
        GameObject entityA;
        GameObject entityB;
        glm::vec3 point;           // 碰撞点
        glm::vec3 normal;          // 碰撞法线
        float penetration;         // 穿透深度
    };

    // 射线检测结果
    struct RaycastHit {
        GameObject entity;
        glm::vec3 point;
        glm::vec3 normal;
        float distance;
    };

    class ITR_API PhysicsSystem {
    public:
        PhysicsSystem() = default;
        ~PhysicsSystem() = default;

        // 每帧更新物理
        static void OnUpdate(ECS& ecs, float deltaTime);
        static void OnUpdate( float deltaTime, ECS& ecs, bool isPlaying);

        // 射线检测
        static bool Raycast(ECS& ecs, const glm::vec3& origin,
            const glm::vec3& direction, float maxDistance,
            RaycastHit& hitInfo);

        // 添加力
        static void AddForce(GameObject entity, const glm::vec3& force);
        static void AddForceAtPosition(GameObject entity, const glm::vec3& force,
            const glm::vec3& position);

        // 设置速度
        static void SetVelocity(GameObject entity, const glm::vec3& velocity);

        static void DebugDrawColliders(ECS& ecs, std::vector<glm::vec3>& lines);
        static void GetColliderWireframe(const Transform& transform, const ColliderComponent& collider,
            std::vector<glm::vec3>& lines);
        static void GetAABBWireframe(const glm::vec3& center, const Transform& transform,
            const ColliderComponent& collider, std::vector<glm::vec3>& lines);
        static void GetSphereWireframe(const glm::vec3& center, const Transform& transform,
            const ColliderComponent& collider, std::vector<glm::vec3>& lines);
    private:
        // 内部辅助方法
        static void IntegrateForces(ECS& ecs, float deltaTime);
        static void DetectCollisions(ECS& ecs);
        static void ResolveCollisions(ECS& ecs);
        static void IntegrateVelocities(ECS& ecs, float deltaTime);

        // 碰撞检测方法
        static bool CheckAABBCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckAABBSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        // 工具函数
        static glm::vec3 GetColliderWorldPosition(const Transform& transform, const ColliderComponent& collider);
        static glm::vec3 GetColliderWorldSize(const Transform& transform, const ColliderComponent& collider);
    };

}