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
        using Entity = entt::entity; // ʵ������

        ECS() = default;
        ~ECS() = default;

        // ���ÿ�����entt::registry ���ɿ�����
        ECS(const ECS&) = delete;
        ECS& operator=(const ECS&) = delete;

        ECS(ECS&&) = delete;
        ECS& operator=(ECS&&) = delete;

        // ����ʵ��
        Entity CreateEntity() {
            return m_Registry.create();
        }

        // ����ʵ��
        void DestroyEntity(Entity entity) {
            m_Registry.destroy(entity);
        }

        // ������
        template <typename Component, typename... Args>
        Component& AddComponent(Entity entity, Args&&... args) {
            // emplace_or_replace �ᴴ�����滻�����������������
            
            if constexpr (std::is_invocable_v<decltype(&entt::registry::template emplace_or_replace<Component>), entt::registry, Entity, Args...>) {
                m_Registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
                return m_Registry.get<Component>(entity);
            }
            else {
                // ���ݾɰ棺���Ƴ��� emplace
                if (m_Registry.all_of<Component>(entity)) {
                    m_Registry.remove<Component>(entity);
                }
                m_Registry.emplace<Component>(entity, std::forward<Args>(args)...);
                return m_Registry.get<Component>(entity);
            }
        }

        // ��ȡ���
        template <typename Component>
        Component& GetComponent(Entity entity) {
            return m_Registry.get<Component>(entity);
        }

        // ��ȡֻ�������const �汾��
        template <typename Component>
        const Component& GetComponent(Entity entity) const {
            return m_Registry.get<Component>(entity);
        }

        // ���Ի�ȡ�������ȫ�������ڷ��� nullptr��
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

        // �� const ע�����ʣ������ϵͳ��Ҫ��
        entt::registry& GetRegistry() { return m_Registry; }

        // const ע�����ʣ�ֻ����
        const entt::registry& GetRegistry() const { return m_Registry; }

    private:
        entt::registry m_Registry; // EnTT �ĺ���ע���
    };

} // namespace Intro
