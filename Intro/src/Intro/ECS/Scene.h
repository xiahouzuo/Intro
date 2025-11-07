#pragma once

#include <string>
#include <memory>
#include <vector>
#include "ECS.h"
#include "Intro/Core.h"
#include "GameObjectManager.h"
#include "Intro/Physics/PhysicsSystem.h"
#include <glm/glm.hpp>

namespace Intro {

    // 前置声明以避免头文件循环包含
    class GameObjectManager;

    // Scene 表示一个独立的世界 / 场景（拥有自己的 ECS/registry 等）
    class ITR_API Scene {
    public:
        explicit Scene(std::string name = "Untitled Scene")
            : m_Name(std::move(name))
            , m_Active(true)
        {
            // 在构造体内创建 GameObjectManager 实例，传入 this
            // （延后包含 GameObjectManager.h 可以放在 .cpp 中，如果需要）
            m_GameObjectManager = std::make_unique<GameObjectManager>(this);
        }

        virtual ~Scene() = default;

        // 禁用拷贝与移动（entt::registry 在我们的 ECS 中不可拷贝/移动）
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        // 生命周期钩子
        virtual void OnLoad();
        virtual void OnUnload() {}
        virtual void OnUpdate(float dt, bool isPlaying) {
            if (isPlaying) {
                // 只在运行模式下更新物理
                PhysicsSystem::OnUpdate(dt, m_ECS, isPlaying);
            }
            // 其他系统（如动画、音频）也可以根据运行状态决定是否更新
        }

        // 保持向后兼容
        virtual void OnUpdate(float dt) {
            OnUpdate(dt, true);
        }
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


        GameObject Instantiate(const GameObject& original, const glm::vec3& position = glm::vec3(0.0f)) {
            auto newEntity = m_ECS.CreateEntity();
            // TODO: 复制组件（按需实现）
            return GameObject(newEntity, &m_ECS);
        }

        void Destroy(GameObject& gameObject) {
            if (gameObject.IsValid()) {
                m_ECS.DestroyEntity(gameObject.GetEntity());
                gameObject = GameObject(); // 设为无效
            }
        }

        std::vector<GameObject> GetAllGameObjects();

        // GameObjectManager 访问（返回引用以兼容现有代码）
        GameObjectManager& GetGameObjectManager() { return *m_GameObjectManager; }
        const GameObjectManager& GetGameObjectManager() const { return *m_GameObjectManager; }

        // GameObject API
        GameObject CreateGameObject(const std::string& name = "GameObject");
        void DestroyGameObject(GameObject& gameObject);

        GameObject GetMainCamera() const { return m_MainCamera; }
        void SetMainCamera(GameObject camera);
        GameObject FindMainCamera();

        void CreateDefaultMainCamera();
    protected:
        ECS m_ECS;

    private:
        std::string m_Name;
        bool m_Active;

        // 使用 unique_ptr 以避免头文件循环依赖与初始化问题
        std::unique_ptr<GameObjectManager> m_GameObjectManager;

        GameObject m_MainCamera;
    };

} // namespace Intro
