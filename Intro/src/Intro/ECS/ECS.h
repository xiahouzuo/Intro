// ECS/ECS.h
#pragma once

#include <entt/entt.hpp>
#include "Components.h"
#include <utility> // for std::forward
#include <type_traits>
#include "Intro/Core.h"

namespace Intro {

    class ITR_API ECS {
    public:
        using Entity = entt::entity; // 实体类型

        ECS() = default;
        ~ECS() = default;

        // 禁用拷贝（entt::registry 不可拷贝）
        ECS(const ECS&) = delete;
        ECS& operator=(const ECS&) = delete;

        ECS(ECS&&) = delete;
        ECS& operator=(ECS&&) = delete;

        // 创建实体
        Entity CreateEntity() {
            return m_Registry.create();
        }

        // 销毁实体
        void DestroyEntity(Entity entity) {
            m_Registry.destroy(entity);
        }

        // 添加组件
        template <typename Component, typename... Args>
        Component& AddComponent(Entity entity, Args&&... args) {
            // emplace_or_replace 会创建或替换现有组件并返回引用
            
            if constexpr (std::is_invocable_v<decltype(&entt::registry::template emplace_or_replace<Component>), entt::registry, Entity, Args...>) {
                m_Registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
                return m_Registry.get<Component>(entity);
            }
            else {
                // 兼容旧版：先移除再 emplace
                if (m_Registry.all_of<Component>(entity)) {
                    m_Registry.remove<Component>(entity);
                }
                m_Registry.emplace<Component>(entity, std::forward<Args>(args)...);
                return m_Registry.get<Component>(entity);
            }
        }

        // 获取组件
        template <typename Component>
        Component& GetComponent(Entity entity) {
            return m_Registry.get<Component>(entity);
        }

        // 获取只读组件（const 版本）
        template <typename Component>
        const Component& GetComponent(Entity entity) const {
            return m_Registry.get<Component>(entity);
        }

        // 尝试获取组件（安全，不存在返回 nullptr）
        template <typename Component>
        Component* TryGetComponent(Entity entity) {
            return m_Registry.try_get<Component>(entity);
        }

        template <typename Component>
        const Component* TryGetComponent(Entity entity) const {
            return m_Registry.try_get<Component>(entity);
        }

        template <typename Component>
        bool HasComponent(Entity entity) const {
            return m_Registry.all_of<Component>(entity);
        }

        template <typename Component>
        void RemoveComponent(Entity entity) {
            if (m_Registry.all_of<Component>(entity)) {
                m_Registry.remove<Component>(entity);
            }
        }

        // 非 const 注册表访问（大多数系统需要）
        entt::registry& GetRegistry() { return m_Registry; }

        // const 注册表访问（只读）
        const entt::registry& GetRegistry() const { return m_Registry; }

    private:
        entt::registry m_Registry; // EnTT 的核心注册表
    };

} // namespace Intro
