#pragma once

#include "Intro/Core.h"
#include "Intro/ECS/GameObject.h"
#include <memory>

namespace Intro {

    class ITR_API IScript {
    public:
        virtual ~IScript() = default;

        // 生命周期方法
        virtual void OnCreate() {}        // 脚本创建时调用
        virtual void OnStart() {}         // 第一次更新前调用
        virtual void OnUpdate(float deltaTime) {}  // 每帧调用
        virtual void OnDestroy() {}       // 脚本销毁时调用

        // 获取关联的GameObject
        GameObject GetGameObject() const { return m_GameObject; }
        void SetGameObject(GameObject gameObject) { m_GameObject = gameObject; }

        // 启用/禁用脚本
        bool IsEnabled() const { return m_Enabled; }
        void SetEnabled(bool enabled) { m_Enabled = enabled; }

    protected:
        GameObject m_GameObject;
        bool m_Enabled = true;
    };



} // namespace Intro