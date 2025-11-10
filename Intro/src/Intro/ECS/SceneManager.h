// SceneManager.h
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "Scene.h"
#include "Intro/Core.h"
#include "Intro/Log.h"

namespace Intro {

    class ITR_API SceneManager {
    public:
        SceneManager();
        ~SceneManager();

        // 禁用拷贝/移动
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        SceneManager(SceneManager&&) = delete;
        SceneManager& operator=(SceneManager&&) = delete;

        // 创建并注册一个场景
        template<typename T, typename... Args>
        T& CreateScene(std::string name, Args&&... args) {
            static_assert(std::is_base_of_v<Scene, T>, "T must derive from Scene");
            auto scene = std::make_unique<T>(std::forward<Args>(args)...);
            scene->SetName(name);
            T* ptr = scene.get();
            m_Scenes.push_back(std::move(scene));
            return *ptr;
        }
        
        //添加与移除scene
        void AddScene(std::unique_ptr<Scene> scene);
        void RemoveScene(size_t index);

        //通过索引启动Scene
        bool SetActiveSceneByIndex(size_t index);
        bool SetActiveSceneByName(const std::string& name);

        //获取当前Scene
        Scene* GetActiveScene();
        const Scene* GetActiveScene() const;

        size_t GetSceneCount() const;
        void OnUpdate(float dt);

        size_t FindIndexByName(const std::string& name) const;

        // 运行状态控制
        bool IsPlaying() const { return m_IsPlaying; }
        void StartRuntime() {
            if (!m_IsPlaying) {
                m_IsPlaying = true;
                // 保存场景状态
                SaveSceneState();
                ITR_INFO("Entering Play Mode");
            }
        }

        void StopRuntime() {
            if (m_IsPlaying) {
                m_IsPlaying = false;
                // 恢复场景状态
                RestoreSceneState();
                ITR_INFO("Exiting Play Mode");
            }
        }

        void ToggleRuntime() {
            if (m_IsPlaying) StopRuntime();
            else StartRuntime();
        }


        // 回调
        std::function<void(const Scene&)> onSceneLoaded;
        std::function<void(const Scene&)> onSceneUnloaded;

        static constexpr size_t npos = static_cast<size_t>(-1);

    private:
        // 场景状态保存
        void SaveSceneState();
        void RestoreSceneState();

    private:
        std::vector<std::unique_ptr<Scene>> m_Scenes;
        size_t m_ActiveIndex = npos;

        bool m_IsPlaying = false;


        std::unordered_map<entt::entity, TransformComponent> m_SavedTransforms;
        std::unordered_map<entt::entity, RigidbodyComponent> m_SavedRigidbodies;
    };

} // namespace Intro
