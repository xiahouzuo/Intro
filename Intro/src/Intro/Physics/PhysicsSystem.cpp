// Physics/PhysicsSystem.cpp - 修复版本
#include "itrpch.h"
#include "PhysicsSystem.h"
#include "Intro/ECS/GameObject.h"
#include "Intro/Log.h"
#include <algorithm>
#include <glm/gtx/norm.hpp>

namespace Intro {

    // 静态成员定义
    PhysicsConfig PhysicsSystem::s_Config;
    float PhysicsSystem::s_AccumulatedTime = 0.0f;
    bool PhysicsSystem::s_Initialized = false;
    bool PhysicsSystem::s_DebugDraw = true;
    std::vector<CollisionInfo> PhysicsSystem::s_CollisionPairs;
    std::vector<CollisionInfo> PhysicsSystem::s_TriggerPairs;

    void PhysicsSystem::Initialize(const PhysicsConfig& config) {
        if (s_Initialized) return;

        s_Config = config;
        s_AccumulatedTime = 0.0f;
        s_Initialized = true;

        ITR_INFO("Physics System Initialized");
    }

    void PhysicsSystem::OnUpdate(float deltaTime, ECS& ecs, bool isPlaying) {
        if (!s_Initialized || !isPlaying) return;

        // 累积时间并执行固定时间步长更新
        s_AccumulatedTime += deltaTime;

        int numSubSteps = 0;
        while (s_AccumulatedTime >= s_Config.fixedTimeStep && numSubSteps < s_Config.maxSubSteps) {
            FixedUpdate(ecs, s_Config.fixedTimeStep);
            s_AccumulatedTime -= s_Config.fixedTimeStep;
            numSubSteps++;
        }
    }

    void PhysicsSystem::FixedUpdate(ECS& ecs, float fixedDeltaTime) {
        // 清除上一帧的碰撞对
        s_CollisionPairs.clear();
        s_TriggerPairs.clear();

        // 物理更新步骤
        IntegrateForces(ecs, fixedDeltaTime);
        DetectCollisions(ecs);
        ResolveCollisions(ecs, fixedDeltaTime);
        IntegrateVelocities(ecs, fixedDeltaTime);
    }

    void PhysicsSystem::IntegrateForces(ECS& ecs, float deltaTime) {
        auto view = ecs.GetRegistry().view<TransformComponent, RigidbodyComponent, ColliderComponent>();

        for (auto [entity, transform, rigidbody, collider] : view.each()) {
            if (rigidbody.isKinematic || !collider.enabled) continue;

            // 应用重力
            if (rigidbody.useGravity) {
                rigidbody.force += s_Config.gravity * rigidbody.mass;
            }

            // 计算加速度 (F = ma -> a = F/m)
            glm::vec3 acceleration = rigidbody.force / rigidbody.mass;
            rigidbody.velocity += acceleration * deltaTime;

            // 应用线性阻力 - 修复：使用更稳定的阻力计算
            if (glm::length(rigidbody.velocity) > 0.01f) {
                rigidbody.velocity *= (1.0f - rigidbody.drag * deltaTime);
                // 防止速度过小导致数值不稳定
                if (glm::length(rigidbody.velocity) < 0.01f) {
                    rigidbody.velocity = glm::vec3(0.0f);
                }
            }

            // 重置力
            rigidbody.force = glm::vec3(0.0f);
        }
    }

    void PhysicsSystem::DetectCollisions(ECS& ecs) {
        auto view = ecs.GetRegistry().view<TransformComponent, ColliderComponent>();
        std::vector<entt::entity> entities(view.begin(), view.end());

        // 简单的两两检测
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto entityA = entities[i];
                auto entityB = entities[j];

                auto& transformA = ecs.GetComponent<TransformComponent>(entityA).transform;
                auto& colliderA = ecs.GetComponent<ColliderComponent>(entityA);
                auto& transformB = ecs.GetComponent<TransformComponent>(entityB).transform;
                auto& colliderB = ecs.GetComponent<ColliderComponent>(entityB);

                if (!colliderA.enabled || !colliderB.enabled) continue;

                CollisionInfo collision;
                bool hasCollision = CheckCollision(transformA, colliderA, transformB, colliderB, collision);

                if (hasCollision) {
                    collision.entityA = GameObject(entityA, &ecs);
                    collision.entityB = GameObject(entityB, &ecs);
                    collision.isTrigger = colliderA.isTrigger || colliderB.isTrigger;

                    // 计算相对速度
                    if (collision.entityA.HasComponent<RigidbodyComponent>() &&
                        collision.entityB.HasComponent<RigidbodyComponent>()) {
                        auto& rbA = collision.entityA.GetComponent<RigidbodyComponent>();
                        auto& rbB = collision.entityB.GetComponent<RigidbodyComponent>();
                        collision.relativeVelocity = rbB.velocity - rbA.velocity;
                    }

                    if (collision.isTrigger) {
                        s_TriggerPairs.push_back(collision);
                    }
                    else {
                        s_CollisionPairs.push_back(collision);
                    }
                }
            }
        }
    }

    bool PhysicsSystem::CheckCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        // 根据碰撞体类型调用相应的检测函数
        if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Box) {
            return CheckAABBCollision(transformA, colliderA, transformB, colliderB, collision);
        }
        else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Sphere) {
            return CheckSphereCollision(transformA, colliderA, transformB, colliderB, collision);
        }
        else if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Sphere) {
            return CheckAABBSphereCollision(transformA, colliderA, transformB, colliderB, collision);
        }
        else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Box) {
            return CheckSphereAABBCollision(transformA, colliderA, transformB, colliderB, collision);
        }

        return false;
    }

    bool PhysicsSystem::CheckAABBCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        glm::vec3 posA = GetColliderWorldPosition(transformA, colliderA);
        glm::vec3 sizeA = GetColliderWorldSize(transformA, colliderA);
        glm::vec3 posB = GetColliderWorldPosition(transformB, colliderB);
        glm::vec3 sizeB = GetColliderWorldSize(transformB, colliderB);

        // AABB 碰撞检测
        bool collisionX = posA.x + sizeA.x / 2.0f >= posB.x - sizeB.x / 2.0f &&
            posB.x + sizeB.x / 2.0f >= posA.x - sizeA.x / 2.0f;
        bool collisionY = posA.y + sizeA.y / 2.0f >= posB.y - sizeB.y / 2.0f &&
            posB.y + sizeB.y / 2.0f >= posA.y - sizeA.y / 2.0f;
        bool collisionZ = posA.z + sizeA.z / 2.0f >= posB.z - sizeB.z / 2.0f &&
            posB.z + sizeB.z / 2.0f >= posA.z - sizeA.z / 2.0f;

        if (collisionX && collisionY && collisionZ) {
            // 计算穿透向量和法线
            glm::vec3 delta = posB - posA;
            glm::vec3 penetration = (sizeA + sizeB) / 2.0f - glm::abs(delta);

            // 找到最小穿透轴
            if (penetration.x < penetration.y && penetration.x < penetration.z) {
                collision.normal = glm::vec3(delta.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
                collision.penetration = penetration.x;
            }
            else if (penetration.y < penetration.z) {
                collision.normal = glm::vec3(0.0f, delta.y > 0 ? 1.0f : -1.0f, 0.0f);
                collision.penetration = penetration.y;
            }
            else {
                collision.normal = glm::vec3(0.0f, 0.0f, delta.z > 0 ? 1.0f : -1.0f);
                collision.penetration = penetration.z;
            }

            collision.point = posA + collision.normal * sizeA / 2.0f;
            return true;
        }

        return false;
    }

    bool PhysicsSystem::CheckSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        glm::vec3 posA = GetColliderWorldPosition(transformA, colliderA);
        glm::vec3 posB = GetColliderWorldPosition(transformB, colliderB);

        float radiusA = GetColliderWorldRadius(transformA, colliderA);
        float radiusB = GetColliderWorldRadius(transformB, colliderB);

        float distance = glm::length(posB - posA);
        float minDistance = radiusA + radiusB;

        // 修复：添加距离检查，避免除以零
        if (distance < 0.001f) {
            // 球体重合，使用默认法线
            collision.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            collision.penetration = minDistance;
            collision.point = posA;
            return true;
        }

        if (distance < minDistance) {
            collision.normal = glm::normalize(posB - posA);
            collision.penetration = minDistance - distance;
            collision.point = posA + collision.normal * radiusA;
            return true;
        }

        return false;
    }

    bool PhysicsSystem::CheckAABBSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        glm::vec3 boxPos = GetColliderWorldPosition(transformA, colliderA);
        glm::vec3 boxSize = GetColliderWorldSize(transformA, colliderA);
        glm::vec3 spherePos = GetColliderWorldPosition(transformB, colliderB);
        float sphereRadius = GetColliderWorldRadius(transformB, colliderB);

        // 找到AABB上距离球心最近的点
        glm::vec3 closestPoint;
        closestPoint.x = std::max(boxPos.x - boxSize.x / 2.0f, std::min(spherePos.x, boxPos.x + boxSize.x / 2.0f));
        closestPoint.y = std::max(boxPos.y - boxSize.y / 2.0f, std::min(spherePos.y, boxPos.y + boxSize.y / 2.0f));
        closestPoint.z = std::max(boxPos.z - boxSize.z / 2.0f, std::min(spherePos.z, boxPos.z + boxSize.z / 2.0f));

        float distance = glm::length(spherePos - closestPoint);

        // 修复：避免除以零和数值不稳定
        if (distance < 0.001f) {
            // 球心在AABB内部，使用从球心到AABB中心的方向作为法线
            glm::vec3 centerToSphere = spherePos - boxPos;
            if (glm::length(centerToSphere) < 0.001f) {
                collision.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else {
                collision.normal = glm::normalize(centerToSphere);
            }
            collision.penetration = sphereRadius;
            collision.point = closestPoint;
            return true;
        }

        if (distance < sphereRadius) {
            collision.normal = glm::normalize(spherePos - closestPoint);
            collision.penetration = sphereRadius - distance;
            collision.point = closestPoint;
            return true;
        }

        return false;
    }

    bool PhysicsSystem::CheckSphereAABBCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        // 交换A和B，然后调用AABB-Sphere检测
        bool result = CheckAABBSphereCollision(transformB, colliderB, transformA, colliderA, collision);
        if (result) {
            // 交换碰撞信息
            collision.normal = -collision.normal;
        }
        return result;
    }

    void PhysicsSystem::ResolveCollisions(ECS& ecs, float deltaTime) {
        // 处理触发器
        for (auto& trigger : s_TriggerPairs) {
            ITR_INFO("Trigger detected between {} and {}",
                trigger.entityA.GetName(), trigger.entityB.GetName());
        }

        // 处理碰撞 - 修复：限制每帧最大碰撞解决次数
        const int maxIterations = 4;
        for (int iteration = 0; iteration < maxIterations && !s_CollisionPairs.empty(); ++iteration) {
            for (auto& collision : s_CollisionPairs) {
                ResolveCollision(collision, deltaTime);
            }
        }
    }

    void PhysicsSystem::ResolveCollision(CollisionInfo& collision, float deltaTime) {
        if (!collision.entityA.IsValid() || !collision.entityB.IsValid()) {
            return;
        }

        // 检查是否都有刚体组件
        bool hasRbA = collision.entityA.HasComponent<RigidbodyComponent>();
        bool hasRbB = collision.entityB.HasComponent<RigidbodyComponent>();

        if (!hasRbA && !hasRbB) return;

        auto& transformA = collision.entityA.GetComponent<TransformComponent>();
        auto& transformB = collision.entityB.GetComponent<TransformComponent>();

        RigidbodyComponent* rbA = nullptr;
        RigidbodyComponent* rbB = nullptr;

        if (hasRbA) rbA = &collision.entityA.GetComponent<RigidbodyComponent>();
        if (hasRbB) rbB = &collision.entityB.GetComponent<RigidbodyComponent>();

        // 如果两个都是运动学刚体，不处理碰撞
        if ((rbA && rbA->isKinematic) && (rbB && rbB->isKinematic)) return;

        // 位置修正（防止穿透）- 修复：更温和的位置修正
        float correctionFactor = 0.2f; // 减少修正强度
        float slop = 0.01f; // 穿透容差

        if (collision.penetration > slop) {
            float totalInverseMass = 0.0f;
            if (rbA && !rbA->isKinematic) totalInverseMass += 1.0f / rbA->mass;
            if (rbB && !rbB->isKinematic) totalInverseMass += 1.0f / rbB->mass;

            if (totalInverseMass > 0.0f) {
                glm::vec3 correction = collision.normal * (collision.penetration - slop) * correctionFactor / totalInverseMass;

                if (rbA && !rbA->isKinematic) {
                    transformA.transform.position -= correction * (1.0f / rbA->mass);
                }
                if (rbB && !rbB->isKinematic) {
                    transformB.transform.position += correction * (1.0f / rbB->mass);
                }
            }
        }

        // 速度响应 - 修复：更保守的冲量计算
        if (rbA && rbB && !rbA->isKinematic && !rbB->isKinematic) {
            ApplyImpulse(*rbA, *rbB, collision);
        }

        ITR_INFO("Collision resolved - Penetration: {:.3f}, Normal: ({:.2f}, {:.2f}, {:.2f})",
            collision.penetration, collision.normal.x, collision.normal.y, collision.normal.z);
    }

    void PhysicsSystem::ApplyImpulse(RigidbodyComponent& rbA, RigidbodyComponent& rbB,
        const CollisionInfo& collision) {
        // 修复：更稳定的冲量计算
        glm::vec3 relativeVelocity = rbB.velocity - rbA.velocity;
        float velocityAlongNormal = glm::dot(relativeVelocity, collision.normal);

        // 如果物体正在分离，不应用冲量
        if (velocityAlongNormal > 0) return;

        // 修复：限制弹性系数，避免过度反弹
        float restitution = 0.2f; // 降低弹性

        float j = -(1.0f + restitution) * velocityAlongNormal;
        j /= (1.0f / rbA.mass + 1.0f / rbB.mass);

        // 修复：限制冲量大小，避免速度过大
        float maxImpulse = 10.0f; // 最大冲量
        j = std::min(j, maxImpulse);

        glm::vec3 impulse = j * collision.normal;

        rbA.velocity -= impulse / rbA.mass;
        rbB.velocity += impulse / rbB.mass;

        // 修复：限制最大速度，避免物体飞走
        float maxSpeed = 50.0f;
        if (glm::length(rbA.velocity) > maxSpeed) {
            rbA.velocity = glm::normalize(rbA.velocity) * maxSpeed;
        }
        if (glm::length(rbB.velocity) > maxSpeed) {
            rbB.velocity = glm::normalize(rbB.velocity) * maxSpeed;
        }
    }

    void PhysicsSystem::IntegrateVelocities(ECS& ecs, float deltaTime) {
        auto view = ecs.GetRegistry().view<TransformComponent, RigidbodyComponent>();

        for (auto [entity, transform, rigidbody] : view.each()) {
            if (rigidbody.isKinematic) continue;

            // 更新位置
            transform.transform.position += rigidbody.velocity * deltaTime;

            // 修复：简单的边界检查，防止物体掉出世界
            if (transform.transform.position.y < -100.0f) {
                transform.transform.position.y = 10.0f;
                rigidbody.velocity = glm::vec3(0.0f);
                ITR_WARN("Object respawned due to falling out of world");
            }
        }
    }

    // 工具函数实现
    glm::vec3 PhysicsSystem::GetColliderWorldPosition(const Transform& transform, const ColliderComponent& collider) {
        return transform.position + collider.offset;
    }

    glm::vec3 PhysicsSystem::GetColliderWorldSize(const Transform& transform, const ColliderComponent& collider) {
        return collider.size * transform.scale;
    }

    float PhysicsSystem::GetColliderWorldRadius(const Transform& transform, const ColliderComponent& collider) {
        float maxScale = std::max({ transform.scale.x, transform.scale.y, transform.scale.z });
        return collider.radius * maxScale;
    }

    // 调试绘制
    void PhysicsSystem::DebugDrawColliders(ECS& ecs, std::vector<glm::vec3>& lines) {
        if (!s_DebugDraw) return;

        lines.clear();
        auto view = ecs.GetRegistry().view<TransformComponent, ColliderComponent>();

        for (auto [entity, transform, collider] : view.each()) {
            if (!collider.enabled) continue;
            GetColliderWireframe(transform.transform, collider, lines);
        }
    }

    void PhysicsSystem::GetColliderWireframe(const Transform& transform, const ColliderComponent& collider,
        std::vector<glm::vec3>& lines) {
        glm::vec3 worldPos = GetColliderWorldPosition(transform, collider);

        switch (collider.type) {
        case ColliderType::Box:
            GetAABBWireframe(worldPos, transform, collider, lines);
            break;
        case ColliderType::Sphere:
            GetSphereWireframe(worldPos, transform, collider, lines);
            break;
        default:
            break;
        }
    }

    void PhysicsSystem::GetAABBWireframe(const glm::vec3& center, const Transform& transform,
        const ColliderComponent& collider, std::vector<glm::vec3>& lines) {
        glm::vec3 size = GetColliderWorldSize(transform, collider);
        glm::vec3 halfSize = size * 0.5f;

        // 8个顶点
        std::vector<glm::vec3> vertices = {
            center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x, -halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x,  halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z)
        };

        // 12条边
        const std::vector<std::pair<int, int>> edges = {
            {0,1}, {1,2}, {2,3}, {3,0}, // 底部
            {4,5}, {5,6}, {6,7}, {7,4}, // 顶部
            {0,4}, {1,5}, {2,6}, {3,7}  // 垂直边
        };

        for (const auto& edge : edges) {
            lines.push_back(vertices[edge.first]);
            lines.push_back(vertices[edge.second]);
        }
    }

    void PhysicsSystem::GetSphereWireframe(const glm::vec3& center, const Transform& transform,
        const ColliderComponent& collider, std::vector<glm::vec3>& lines) {
        float radius = GetColliderWorldRadius(transform, collider);
        const int segments = 16; // 减少细分段数以提高性能

        // 生成三个方向的圆环
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float)i / segments * 2.0f * glm::pi<float>();
            float angle2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

            // XY平面
            glm::vec3 v1 = center + radius * glm::vec3(cos(angle1), sin(angle1), 0.0f);
            glm::vec3 v2 = center + radius * glm::vec3(cos(angle2), sin(angle2), 0.0f);
            lines.push_back(v1); lines.push_back(v2);

            // XZ平面
            v1 = center + radius * glm::vec3(cos(angle1), 0.0f, sin(angle1));
            v2 = center + radius * glm::vec3(cos(angle2), 0.0f, sin(angle2));
            lines.push_back(v1); lines.push_back(v2);

            // YZ平面
            v1 = center + radius * glm::vec3(0.0f, cos(angle1), sin(angle1));
            v2 = center + radius * glm::vec3(0.0f, cos(angle2), sin(angle2));
            lines.push_back(v1); lines.push_back(v2);
        }
    }

} // namespace Intro