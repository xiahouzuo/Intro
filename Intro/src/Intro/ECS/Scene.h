// Scene.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include "ECS.h"
#include "Intro/Core.h"
#include <glm/glm.hpp>

namespace Intro {

    // 前置声明
    class GameObjectManager;
    class GameObject;

    // Scene 表示一个独立的世界 / 场景（拥有自己的 ECS/registry 等）
    class ITR_API Scene {
    public:
        explicit Scene(std::string name = "Untitled Scene");
        virtual ~Scene();

        // 禁用拷贝与移动（entt::registry 在我们的 ECS 中不可拷贝/移动）
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        // 生命周期钩子
        virtual void OnLoad();
        virtual void OnUnload();
        virtual void OnUpdate(float dt, bool isPlaying);

        // 保持向后兼容
        virtual void OnUpdate(float dt);

        // 访问 ECS
        ECS& GetECS() { return m_ECS; }
        const ECS& GetECS() const { return m_ECS; }

        // 便捷 API 转发
        ECS::Entity CreateEntity() { return m_ECS.CreateEntity(); }
        void DestroyEntity(ECS::Entity e) { m_ECS.DestroyEntity(e); }

        // 名称
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // 激活/停用
        void SetActive(bool active) { m_Active = active; }
        bool IsActive() const { return m_Active; }

        GameObject Instantiate(const GameObject& original, const glm::vec3& position = glm::vec3(0.0f));
        void Destroy(GameObject& gameObject);
        std::vector<GameObject> GetAllGameObjects();

        // GameObjectManager 访问（返回引用以兼容现有代码）
        GameObjectManager& GetGameObjectManager();
        const GameObjectManager& GetGameObjectManager() const;

        // GameObject API
        GameObject CreateGameObject(const std::string& name = "GameObject");
        void DestroyGameObject(GameObject& gameObject);

        // 修改：使用 Entity 而不是 GameObject 来存储主相机
        ECS::Entity GetMainCameraEntity() const;
        void SetMainCameraEntity(ECS::Entity cameraEntity);
        ECS::Entity FindMainCameraEntity();
        void CreateDefaultMainCamera();

        // 添加获取 GameObject 的方法
        GameObject GetMainCamera();
        void SetMainCamera(GameObject camera);
        GameObject FindMainCamera();

    protected:
        ECS m_ECS;

    private:
        std::string m_Name;
        bool m_Active;

        // 使用 unique_ptr 以避免头文件循环依赖与初始化问题
        std::unique_ptr<GameObjectManager> m_GameObjectManager;

        // 修改：使用 Entity 而不是 GameObject 来避免头文件依赖
        ECS::Entity m_MainCameraEntity{ entt::null };
    };

} // namespace Intro