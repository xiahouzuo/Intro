// �޸� Intro/Intro/src/Intro/Renderer/RendererLayer.h
#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Intro/Assert/Model.h"
#include "ShapeGenerator.h"  // ��Ӱ���
#include <memory>
#include <vector>  // ��Ӱ���

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
        void InitShapes();  // ��ʼ��������״
        void UpdateCurrentShape();  // ���µ�ǰ��ʾ����״
        bool OnWindowResized(WindowResizeEvent& e);

    private:
        const Window& m_Window;
        Camera m_Camera;
        std::unique_ptr<Shader> m_Shader;

        ShapeType m_CurrentShape = ShapeType::Null;
        std::vector<std::unique_ptr<Mesh>> m_Shapes;  // �洢������״
        std::unique_ptr<Mesh> m_TestMesh;  // ����ԭ����������
        std::unique_ptr<Model> m_Model;
    };
}