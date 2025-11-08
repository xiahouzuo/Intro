#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Cameras/FreeCamera.h"
#include "Cameras/Frustum.h"
#include "UniformBuffers.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Skybox.h"
#include "ShapeGenerator.h"
#include "Intro/ECS/System.h"
#include "Intro/ECS/GameObject.h"
#include <memory>
#include <vector>

namespace Intro {

    enum class ShapeType {
        Null,
        Cube,
        Sphere,
        Plane
    };

    class ITR_API RendererLayer : public Layer
    {
    public:
        RendererLayer(const Window& window);
        ~RendererLayer() override;

        void OnAttach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(Event& event) override;

        void ResizeViewport(uint32_t width, uint32_t height);
        GLuint GetSceneTextureID() const { return m_ColorTexture; }

        void SetUseEditorCamera(bool useEditor);
        bool IsUsingEditorCamera() const { return m_UseEditorCamera; };
        Camera& GetActiveCamera();
        const Camera& GetActiveCamera() const;

        std::unique_ptr<Camera> CreateCameraForComponent(CameraComponent& comp, GameObject cameraObject);
        void SyncCameraWithTransform(Camera& camera, const Transform& transform);
        void SyncGameCameraFromScene();

        bool GetShowFrustum() { return m_ShowFrustum; }
        void SetShowFrustum(bool showFrustum) { m_ShowFrustum = showFrustum; }

        bool GetShowColliders() const { return m_ShowColliders; }
        void SetShowColliders(bool show) { m_ShowColliders = show; }
        void RenderColliderWireframes();

        void SetEnableSkybox(bool enable) { m_EnableSkybox = enable; }
        bool IsSkyboxEnabled() const { return m_EnableSkybox; }
        void ReloadSkybox(const std::vector<std::string>& facePaths = {});
        void RenderSkybox();

    private:
        bool OnWindowResized(WindowResizeEvent& e);

        void CreateFramebuffer(uint32_t width, uint32_t height);
        void DestroyFramebuffer();
        void BindRenderState();
        void UnbindRenderState();

        void RenderOpaqueObjects();
        void RenderTransparentObjects();
        void BindMaterial(const std::shared_ptr<Material>& material);
        void SetupShaderUniforms(const std::shared_ptr<Shader>& shader);


        Camera* GetMainCameraFromScene();

        void RenderFrustum(const Frustum& frustum, const glm::vec3& color = glm::vec3(1.0f, 0.5f, 0.0f));
    private:
        const Window& m_Window;

        std::unique_ptr<FreeCamera> m_GameCamera;
        FreeCamera m_EditorCamera;
        bool m_UseEditorCamera = true;

        std::shared_ptr<Shader> m_Shader;

        std::vector<RenderSystem::RenderableData> m_RenderableData;

        GLuint m_FBO = 0;
        GLuint m_ColorTexture = 0;
        GLuint m_RBO = 0;
        uint32_t m_ViewportWidth = 1280;
        uint32_t m_ViewportHeight = 720;

        struct RenderState {
            GLint viewport[4];
            GLint framebuffer;
            GLboolean depthTest;
            GLboolean cullFace;
        } m_SavedState;

        std::unique_ptr<CameraUBO> m_CameraUBO;
        std::unique_ptr<LightsUBO> m_LightsUBO;
        RenderQueue m_RenderQueue;
        float m_Time = 0.0f;

        std::shared_ptr<Material> m_DefaultMaterial;

        //视锥
        Frustum m_EditorFrustum;
        Frustum m_GameFrustum;
        bool m_ShowFrustum = true;

        //物理
        std::vector<glm::vec3> m_ColliderLines;
        GLuint m_ColliderVAO = 0;
        GLuint m_ColliderVBO = 0;
        bool m_ShowColliders = true; // 控制是否显示碰撞体线框

        //skybox
        std::unique_ptr<Skybox> m_Skybox;
        bool m_EnableSkybox = true;
    };
}
