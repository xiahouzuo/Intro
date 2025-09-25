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

    void SceneManager::OnUpdate(float dt) {
        if (auto scene = GetActiveScene()) {
            if (scene->IsActive()) scene->OnUpdate(dt);
        }
    }

    size_t SceneManager::FindIndexByName(const std::string& name) const {
        for (size_t i = 0; i < m_Scenes.size(); ++i) {
            if (m_Scenes[i]->GetName() == name) return i;
        }
        return npos;
    }

} // namespace Intro
