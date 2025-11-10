// Scene.cpp
#include "itrpch.h"
#include "Scene.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Components.h"

namespace Intro {

    Scene::Scene(std::string name)
        : m_Name(std::move(name))
        , m_Active(true)
    {
        m_GameObjectManager = std::make_unique<GameObjectManager>(this);
    }

    Scene::~Scene() = default;

    void Scene::OnLoad()
    {
        if (m_MainCameraEntity == entt::null)
        {
            CreateDefaultMainCamera();
        }
    }

    void Scene::OnUnload() {}

    void Scene::OnUpdate(float dt, bool isPlaying) {
        // 实现...
    }

    void Scene::OnUpdate(float dt) {
        OnUpdate(dt, true);
    }

    GameObject Scene::CreateGameObject(const std::string& name) {
        auto entity = CreateEntity();
        m_ECS.AddComponent<TagComponent>(entity, name);
        m_ECS.AddComponent<TransformComponent>(entity);
        m_ECS.AddComponent<ActiveComponent>(entity);
        GameObject go(entity, &m_ECS);

        if (m_GameObjectManager) {
            m_GameObjectManager->MarkNeedsRefresh();
        }

        return go;
    }

    void Scene::DestroyGameObject(GameObject& gameObject) {
        if (gameObject.IsValid()) {
            // 如果销毁的是主相机，清除引用
            if (gameObject.GetEntity() == m_MainCameraEntity) {
                m_MainCameraEntity = entt::null;
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

    GameObjectManager& Scene::GetGameObjectManager() {
        return *m_GameObjectManager;
    }

    const GameObjectManager& Scene::GetGameObjectManager() const {
        return *m_GameObjectManager;
    }

    GameObject Scene::Instantiate(const GameObject& original, const glm::vec3& position) {
        auto newEntity = m_ECS.CreateEntity();
        // TODO: 复制组件
        return GameObject(newEntity, &m_ECS);
    }

    void Scene::Destroy(GameObject& gameObject) {
        if (gameObject.IsValid()) {
            m_ECS.DestroyEntity(gameObject.GetEntity());
            gameObject = GameObject();
        }
    }

    // 新的 Entity 方法
    ECS::Entity Scene::GetMainCameraEntity() const {
        return m_MainCameraEntity;
    }

    void Scene::SetMainCameraEntity(ECS::Entity cameraEntity) {
        if (cameraEntity != entt::null && m_ECS.HasComponent<CameraComponent>(cameraEntity)) {
            // 清除之前主相机的标志
            if (m_MainCameraEntity != entt::null && m_ECS.HasComponent<CameraComponent>(m_MainCameraEntity)) {
                m_ECS.GetComponent<CameraComponent>(m_MainCameraEntity).isMainCamera = false;
            }

            m_MainCameraEntity = cameraEntity;
            m_ECS.GetComponent<CameraComponent>(m_MainCameraEntity).isMainCamera = true;
        }
    }

    ECS::Entity Scene::FindMainCameraEntity() {
        auto view = m_ECS.GetRegistry().view<CameraComponent>();
        for (auto entity : view) {
            auto& cameraComp = view.get<CameraComponent>(entity);
            if (cameraComp.isMainCamera) {
                return entity;
            }
        }
        return entt::null;
    }

    // 兼容的 GameObject 方法
    GameObject Scene::GetMainCamera() {
        return GameObject(m_MainCameraEntity, &m_ECS);
    }

    void Scene::SetMainCamera(GameObject camera) {
        if (camera.IsValid()) {
            SetMainCameraEntity(camera.GetEntity());
        }
    }

    GameObject Scene::FindMainCamera() {
        auto entity = FindMainCameraEntity();
        return GameObject(entity, &m_ECS);
    }

    void Scene::CreateDefaultMainCamera() {
        GameObject mainCamera = CreateGameObject("Main Camera");

        // 设置初始位置
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

        SetMainCameraEntity(mainCamera.GetEntity());
    }
} // namespace Intro