// Physics/PhysicsSystem.cpp
#include "itrpch.h"
#include "PhysicsSystem.h"
#include "Intro/ECS/GameObject.h"
#include <algorithm>

namespace Intro {

    // 重力加速度
    static const glm::vec3 GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);

    void PhysicsSystem::OnUpdate(ECS& ecs, float deltaTime) {
        // 限制时间步长避免不稳定
        float fixedDeltaTime = std::min(deltaTime, 1.0f / 30.0f);

        IntegrateForces(ecs, fixedDeltaTime);
        DetectCollisions(ecs);
        ResolveCollisions(ecs);
        IntegrateVelocities(ecs, fixedDeltaTime);
    }
    void PhysicsSystem::OnUpdate(float deltaTime, ECS& ecs, bool isPlaying)
    {
        if (!isPlaying) {
            return; // 不在运行状态时跳过物理更新
        }

        // 原有的物理更新代码保持不变
        float fixedDeltaTime = std::min(deltaTime, 1.0f / 30.0f);
        IntegrateForces(ecs, fixedDeltaTime);
        DetectCollisions(ecs);
        ResolveCollisions(ecs);
        IntegrateVelocities(ecs, fixedDeltaTime);
    }

    void PhysicsSystem::IntegrateForces(ECS& ecs, float deltaTime) {
        auto view = ecs.GetRegistry().view<TransformComponent, RigidbodyComponent, ColliderComponent>();

        for (auto [entity, transform, rigidbody, collider] : view.each()) {
            if (rigidbody.isKinematic || !collider.enabled) continue;

            // 应用重力
            if (rigidbody.useGravity) {
                rigidbody.force += GRAVITY * rigidbody.mass;
            }

            // 计算加速度 (F = ma -> a = F/m)
            glm::vec3 acceleration = rigidbody.force / rigidbody.mass;
            rigidbody.velocity += acceleration * deltaTime;

            // 应用阻力
            rigidbody.velocity *= (1.0f - rigidbody.drag * deltaTime);

            // 重置力
            rigidbody.force = glm::vec3(0.0f);
        }
    }

    void PhysicsSystem::DetectCollisions(ECS& ecs) {
        static std::vector<CollisionInfo> collisions;
        collisions.clear();

        auto view = ecs.GetRegistry().view<TransformComponent, ColliderComponent>();
        std::vector<entt::entity> entities(view.begin(), view.end());

        // 简单的两两检测（可以优化为空间划分）
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
                bool hasCollision = false;

                // 根据碰撞体类型进行检测
                if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Box) {
                    hasCollision = CheckAABBCollision(transformA, colliderA, transformB, colliderB, collision);
                }
                else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Sphere) {
                    hasCollision = CheckSphereCollision(transformA, colliderA, transformB, colliderB, collision);
                }
                else {
                    // 混合碰撞体类型检测
                    if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Sphere) {
                        hasCollision = CheckAABBSphereCollision(transformA, colliderA, transformB, colliderB, collision);
                    }
                    else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Box) {
                        hasCollision = CheckAABBSphereCollision(transformB, colliderB, transformA, colliderA, collision);
                        // 交换碰撞信息
                        if (hasCollision) {
                            std::swap(collision.entityA, collision.entityB);
                            collision.normal = -collision.normal;
                        }
                    }
                }

                if (hasCollision) {
                    collision.entityA = GameObject(entityA, &ecs);
                    collision.entityB = GameObject(entityB, &ecs);
                    collisions.push_back(collision);
                }
            }
        }

        // 处理碰撞（这里可以发送碰撞事件）
        for (auto& collision : collisions) {
            // TODO: 发送碰撞事件
            ITR_INFO("Collision detected between {} and {}",
                collision.entityA.GetName(), collision.entityB.GetName());
        }
    }

    void PhysicsSystem::ResolveCollisions(ECS& ecs) {
        // 简单的碰撞响应实现
        auto view = ecs.GetRegistry().view<TransformComponent, RigidbodyComponent>();

        for (auto [entity, transform, rigidbody] : view.each()) {
            if (rigidbody.isKinematic) continue;

            // 简单的边界检查（防止物体掉出世界）
            if (transform.transform.position.y < -10.0f) {
                transform.transform.position.y = 10.0f;
                rigidbody.velocity.y = 0.0f;
            }
        }
    }

    void PhysicsSystem::IntegrateVelocities(ECS& ecs, float deltaTime) {
        auto view = ecs.GetRegistry().view<TransformComponent, RigidbodyComponent>();

        for (auto [entity, transform, rigidbody] : view.each()) {
            if (rigidbody.isKinematic) continue;

            // 更新位置
            transform.transform.position += rigidbody.velocity * deltaTime;

            // 更新旋转（如果有角速度）
            if (!rigidbody.freezeRotation && glm::length(rigidbody.angularVelocity) > 0.0f) {
                glm::quat rotationDelta = glm::quat(rigidbody.angularVelocity * deltaTime);
                transform.transform.rotation = rotationDelta * transform.transform.rotation;
                rigidbody.angularVelocity *= (1.0f - rigidbody.angularDrag * deltaTime);
            }
        }
    }

    // AABB 碰撞检测
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

    // 球体碰撞检测
    bool PhysicsSystem::CheckSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {
        glm::vec3 posA = GetColliderWorldPosition(transformA, colliderA);
        glm::vec3 posB = GetColliderWorldPosition(transformB, colliderB);

        float radiusA = colliderA.radius * std::max({ transformA.scale.x, transformA.scale.y, transformA.scale.z });
        float radiusB = colliderB.radius * std::max({ transformB.scale.x, transformB.scale.y, transformB.scale.z });

        float distance = glm::length(posB - posA);
        float minDistance = radiusA + radiusB;

        if (distance < minDistance) {
            collision.normal = glm::normalize(posB - posA);
            collision.penetration = minDistance - distance;
            collision.point = posA + collision.normal * radiusA;
            return true;
        }

        return false;
    }

    // AABB-球体碰撞检测
    bool PhysicsSystem::CheckAABBSphereCollision(const Transform& transformA, const ColliderComponent& colliderA,
        const Transform& transformB, const ColliderComponent& colliderB,
        CollisionInfo& collision) {

        glm::vec3 boxPos = GetColliderWorldPosition(transformA, colliderA);
        glm::vec3 boxSize = GetColliderWorldSize(transformA, colliderA);
        glm::vec3 spherePos = GetColliderWorldPosition(transformB, colliderB);
        float sphereRadius = colliderB.radius * std::max({ transformB.scale.x, transformB.scale.y, transformB.scale.z });

        // 添加调试信息
        ITR_INFO("=== AABB-Sphere Collision Check ===");
        ITR_INFO("Box - Pos: ({:.2f}, {:.2f}, {:.2f}), Size: ({:.2f}, {:.2f}, {:.2f})",
            boxPos.x, boxPos.y, boxPos.z, boxSize.x, boxSize.y, boxSize.z);
        ITR_INFO("Sphere - Pos: ({:.2f}, {:.2f}, {:.2f}), Radius: {:.2f}",
            spherePos.x, spherePos.y, spherePos.z, sphereRadius);
        ITR_INFO("Box Collider Type: {}, Sphere Collider Type: {}",
            (int)colliderA.type, (int)colliderB.type);
        ITR_INFO("Box Transform - Pos: ({:.2f}, {:.2f}, {:.2f}), Scale: ({:.2f}, {:.2f}, {:.2f})",
            transformA.position.x, transformA.position.y, transformA.position.z,
            transformA.scale.x, transformA.scale.y, transformA.scale.z);
        ITR_INFO("Sphere Transform - Pos: ({:.2f}, {:.2f}, {:.2f}), Scale: ({:.2f}, {:.2f}, {:.2f})",
            transformB.position.x, transformB.position.y, transformB.position.z,
            transformB.scale.x, transformB.scale.y, transformB.scale.z);

        // 找到AABB上距离球心最近的点
        glm::vec3 closestPoint;
        closestPoint.x = std::max(boxPos.x - boxSize.x / 2.0f, std::min(spherePos.x, boxPos.x + boxSize.x / 2.0f));
        closestPoint.y = std::max(boxPos.y - boxSize.y / 2.0f, std::min(spherePos.y, boxPos.y + boxSize.y / 2.0f));
        closestPoint.z = std::max(boxPos.z - boxSize.z / 2.0f, std::min(spherePos.z, boxPos.z + boxSize.z / 2.0f));

        ITR_INFO("Closest point on AABB to sphere: ({:.2f}, {:.2f}, {:.2f})",
            closestPoint.x, closestPoint.y, closestPoint.z);

        float distance = glm::length(spherePos - closestPoint);
        ITR_INFO("Distance between sphere center and closest point: {:.2f}", distance);
        ITR_INFO("Sphere radius: {:.2f}", sphereRadius);

        if (distance < sphereRadius) {
            collision.normal = glm::normalize(spherePos - closestPoint);
            collision.penetration = sphereRadius - distance;
            collision.point = closestPoint;

            ITR_INFO("*** COLLISION DETECTED ***");
            ITR_INFO("Collision normal: ({:.2f}, {:.2f}, {:.2f})",
                collision.normal.x, collision.normal.y, collision.normal.z);
            ITR_INFO("Penetration depth: {:.2f}", collision.penetration);

            return true;
        }
        else {
            ITR_INFO("No collision - distance > radius");
        }

        return false;
    }

    void PhysicsSystem::DebugDrawColliders(ECS& ecs, std::vector<glm::vec3>& lines) {
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
        case ColliderType::Box: {
            GetAABBWireframe(worldPos, transform, collider, lines);
            break;
        }
        case ColliderType::Sphere: {
            GetSphereWireframe(worldPos, transform, collider, lines);
            break;
        }
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
            // 底部四个顶点
            center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x, -halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
            // 顶部四个顶点  
            center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(halfSize.x,  halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z)
        };

        // 12条边（每边2个顶点）
        // 底部矩形
        lines.push_back(vertices[0]); lines.push_back(vertices[1]);
        lines.push_back(vertices[1]); lines.push_back(vertices[2]);
        lines.push_back(vertices[2]); lines.push_back(vertices[3]);
        lines.push_back(vertices[3]); lines.push_back(vertices[0]);

        // 顶部矩形
        lines.push_back(vertices[4]); lines.push_back(vertices[5]);
        lines.push_back(vertices[5]); lines.push_back(vertices[6]);
        lines.push_back(vertices[6]); lines.push_back(vertices[7]);
        lines.push_back(vertices[7]); lines.push_back(vertices[4]);

        // 垂直边
        lines.push_back(vertices[0]); lines.push_back(vertices[4]);
        lines.push_back(vertices[1]); lines.push_back(vertices[5]);
        lines.push_back(vertices[2]); lines.push_back(vertices[6]);
        lines.push_back(vertices[3]); lines.push_back(vertices[7]);
    }

    void PhysicsSystem::GetSphereWireframe(const glm::vec3& center, const Transform& transform,
        const ColliderComponent& collider, std::vector<glm::vec3>& lines) {
        float radius = collider.radius * std::max({ transform.scale.x, transform.scale.y, transform.scale.z });
        const int segments = 24; // 球体的细分段数

        // 生成三个方向的圆环：XY平面、XZ平面、YZ平面

        // XY平面（水平圆）
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float)i / segments * 2.0f * glm::pi<float>();
            float angle2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

            glm::vec3 v1 = center + radius * glm::vec3(cos(angle1), sin(angle1), 0.0f);
            glm::vec3 v2 = center + radius * glm::vec3(cos(angle2), sin(angle2), 0.0f);

            lines.push_back(v1);
            lines.push_back(v2);
        }

        // XZ平面（垂直圆，绕Y轴）
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float)i / segments * 2.0f * glm::pi<float>();
            float angle2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

            glm::vec3 v1 = center + radius * glm::vec3(cos(angle1), 0.0f, sin(angle1));
            glm::vec3 v2 = center + radius * glm::vec3(cos(angle2), 0.0f, sin(angle2));

            lines.push_back(v1);
            lines.push_back(v2);
        }

        // YZ平面（垂直圆，绕X轴）
        for (int i = 0; i < segments; ++i) {
            float angle1 = (float)i / segments * 2.0f * glm::pi<float>();
            float angle2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

            glm::vec3 v1 = center + radius * glm::vec3(0.0f, cos(angle1), sin(angle1));
            glm::vec3 v2 = center + radius * glm::vec3(0.0f, cos(angle2), sin(angle2));

            lines.push_back(v1);
            lines.push_back(v2);
        }
    }

    // 工具函数实现
    glm::vec3 PhysicsSystem::GetColliderWorldPosition(const Transform& transform, const ColliderComponent& collider) {
        return transform.position + transform.rotation * collider.offset;
    }

    glm::vec3 PhysicsSystem::GetColliderWorldSize(const Transform& transform, const ColliderComponent& collider) {
        return collider.size * transform.scale;
    }

    // 其他方法实现...
    bool PhysicsSystem::Raycast(ECS& ecs, const glm::vec3& origin, const glm::vec3& direction,
        float maxDistance, RaycastHit& hitInfo) {
        // 简化的射线检测实现
        // TODO: 实现完整的射线与各种碰撞体检测
        return false;
    }

    void PhysicsSystem::AddForce(GameObject entity, const glm::vec3& force) {
        if (!entity.IsValid() || !entity.HasComponent<RigidbodyComponent>()) return;
        entity.GetComponent<RigidbodyComponent>().force += force;
    }

    void PhysicsSystem::SetVelocity(GameObject entity, const glm::vec3& velocity) {
        if (!entity.IsValid() || !entity.HasComponent<RigidbodyComponent>()) return;
        entity.GetComponent<RigidbodyComponent>().velocity = velocity;
    }

} // namespace Intro