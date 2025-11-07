#pragma once

#include "Intro/Core.h"
#include "GameObject.h"
#include "Scene.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace Intro {

    class ITR_API GameObjectManager {
    public:
        GameObjectManager(Scene* scene = nullptr);
        ~GameObjectManager() = default;

        // 禁用拷贝和移动
        GameObjectManager(const GameObjectManager&) = delete;
        GameObjectManager& operator=(const GameObjectManager&) = delete;
        GameObjectManager(GameObjectManager&&) = delete;
        GameObjectManager& operator=(GameObjectManager&&) = delete;

        // 设置关联的场景
        void SetScene(Scene* scene);
        Scene* GetScene() const { return m_Scene; }

        // GameObject 创建和管理
        GameObject CreateGameObject(const std::string& name = "GameObject");
        GameObject CreateGameObjectWithParent(const std::string& name, GameObject parent);

        void DestroyGameObject(GameObject& gameObject);
        void DestroyGameObjectImmediate(GameObject& gameObject);

        // 查找方法
        GameObject Find(const std::string& name) const;
        GameObject FindWithTag(const std::string& tag) const;
        std::vector<GameObject> FindAllWithTag(const std::string& tag) const;

        template<typename T>
        std::vector<GameObject> FindAllWithComponent() const;

        // 批量操作
        std::vector<GameObject> GetAllGameObjects() const;
        void DestroyAllGameObjects();

        // 层级关系
        void SetParent(GameObject child, GameObject parent);
        GameObject GetParent(GameObject gameObject) const;
        std::vector<GameObject> GetChildren(GameObject parent) const;

        // 激活状态管理
        void SetActive(GameObject gameObject, bool active);
        void SetActiveRecursive(GameObject gameObject, bool active);

        // 统计信息
        size_t GetGameObjectCount() const;
        size_t GetActiveGameObjectCount() const;

        // 回调函数
        std::function<void(GameObject)> onGameObjectCreated;
        std::function<void(GameObject)> onGameObjectDestroyed;
        std::function<void(GameObject, bool)> onGameObjectActiveChanged;

        void MarkNeedsRefresh() { m_NeedsRefresh = true; }

    private:
        // 内部辅助方法
        void RefreshGameObjectList();
        bool IsInHierarchy(GameObject parent, GameObject child) const;

        // 层级关系组件
        struct HierarchyComponent {
            GameObject parent{};
            std::vector<GameObject> children;
        };

    private:
        Scene* m_Scene = nullptr;
        std::vector<GameObject> m_CachedGameObjects;
        bool m_NeedsRefresh = true;
    };

} // namespace Intro

// 模板方法实现
namespace Intro {

    template<typename T>
    std::vector<GameObject> GameObjectManager::FindAllWithComponent() const {
        if (!m_Scene) return {};
        return m_Scene->FindGameObjectsWithComponent<T>();
    }

} // namespace Intro