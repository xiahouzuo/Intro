#include "itrpch.h"
#include "ScriptSystem.h"
#include "Intro/ECS/Components.h"
#include "Intro/ECS/Scene.h"
#include "Intro/Log.h"

namespace Intro {

    ScriptSystem::ScriptSystem(Scene* scene) : m_Scene(scene) {
    }

    void ScriptSystem::OnUpdate(float deltaTime) {
        if (!m_Scene) return;

        auto view = m_Scene->GetECS().GetRegistry().view<ScriptComponent>();
        for (auto entity : view) {
            auto& scriptComp = view.get<ScriptComponent>(entity);

            if (!scriptComp.enabled || !scriptComp.HasScript()) continue;

            GameObject go(entity, &m_Scene->GetECS());
            auto& script = scriptComp.script;

            // 调用脚本更新
            if (script->IsEnabled()) {
                script->OnUpdate(deltaTime);
            }
        }
    }

    void ScriptSystem::AddScript(GameObject gameObject, const std::string& scriptClassName) {
        if (!gameObject.IsValid()) {
            ITR_WARN("Cannot add script to invalid GameObject");
            return;
        }

        // 创建脚本实例
        auto script = ScriptRegistry::Get().CreateScript(scriptClassName);
        if (!script) {
            ITR_ERROR("Failed to create script instance: {}", scriptClassName);
            return;
        }

        // 设置GameObject引用
        script->SetGameObject(gameObject);

        // 添加或更新ScriptComponent
        if (gameObject.HasComponent<ScriptComponent>()) {
            auto& existingComp = gameObject.GetComponent<ScriptComponent>();
            // 调用旧脚本的OnDestroy
            if (existingComp.script) {
                existingComp.script->OnDestroy();
            }
            existingComp.script = script;
            existingComp.scriptClassName = scriptClassName;
        }
        else {
            gameObject.AddComponent<ScriptComponent>(script, scriptClassName);
        }

        // 初始化新脚本
        InitializeScript(gameObject, gameObject.GetComponent<ScriptComponent>());

        ITR_INFO("Added script '{}' to GameObject '{}'", scriptClassName, gameObject.GetName());
    }

    void ScriptSystem::RemoveScript(GameObject gameObject) {
        if (!gameObject.IsValid() || !gameObject.HasComponent<ScriptComponent>()) {
            return;
        }

        auto& scriptComp = gameObject.GetComponent<ScriptComponent>();
        if (scriptComp.script) {
            scriptComp.script->OnDestroy();
        }

        gameObject.RemoveComponent<ScriptComponent>();
        ITR_INFO("Removed script from GameObject '{}'", gameObject.GetName());
    }

    bool ScriptSystem::HasScript(GameObject gameObject) const {
        return gameObject.IsValid() && gameObject.HasComponent<ScriptComponent>();
    }

    void ScriptSystem::ReloadAllScripts() {
        if (!m_Scene) return;

        ITR_INFO("Reloading all scripts...");

        auto view = m_Scene->GetECS().GetRegistry().view<ScriptComponent>();
        for (auto entity : view) {
            auto& scriptComp = view.get<ScriptComponent>(entity);
            GameObject go(entity, &m_Scene->GetECS());

            if (scriptComp.HasScript()) {
                // 保存类名和启用状态
                std::string className = scriptComp.scriptClassName;
                bool wasEnabled = scriptComp.enabled;

                // 销毁旧脚本
                scriptComp.script->OnDestroy();

                // 创建新实例
                auto newScript = ScriptRegistry::Get().CreateScript(className);
                if (newScript) {
                    newScript->SetGameObject(go);
                    scriptComp.script = newScript;
                    scriptComp.enabled = wasEnabled;

                    // 重新初始化
                    InitializeScript(go, scriptComp);
                }
            }
        }

        ITR_INFO("Script reload completed");
    }

    std::vector<GameObject> ScriptSystem::GetAllScriptedGameObjects() const {
        std::vector<GameObject> result;
        if (!m_Scene) return result;

        auto view = m_Scene->GetECS().GetRegistry().view<ScriptComponent>();
        for (auto entity : view) {
            result.emplace_back(entity, &m_Scene->GetECS());
        }
        return result;
    }

    void ScriptSystem::InitializeScript(GameObject gameObject, ScriptComponent& scriptComp) {
        if (!scriptComp.HasScript()) return;

        // 设置GameObject引用
        scriptComp.script->SetGameObject(gameObject);

        // 调用生命周期方法
        scriptComp.script->OnCreate();
        scriptComp.script->OnStart();
    }

} // namespace Intro