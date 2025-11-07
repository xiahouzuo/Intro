#include "itrpch.h"
#include "SceneManager.h"

namespace Intro {

    SceneManager::SceneManager() = default;
    SceneManager::~SceneManager() = default;

    void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
        if (!scene) return;
        m_Scenes.push_back(std::move(scene));
    }

    void SceneManager::RemoveScene(size_t index) {
        if (index >= m_Scenes.size()) return;
        if (m_ActiveIndex == index) {
            m_Scenes[index]->OnUnload();
            if (onSceneUnloaded) onSceneUnloaded(*m_Scenes[index]);
            m_ActiveIndex = npos;
        }
        m_Scenes.erase(m_Scenes.begin() + index);
        if (m_ActiveIndex != npos && index < m_ActiveIndex) {
            --m_ActiveIndex;
        }
    }

    bool SceneManager::SetActiveSceneByIndex(size_t index) {
        if (index >= m_Scenes.size()) return false;
        if (m_ActiveIndex == index) return true;

        if (m_ActiveIndex != npos) {
            m_Scenes[m_ActiveIndex]->OnUnload();
            if (onSceneUnloaded) onSceneUnloaded(*m_Scenes[m_ActiveIndex]);
        }

        m_ActiveIndex = index;
        m_Scenes[m_ActiveIndex]->OnLoad();
        if (onSceneLoaded) onSceneLoaded(*m_Scenes[m_ActiveIndex]);
        return true;
    }

    bool SceneManager::SetActiveSceneByName(const std::string& name) {
        auto idx = FindIndexByName(name);
        if (idx == npos) return false;
        return SetActiveSceneByIndex(idx);
    }

    Scene* SceneManager::GetActiveScene() {
        if (m_ActiveIndex == npos) return nullptr;
        return m_Scenes[m_ActiveIndex].get();
    }

    const Scene* SceneManager::GetActiveScene() const {
        if (m_ActiveIndex == npos) return nullptr;
        return m_Scenes[m_ActiveIndex].get();
    }

    size_t SceneManager::GetSceneCount() const {
        return m_Scenes.size();
    }



    size_t SceneManager::FindIndexByName(const std::string& name) const {
        for (size_t i = 0; i < m_Scenes.size(); ++i) {
            if (m_Scenes[i]->GetName() == name) return i;
        }
        return npos;
    }

    void SceneManager::SaveSceneState()
    {
        m_SavedTransforms.clear();
        m_SavedRigidbodies.clear();

        auto* activeScene = GetActiveScene();
        if (!activeScene) return;

        auto& ecs = activeScene->GetECS();
        auto& registry = ecs.GetRegistry();

        // 保存所有实体的变换和刚体状态
        auto transformView = registry.view<TransformComponent>();
        for (auto entity : transformView) {
            m_SavedTransforms[entity] = transformView.get<TransformComponent>(entity);
        }

        auto rigidbodyView = registry.view<RigidbodyComponent>();
        for (auto entity : rigidbodyView) {
            m_SavedRigidbodies[entity] = rigidbodyView.get<RigidbodyComponent>(entity);
        }
    }

    void SceneManager::RestoreSceneState()
    {
        auto* activeScene = GetActiveScene();
        if (!activeScene) return;

        auto& ecs = activeScene->GetECS();
        auto& registry = ecs.GetRegistry();

        // 恢复变换状态
        for (auto& [entity, transform] : m_SavedTransforms) {
            if (registry.valid(entity) && registry.all_of<TransformComponent>(entity)) {
                registry.get<TransformComponent>(entity) = transform;
            }
        }

        // 恢复刚体状态（重置速度和力）
        for (auto& [entity, rigidbody] : m_SavedRigidbodies) {
            if (registry.valid(entity) && registry.all_of<RigidbodyComponent>(entity)) {
                auto& currentRb = registry.get<RigidbodyComponent>(entity);
                currentRb.velocity = glm::vec3(0.0f);
                currentRb.angularVelocity = glm::vec3(0.0f);
                currentRb.force = glm::vec3(0.0f);
                currentRb.torque = glm::vec3(0.0f);
                // 保持其他属性不变（质量、阻力等）
            }
        }
    }

    void SceneManager::OnUpdate(float dt)
    {
        for (auto& scene : m_Scenes)
        {
            if (scene->IsActive())
            {
                scene->OnUpdate(dt, m_IsPlaying);
            }
        }
    }

} // namespace Intro
