// GameObject.cpp
#include "itrpch.h"
#include "GameObject.h"
#include "Scene.h"
#include "Intro/Log.h"

namespace Intro {

    // -------------------------------------------------------------------------
    // GameObject implementation
    // -------------------------------------------------------------------------

    GameObject::GameObject()
        : m_Entity(entt::null), m_ECS(nullptr) {
    }

    GameObject::GameObject(ECS::Entity entity, ECS* ecs)
        : m_Entity(entity), m_ECS(ecs) {
        ITR_CORE_ASSERT(ecs, "ECS pointer cannot be null!");
    }

    std::string GameObject::GetName() const {
        if (!IsValid()) return "Invalid GameObject";

        if (HasComponent<TagComponent>()) {
            return GetComponent<TagComponent>().Tag;
        }
        return "GameObject";
    }

    void GameObject::SetName(const std::string& name) {
        if (!IsValid()) return;

        if (HasComponent<TagComponent>()) {
            GetComponent<TagComponent>().Tag = name;
        }
        else {
            AddComponent<TagComponent>(name);
        }
    }

    Transform& GameObject::GetTransform() {
        ITR_CORE_ASSERT(IsValid(), "Cannot get transform from invalid GameObject!");
        ITR_CORE_ASSERT(HasComponent<TransformComponent>(), "GameObject must have TransformComponent!");
        return GetComponent<TransformComponent>().transform;
    }

    const Transform& GameObject::GetTransform() const {
        ITR_CORE_ASSERT(IsValid(), "Cannot get transform from invalid GameObject!");
        ITR_CORE_ASSERT(HasComponent<TransformComponent>(), "GameObject must have TransformComponent!");
        return GetComponent<TransformComponent>().transform;
    }

    void GameObject::SetActive(bool active) {
        if (!IsValid()) return;

        if (active) {
            if (!HasComponent<ActiveComponent>()) {
                AddComponent<ActiveComponent>();
            }
            else {
                GetComponent<ActiveComponent>().active = true;
            }
        }
        else {
            if (HasComponent<ActiveComponent>()) {
                GetComponent<ActiveComponent>().active = false;
            }
            // 如果不包含ActiveComponent，默认为激活状态
        }
    }

    bool GameObject::IsActive() const {
        if (!IsValid()) return false;

        // 如果没有ActiveComponent，默认为激活状态
        if (!HasComponent<ActiveComponent>()) {
            return true;
        }
        return GetComponent<ActiveComponent>().active;
    }

    bool GameObject::IsValid() const {
        return m_ECS && m_ECS->GetRegistry().valid(m_Entity);
    }

    // -------------------------------------------------------------------------
    // Static methods
    // -------------------------------------------------------------------------

    GameObject GameObject::Find(Scene* scene, const std::string& name) {
        if (!scene) return GameObject();

        auto& registry = scene->GetECS().GetRegistry();
        auto view = registry.view<TagComponent>();

        for (auto entity : view) {
            const auto& tag = view.get<TagComponent>(entity);
            if (tag.Tag == name) {
                return GameObject(entity, &scene->GetECS());
            }
        }

        return GameObject(); // 返回无效的GameObject
    }

    std::vector<GameObject> GameObject::FindGameObjectsWithTag(Scene* scene, const std::string& tag) {
        std::vector<GameObject> result;

        if (!scene) return result;

        auto& registry = scene->GetECS().GetRegistry();
        auto view = registry.view<TagComponent>();

        for (auto entity : view) {
            const auto& tagComp = view.get<TagComponent>(entity);
            if (tagComp.Tag == tag) {
                result.emplace_back(entity, &scene->GetECS());
            }
        }

        return result;
    }

    GameObject GameObject::FindWithTag(Scene* scene, const std::string& tag) {
        auto objects = FindGameObjectsWithTag(scene, tag);
        return objects.empty() ? GameObject() : objects[0];
    }

} // namespace Intro