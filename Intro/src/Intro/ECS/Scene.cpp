#include "itrpch.h"
#include "Scene.h"
#include "GameObjectManager.h"

// 如果你在 Scene.h 已经包含了所需 component headers，则直接实现：
namespace Intro {

    void Scene::OnLoad()
    {
        if (!m_MainCamera.IsValid())
        {
            CreateDefaultMainCamera();
        }
    }

    GameObject Scene::CreateGameObject(const std::string& name) {
        auto entity = CreateEntity();
        m_ECS.AddComponent<TagComponent>(entity, name);
        m_ECS.AddComponent<TransformComponent>(entity);
        m_ECS.AddComponent<ActiveComponent>(entity);
        GameObject go(entity, &m_ECS);

        // 通知 manager 需要刷新（m_GameObjectManager 是 unique_ptr）
        if (m_GameObjectManager) {
            m_GameObjectManager->MarkNeedsRefresh();
        }

        return go;
    }

    void Scene::DestroyGameObject(GameObject& gameObject) {
        if (gameObject.IsValid()) {
            // 如果销毁的是主相机，清除引用
            if (gameObject == m_MainCamera) {
                m_MainCamera = GameObject();
            }
            m_ECS.DestroyEntity(gameObject.GetEntity());
            gameObject = GameObject();
        }
    }

    std::vector<GameObject> Scene::GetAllGameObjects() {
        std::vector<GameObject> result;
        auto view = m_ECS.GetRegistry().view<TagComponent>();
        for (auto entity : view) {
            result.emplace_back(entity, &m_ECS);
        }
        return result;
    }

    void Scene::SetMainCamera(GameObject camera) {
        if (camera.IsValid() && camera.HasComponent<CameraComponent>()) {
            // 清除之前主相机的标志
            if (m_MainCamera.IsValid() && m_MainCamera.HasComponent<CameraComponent>()) {
                m_MainCamera.GetComponent<CameraComponent>().isMainCamera = false;
            }

            m_MainCamera = camera;
            m_MainCamera.GetComponent<CameraComponent>().isMainCamera = true;
        }
    }

    GameObject Scene::FindMainCamera(){
        auto view = m_ECS.GetRegistry().view<CameraComponent>();
        for (auto entity : view) {
            auto& cameraComp = view.get<CameraComponent>(entity);
            if (cameraComp.isMainCamera) {
                return GameObject(entity, &m_ECS);
            }
        }
        return GameObject();
    }

    void Scene::CreateDefaultMainCamera() {
        GameObject mainCamera = CreateGameObject("Main Camera");

        // 设置初始位置 - 在编辑器相机后方，看向场景
        auto& transform = mainCamera.GetComponent<TransformComponent>();
        transform.transform.position = glm::vec3(0.0f, 2.0f, 5.0f);
        transform.transform.rotation = glm::quatLookAt(
            glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // 添加相机组件并设为主相机
        CameraComponent cameraComp;
        cameraComp.fov = 60.0f;
        cameraComp.nearClip = 0.1f;
        cameraComp.farClip = 1000.0f;
        cameraComp.isMainCamera = true;
        mainCamera.AddComponent<CameraComponent>(cameraComp);

        SetMainCamera(mainCamera);
    }
} // namespace Intro
