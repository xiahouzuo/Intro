// SceneManager.h
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "Scene.h"
#include "Intro/Core.h"

namespace Intro {

    class ITR_API SceneManager {
    public:
        SceneManager();
        ~SceneManager();

        // ���ÿ���/�ƶ�
        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        SceneManager(SceneManager&&) = delete;
        SceneManager& operator=(SceneManager&&) = delete;

        // ������ע��һ������
        template<typename T, typename... Args>
        T& CreateScene(std::string name, Args&&... args) {
            static_assert(std::is_base_of_v<Scene, T>, "T must derive from Scene");
            auto scene = std::make_unique<T>(std::forward<Args>(args)...);
            scene->SetName(name);
            T* ptr = scene.get();
            m_Scenes.push_back(std::move(scene));
            return *ptr;
        }
        
        //������Ƴ�scene
        void AddScene(std::unique_ptr<Scene> scene);
        void RemoveScene(size_t index);

        //ͨ����������Scene
        bool SetActiveSceneByIndex(size_t index);
        bool SetActiveSceneByName(const std::string& name);

        //��ȡ��ǰScene
        Scene* GetActiveScene();
        const Scene* GetActiveScene() const;

        size_t GetSceneCount() const;
        void OnUpdate(float dt);

        size_t FindIndexByName(const std::string& name) const;

        // �ص�
        std::function<void(const Scene&)> onSceneLoaded;
        std::function<void(const Scene&)> onSceneUnloaded;

        static constexpr size_t npos = static_cast<size_t>(-1);

    private:
        std::vector<std::unique_ptr<Scene>> m_Scenes;
        size_t m_ActiveIndex = npos;
    };

} // namespace Intro
