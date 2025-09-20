// 修改 Intro/Intro/src/Intro/Renderer/RendererLayer.h
#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Intro/Assert/Model.h"
#include "ShapeGenerator.h"  // 添加包含
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

        void OnAttach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(Event& event) override;

        ShapeType GetCurrentType() const { return m_CurrentShape; }
        void SetCurrentType(ShapeType type);
    private:
        void InitShapes();  // 初始化所有形状
        void UpdateCurrentShape();  // 更新当前显示的形状
        bool OnWindowResized(WindowResizeEvent& e);

    private:
        const Window& m_Window;
        Camera m_Camera;
        std::unique_ptr<Shader> m_Shader;

        ShapeType m_CurrentShape = ShapeType::Null;
        std::vector<std::unique_ptr<Mesh>> m_Shapes;  // 存储所有形状
        std::unique_ptr<Mesh> m_TestMesh;  // 保留原测试三角形
        std::unique_ptr<Model> m_Model;
    };
}