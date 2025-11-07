// GameObject.h
#pragma once
#include "ECS.h"
#include "Components.h"
#include "Intro/Core.h"
#include <string>
#include <vector>
#include <memory>

namespace Intro {

    // 前置声明
    class Scene;

    // 激活状态组件
    struct ActiveComponent {
        bool active = true;
    };

    class ITR_API GameObject {
    public:
        // 构造/析构
        GameObject();
        GameObject(ECS::Entity entity, ECS* ecs);
        ~GameObject() = default;

        // 允许拷贝（使用默认实现）
        GameObject(const GameObject&) = default;
        GameObject& operator=(const GameObject&) = default;

        // 允许移动
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        // 组件管理
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args);

        template<typename T>
        T& GetComponent();

        template<typename T>
        const T& GetComponent() const;

        template<typename T>
        bool HasComponent() const;

        template<typename T>
        void RemoveComponent();

        // 基础属性
        std::string GetName() const;
        void SetName(const std::string& name);

        Transform& GetTransform();
        const Transform& GetTransform() const;

        void SetActive(bool active);
        bool IsActive() const;

        // 实体操作
        ECS::Entity GetEntity() const { return m_Entity; }
        bool IsValid() const;

        // 静态查找方法
        static GameObject Find(Scene* scene, const std::string& name);
        static std::vector<GameObject> FindGameObjectsWithTag(Scene* scene, const std::string& tag);
        static GameObject FindWithTag(Scene* scene, const std::string& tag);

        // 比较操作
        bool operator==(const GameObject& other) const {
            return m_Entity == other.m_Entity && m_ECS == other.m_ECS;
        }
        bool operator!=(const GameObject& other) const {
            return !(*this == other);
        }

    private:
        ECS::Entity m_Entity{ entt::null };
        ECS* m_ECS = nullptr;
    };

} // namespace Intro

// 模板方法实现
namespace Intro {

    template<typename T, typename... Args>
    T& GameObject::AddComponent(Args&&... args) {
        ITR_CORE_ASSERT(m_ECS, "GameObject has no associated ECS!");
        ITR_CORE_ASSERT(IsValid(), "GameObject is not valid!");
        return m_ECS->AddComponent<T>(m_Entity, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GameObject::GetComponent() {
        ITR_CORE_ASSERT(m_ECS, "GameObject has no associated ECS!");
        ITR_CORE_ASSERT(IsValid(), "GameObject is not valid!");
        ITR_CORE_ASSERT(HasComponent<T>(), "GameObject does not have the requested component!");
        return m_ECS->GetComponent<T>(m_Entity);
    }

    template<typename T>
    const T& GameObject::GetComponent() const {
        ITR_CORE_ASSERT(m_ECS, "GameObject has no associated ECS!");
        ITR_CORE_ASSERT(IsValid(), "GameObject is not valid!");
        ITR_CORE_ASSERT(HasComponent<T>(), "GameObject does not have the requested component!");
        return m_ECS->GetComponent<T>(m_Entity);
    }

    template<typename T>
    bool GameObject::HasComponent() const {
        if (!m_ECS || !IsValid()) return false;
        return m_ECS->HasComponent<T>(m_Entity);
    }

    template<typename T>
    void GameObject::RemoveComponent() {
        if (!m_ECS || !IsValid()) return;
        if (HasComponent<T>()) {
            m_ECS->RemoveComponent<T>(m_Entity);
        }
    }

} // namespace Intro