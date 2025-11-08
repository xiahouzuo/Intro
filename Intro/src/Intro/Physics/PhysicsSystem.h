// Physics/PhysicsSystem.h
#pragma once
#include "Intro/ECS/ECS.h"
#include "Intro/ECS/Components.h"
#include "Intro/Core.h"
#include "Intro/ECS/GameObject.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Intro
{

    // 碰撞信息
    struct CollisionInfo {
        GameObject entityA;
        GameObject entityB;
        glm::vec3 point;           // 碰撞点
        glm::vec3 normal;          // 碰撞法线
        float penetration;         // 穿透深度
        glm::vec3 relativeVelocity; // 相对速度
        bool isTrigger = false;    // 是否是触发器
    };

    // 射线检测结果
    struct RaycastHit {
        GameObject entity;
        glm::vec3 point;
        glm::vec3 normal;
        float distance;
        bool hit = false;
    };

    // 物理材质
    struct PhysicsMaterial {
        float bounciness = 0.0f;              // 弹性系数 [0,1]
        float staticFriction = 0.5f;          // 静摩擦系数
        float dynamicFriction = 0.3f;         // 动摩擦系数

        PhysicsMaterial() = default;
        PhysicsMaterial(float bounce, float staticFric, float dynamicFric)
            : bounciness(bounce), staticFriction(staticFric), dynamicFriction(dynamicFric) {
        }
    };
     
    // 物理配置
    struct PhysicsConfig {
        glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
        float fixedTimeStep = 1.0f / 60.0f;   // 固定时间步长
        int maxSubSteps = 10;                 // 最大子步数
        bool enableSleeping = true;           // 启用休眠
        float sleepThreshold = 0.1f;          // 休眠阈值
        bool enableCCD = false;               // 启用连续碰撞检测
    };

    class ITR_API PhysicsSystem {
    public:
        static void Initialize(const PhysicsConfig& config = PhysicsConfig());
        static void Shutdown();

        // 主更新函数
        static void OnUpdate(ECS& ecs, float deltaTime);
        static void OnUpdate(float deltaTime, ECS& ecs, bool isPlaying);

        // 射线检测
        static bool Raycast(ECS& ecs, const glm::vec3& origin,
            const glm::vec3& direction, float maxDistance,
            RaycastHit& hitInfo, uint32_t layerMask = 0xFFFFFFFF);

        static bool SphereCast(ECS& ecs, const glm::vec3& origin, float radius,
            const glm::vec3& direction, float maxDistance,
            RaycastHit& hitInfo, uint32_t layerMask = 0xFFFFFFFF);

        // 体积查询
        static std::vector<GameObject> OverlapSphere(ECS& ecs, const glm::vec3& center,
            float radius, uint32_t layerMask = 0xFFFFFFFF);
        static std::vector<GameObject> OverlapBox(ECS& ecs, const glm::vec3& center,
            const glm::vec3& halfExtents, uint32_t layerMask = 0xFFFFFFFF);

        // 力操作
        // 力模式枚举
        enum class ForceMode {
            Force,      // 持续力 (质量相关)
            Impulse,    // 冲量 (质量相关)
            Velocity,   // 直接改变速度 (质量无关)
            Acceleration // 直接改变加速度 (质量无关)
        };
        static void AddForce(GameObject entity, const glm::vec3& force, ForceMode mode = ForceMode::Force);
        static void AddForceAtPosition(GameObject entity, const glm::vec3& force,
            const glm::vec3& position, ForceMode mode = ForceMode::Force);
        static void AddTorque(GameObject entity, const glm::vec3& torque, ForceMode mode = ForceMode::Force);

        // 速度操作
        static void SetVelocity(GameObject entity, const glm::vec3& velocity);
        static void SetAngularVelocity(GameObject entity, const glm::vec3& angularVelocity);
        static glm::vec3 GetVelocity(GameObject entity);
        static glm::vec3 GetAngularVelocity(GameObject entity);

        // 调试绘制
        static void DebugDrawColliders(ECS& ecs, std::vector<glm::vec3>& lines);
        static void SetDebugDraw(bool enable) { s_DebugDraw = enable; }

        // 配置
        static void SetConfig(const PhysicsConfig& config) { s_Config = config; }
        static const PhysicsConfig& GetConfig() { return s_Config; }


    private:
        // 内部状态
        static PhysicsConfig s_Config;
        static float s_AccumulatedTime;
        static bool s_Initialized;
        static bool s_DebugDraw;

        // 碰撞对缓存
        static std::vector<CollisionInfo> s_CollisionPairs;
        static std::vector<CollisionInfo> s_TriggerPairs;

        // 内部更新步骤
        static void FixedUpdate(ECS& ecs, float fixedDeltaTime);
        static void IntegrateForces(ECS& ecs, float deltaTime);
        static void DetectCollisions(ECS& ecs);
        static void ResolveCollisions(ECS& ecs, float deltaTime);
        static void IntegrateVelocities(ECS& ecs, float deltaTime);
        static void UpdateSleepState(ECS& ecs, float deltaTime);

        // 碰撞检测方法
        static bool CheckCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckAABBCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckAABBSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static bool CheckSphereAABBCollision(const Transform& transformA, const ColliderComponent& colliderA,
            const Transform& transformB, const ColliderComponent& colliderB,
            CollisionInfo& collision);

        static void ApplyFriction(RigidbodyComponent& rbA, RigidbodyComponent& rbB,
            const CollisionInfo& collision, const PhysicsMaterial& materialA,
            const PhysicsMaterial& materialB);


        // 连续碰撞检测
        static bool CheckCCD(const Transform& transformA, const RigidbodyComponent& rbA, const ColliderComponent& colliderA,
            const Transform& transformB, const RigidbodyComponent& rbB, const ColliderComponent& colliderB,
            CollisionInfo& collision, float deltaTime);

        // 碰撞响应
        static void ResolveCollision(CollisionInfo& collision, float deltaTime);
        static void ApplyImpulse(RigidbodyComponent& rbA, RigidbodyComponent& rbB,
            const CollisionInfo& collision);

        // 工具函数
        static glm::vec3 GetColliderWorldPosition(const Transform& transform, const ColliderComponent& collider);
        static glm::vec3 GetColliderWorldSize(const Transform& transform, const ColliderComponent& collider);
        static float GetColliderWorldRadius(const Transform& transform, const ColliderComponent& collider);

        static PhysicsMaterial GetPhysicsMaterial(const ColliderComponent& collider);
        static float CombineFriction(float frictionA, float frictionB);
        static float CombineBounciness(float bouncinessA, float bouncinessB);

        // 调试绘制辅助
        static void GetColliderWireframe(const Transform& transform, const ColliderComponent& collider,
            std::vector<glm::vec3>& lines);
        static void GetAABBWireframe(const glm::vec3& center, const Transform& transform,
            const ColliderComponent& collider, std::vector<glm::vec3>& lines);
        static void GetSphereWireframe(const glm::vec3& center, const Transform& transform,
            const ColliderComponent& collider, std::vector<glm::vec3>& lines);
        static void GetCapsuleWireframe(const glm::vec3& center, const Transform& transform,
            const ColliderComponent& collider, std::vector<glm::vec3>& lines);

        // 射线检测辅助
        static bool RaycastAABB(const glm::vec3& origin, const glm::vec3& direction,
            const glm::vec3& center, const glm::vec3& halfExtents,
            float& distance);
        static bool RaycastSphere(const glm::vec3& origin, const glm::vec3& direction,
            const glm::vec3& center, float radius, float& distance);
    };
}