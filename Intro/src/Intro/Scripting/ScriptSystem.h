#pragma once
#include "Intro/Core.h"
#include "IScript.h"
#include "ScriptRegistry.h"
#include "Intro/ECS/Components.h"
#include <vector>
#include <memory>

namespace Intro {

    class Scene;

    class ITR_API ScriptSystem {
    public:
        ScriptSystem(Scene* scene);
        ~ScriptSystem() = default;

        // 禁用拷贝和移动
        ScriptSystem(const ScriptSystem&) = delete;
        ScriptSystem& operator=(const ScriptSystem&) = delete;
        ScriptSystem(ScriptSystem&&) = delete;
        ScriptSystem& operator=(ScriptSystem&&) = delete;

        // 系统更新
        void OnUpdate(float deltaTime);

        // 脚本管理
        void AddScript(GameObject gameObject, const std::string& scriptClassName);
        void RemoveScript(GameObject gameObject);
        bool HasScript(GameObject gameObject) const;

        // 重新加载所有脚本（用于热重载）
        void ReloadAllScripts();

        // 获取场景中所有使用脚本的GameObject
        std::vector<GameObject> GetAllScriptedGameObjects() const;

    private:
        // 初始化脚本实例
        void InitializeScript(GameObject gameObject, ScriptComponent& scriptComp);

        Scene* m_Scene = nullptr;
    };

} // namespace Intro