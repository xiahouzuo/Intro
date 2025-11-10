#include "itrpch.h"
#include "GameObjectManager.h"
#include "Intro/ECS/Scene.h"
#include "Intro/Log.h"

namespace Intro {

    // -------------------------------------------------------------------------
    // GameObjectManager implementation
    // -------------------------------------------------------------------------

    GameObjectManager::GameObjectManager(Scene* scene)
        : m_Scene(scene), m_NeedsRefresh(true)
    {
    }

    void GameObjectManager::SetScene(Scene* scene) {
        m_Scene = scene;
        m_NeedsRefresh = true;
    }

    GameObject GameObjectManager::CreateGameObject(const std::string& name) {
        if (!m_Scene) {
            ITR_ERROR("Cannot create GameObject: No scene set!");
            return GameObject();
        }

        GameObject go = m_Scene->CreateGameObject(name);

        // 刷新缓存
        m_NeedsRefresh = true;

        // 触发回调
        if (onGameObjectCreated) {
            onGameObjectCreated(go);
        }

        ITR_INFO("Created GameObject: {}", name);
        return go;
    }

    GameObject GameObjectManager::CreateGameObjectWithParent(const std::string& name, GameObject parent) {
        if (!parent.IsValid()) {
            return CreateGameObject(name);
        }

        GameObject child = CreateGameObject(name);
        SetParent(child, parent);
        return child;
    }

    void GameObjectManager::DestroyGameObject(GameObject& gameObject) {
        if (!gameObject.IsValid() || !m_Scene) return;

        // 先销毁所有子对象
        auto children = GetChildren(gameObject);
        for (auto& child : children) {
            DestroyGameObject(child);
        }

        // 触发销毁前回调
        if (onGameObjectDestroyed) {
            onGameObjectDestroyed(gameObject);
        }

        // 从父对象中移除
        auto parent = GetParent(gameObject);
        if (parent.IsValid()) {
            // 这里需要实现从父对象的HierarchyComponent中移除子对象
        }

        m_Scene->Destroy(gameObject);
        m_NeedsRefresh = true;

        ITR_INFO("Destroyed GameObject: {}", gameObject.GetName());
    }

    void GameObjectManager::DestroyGameObjectImmediate(GameObject& gameObject) {
        DestroyGameObject(gameObject);
    }

    GameObject GameObjectManager::Find(const std::string& name) const {
        if (!m_Scene) return GameObject();
        return GameObject::Find(m_Scene, name);
    }

    GameObject GameObjectManager::FindWithTag(const std::string& tag) const {
        if (!m_Scene) return GameObject();
        return GameObject::FindWithTag(m_Scene, tag);
    }

    std::vector<GameObject> GameObjectManager::FindAllWithTag(const std::string& tag) const {
        if (!m_Scene) return {};
        return GameObject::FindGameObjectsWithTag(m_Scene, tag);
    }

    std::vector<GameObject> GameObjectManager::GetAllGameObjects() const {
        if (!m_Scene) return {};

        // 如果缓存需要刷新，重新获取所有GameObject
        if (m_NeedsRefresh) {
            const_cast<GameObjectManager*>(this)->RefreshGameObjectList();
        }

        return m_CachedGameObjects;
    }

    void GameObjectManager::DestroyAllGameObjects() {
        if (!m_Scene) return;

        auto allObjects = GetAllGameObjects();
        for (auto& go : allObjects) {
            DestroyGameObject(go);
        }

        m_CachedGameObjects.clear();
        m_NeedsRefresh = false;
    }

    void GameObjectManager::SetParent(GameObject child, GameObject parent) {
        if (!child.IsValid() || !parent.IsValid()) return;
        if (child == parent) return; // 不能设置自己为父对象

        // 检查循环引用
        if (IsInHierarchy(child, parent)) {
            ITR_ERROR("Cannot set parent: Circular hierarchy detected!");
            return;
        }

        // 从当前父对象中移除
        auto currentParent = GetParent(child);
        if (currentParent.IsValid()) {
            // 从当前父对象的children列表中移除
            // 这里需要实现HierarchyComponent的children管理
        }

        // 设置新的父对象
        // 这里需要实现HierarchyComponent的parent设置

        ITR_INFO("Set parent: {} -> {}", child.GetName(), parent.GetName());
    }

    GameObject GameObjectManager::GetParent(GameObject gameObject) const {
        if (!gameObject.IsValid()) return GameObject();

        // 检查是否有HierarchyComponent并返回parent
        // 这里需要实现HierarchyComponent的parent获取
        return GameObject();
    }

    std::vector<GameObject> GameObjectManager::GetChildren(GameObject parent) const {
        if (!parent.IsValid()) return {};

        // 从HierarchyComponent中获取children列表
        // 这里需要实现HierarchyComponent的children获取
        return {};
    }

    void GameObjectManager::SetActive(GameObject gameObject, bool active) {
        if (!gameObject.IsValid()) return;

        bool wasActive = gameObject.IsActive();
        gameObject.SetActive(active);

        if (wasActive != active && onGameObjectActiveChanged) {
            onGameObjectActiveChanged(gameObject, active);
        }

        ITR_INFO("Set {} active: {}", gameObject.GetName(), active);
    }

    void GameObjectManager::SetActiveRecursive(GameObject gameObject, bool active) {
        if (!gameObject.IsValid()) return;

        SetActive(gameObject, active);

        // 递归设置所有子对象
        auto children = GetChildren(gameObject);
        for (auto& child : children) {
            SetActiveRecursive(child, active);
        }
    }

    size_t GameObjectManager::GetGameObjectCount() const {
        if (!m_Scene) return 0;
        return GetAllGameObjects().size();
    }

    size_t GameObjectManager::GetActiveGameObjectCount() const {
        if (!m_Scene) return 0;

        auto allObjects = GetAllGameObjects();
        size_t count = 0;
        for (const auto& go : allObjects) {
            if (go.IsActive()) {
                count++;
            }
        }
        return count;
    }

    void GameObjectManager::RefreshGameObjectList() {
        if (!m_Scene) {
            m_CachedGameObjects.clear();
            return;
        }

        m_CachedGameObjects = m_Scene->GetAllGameObjects();
        m_NeedsRefresh = false;
    }

    bool GameObjectManager::IsInHierarchy(GameObject parent, GameObject child) const {
        if (!parent.IsValid() || !child.IsValid()) return false;
        if (parent == child) return true;

        auto children = GetChildren(parent);
        for (const auto& c : children) {
            if (IsInHierarchy(c, child)) {
                return true;
            }
        }

        return false;
    }

} // namespace Intro