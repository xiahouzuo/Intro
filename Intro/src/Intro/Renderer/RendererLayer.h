// RendererLayer.h
#pragma once
#include "Intro/Core.h"
#include "Intro/Layer.h"
#include "Intro/Window.h"
#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Renderer/Camears/FreeCamera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Intro/Renderer/Model.h"
#include "ShapeGenerator.h"
#include "Intro/ECS/System.h"
#include <memory>
#include <vector>

namespace Intro {

    // ����������ö��
    enum class ShapeType {
        Null,
        Cube,
        Sphere,
        Plane
    };

    /**
     * RendererLayer ���𳡾���Ⱦ
     * ְ�𣺹��������FBO������Ⱦ���������ơ��ṩ��Ⱦ�������
     */
    class ITR_API RendererLayer : public Layer
    {
    public:
        // ���캯�������մ������ã����ڳ�ʼ�ߴ磩
        RendererLayer(const Window& window);
        ~RendererLayer() override;

        // ���������ں���
        void OnAttach() override;   // ��ʼ����Ⱦ��Դ��FBO����ɫ���ȣ�
        void OnUpdate(float deltaTime) override;  // ÿ֡��Ⱦ����
        void OnEvent(Event& event) override;      // �����¼������޸�ԭ��ϵͳ��

        // �����ӿڳߴ磨��ImGuiLayer���ã�
        void ResizeViewport(uint32_t width, uint32_t height);
        // ��ȡ������Ⱦ�������ID����ImGuiLayer��ʾ��
        GLuint GetSceneTextureID() const { return m_ColorTexture; }
        // ��ȡ�����ֻ��/��д��
        const FreeCamera& GetCamera() const { return m_Camera; }
        FreeCamera& GetCamera() { return m_Camera; }

    private:
        // ���ڳߴ�仯�¼�����
        bool OnWindowResized(WindowResizeEvent& e);

        // FBO��غ���
        void CreateFramebuffer(uint32_t width, uint32_t height); // ����������Ⱦ֡����
        void DestroyFramebuffer();                              // ����FBO��Դ
        void BindRenderState();                                 // ����Ⱦ״̬��FBO���ӿڵȣ�
        void UnbindRenderState();                               // ��󲢻ָ���Ⱦ״̬

    private:
        const Window& m_Window;        // �������ã���ȡ�ߴ����Ϣ��
        FreeCamera m_Camera;           // ���������������ͼ��
        std::unique_ptr<Shader> m_Shader; // ������ɫ��

        std::vector<RenderSystem::RenderableData> m_RenderableData; // ����Ⱦ�����б�

        // FBO�����Դ
        GLuint m_FBO = 0;              // ֡�������
        GLuint m_ColorTexture = 0;     // ��ɫ����������Ⱦ�����
        GLuint m_RBO = 0;              // ���ģ�建��
        uint32_t m_ViewportWidth = 1280;  // �ӿڿ��
        uint32_t m_ViewportHeight = 720;  // �ӿڸ߶�

        // ������Ⱦ״̬�����ڻָ���
        struct RenderState {
            GLint viewport[4];         // �ӿڲ���
            GLint framebuffer;         // ��ǰ�󶨵�֡����
            GLboolean depthTest;       // ��Ȳ���״̬
            GLboolean cullFace;        // �����޳�״̬
        } m_SavedState;
    };
}