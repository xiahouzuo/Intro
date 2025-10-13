#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Renderer/Camears/FreeCamera.h"
#include "UniformBuffers.h"
#include "Shader.h"
#include "Mesh.h"
#include "Intro/Renderer/Model.h"
#include "ShapeGenerator.h"
#include "Intro/ECS/System.h"
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
        const FreeCamera& GetCamera() const { return m_Camera; }
        FreeCamera& GetCamera() { return m_Camera; }

    private:
        bool OnWindowResized(WindowResizeEvent& e);

        void CreateFramebuffer(uint32_t width, uint32_t height);
        void DestroyFramebuffer();
        void BindRenderState();
        void UnbindRenderState();

    private:
        const Window& m_Window;
        FreeCamera m_Camera;
        std::unique_ptr<Shader> m_Shader;

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
    };
}
