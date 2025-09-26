// 修改 Intro/Intro/src/Intro/Renderer/RendererLayer.h
#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Renderer/Camears/FreeCamera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Intro/Assert/Model.h"
#include "ShapeGenerator.h"  // 添加包含
#include "Intro/ECS/System.h"
#include <memory>
#include <vector>  // 添加包含

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
        ~RendererLayer();

        void OnAttach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(Event& event) override;

    private:
        bool OnWindowResized(WindowResizeEvent& e);

    private:
        const Window& m_Window;
        FreeCamera m_Camera;
        std::unique_ptr<Shader> m_Shader;

        std::vector<RenderSystem::RenderableData> m_RenderableData;
    };
}