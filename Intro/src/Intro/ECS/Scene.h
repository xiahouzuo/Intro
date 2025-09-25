#pragma once

#include <string>
#include <memory>
#include "ECS.h"
#include "Intro/Core.h"

namespace Intro {

    // Scene 表示一个独立的世界 / 场景（拥有自己的 ECS/registry 等）
    class ITR_API Scene {
    public:
        explicit Scene(std::string name = "Untitled Scene")
            : m_Name(std::move(name)) {
        }

        virtual ~Scene() = default;

        // 禁用拷贝与移动（entt::registry 在我们的 ECS 中不可拷贝/移动）
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        // 生命周期钩子：子类可以覆盖这些方法做资源加载/卸载/每帧逻辑
        // 在引擎中，通常 Application/SceneManager 负责调用这些
        virtual void OnLoad() {}
        virtual void OnUnload() {}
        virtual void OnUpdate(float dt) {}

        // 访问 ECS（非 const），系统通常需要写操作或 view() 调用
        ECS& GetECS() { return m_ECS; }
        const ECS& GetECS() const { return m_ECS; }

        // 便捷 API 转发（可选）
        Intro::ECS::Entity CreateEntity() { return m_ECS.CreateEntity(); }
        void DestroyEntity(Intro::ECS::Entity e) { m_ECS.DestroyEntity(e); }

        // 名称
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // 激活/停用：某些场景可能暂时不更新但保留数据
        void SetActive(bool active) { m_Active = active; }
        bool IsActive() const { return m_Active; }

    protected:
        // 子类可以访问 m_ECS 来创建/注册系统，或者重载 OnLoad 做更复杂的初始化
        ECS m_ECS;

    private:
        std::string m_Name;
        bool m_Active = true;
    };

}