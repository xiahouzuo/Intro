// RendererLayer.cpp
#include "itrpch.h"
#include "RendererLayer.h"
#include "imgui.h"
#include "Intro/Application.h"
#include "Intro/ECS/SceneManager.h"
#include "RenderCommand.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Intro {

    RendererLayer::RendererLayer(const Window& window)
        : Layer("Renderer Layer"), m_Window(window), m_Camera(window)
    {
        // ���ػ�����ɫ����ʵ����Ŀ��·��Ӧ��������Դϵͳ��
        m_Shader = std::make_unique<Shader>(
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.vert",
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.frag"
        );

        // ��ʼ���ӿڳߴ�Ϊ���ڳߴ�
        m_ViewportWidth = window.GetWidth();
        m_ViewportHeight = window.GetHeight();
    }

    RendererLayer::~RendererLayer()
    {
        // �ͷ���Դ
        m_Shader.reset();
        DestroyFramebuffer();
    }

    /**
     * ��ʼ����Ⱦ��Դ
     * ע�⣺����OpenGL�����Ĵ��������
     */
    void RendererLayer::OnAttach()
    {
        // ������Ȳ��Ժͱ����޳�
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // ������ʼFBO
        CreateFramebuffer(m_ViewportWidth, m_ViewportHeight);
    }

    /**
     * ����������Ⱦ֡���壨FBO��
     * ���ܣ���������Ⱦ��������ImGui��ʾ
     * @param width ֡������
     * @param height ֡����߶�
     */
    void RendererLayer::CreateFramebuffer(uint32_t width, uint32_t height)
    {
        // �����پɵ�FBO��Դ
        DestroyFramebuffer();

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // 1. ������ɫ�����洢��Ⱦ�����
        glGenTextures(1, &m_ColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
        // ���������ڴ棨��ʼΪ�գ�
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // ����������˺ͻ��Ʒ�ʽ
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // 2. �������ģ�建�壨RBO����Ч�洢��Ⱥ�ģ�����ݣ�
        glGenRenderbuffers(1, &m_RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // 3. ����������FBO
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // ������ɫ��������ģ�建��
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

        // ���FBO�����ԣ������ã�
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            ITR_ERROR("RendererLayer: Framebuffer is not complete! Error code: 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }

        // ���FBO������Ӱ�������Ⱦ
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
     * ����FBO�����Դ
     */
    void RendererLayer::DestroyFramebuffer()
    {
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        if (m_ColorTexture)
        {
            glDeleteTextures(1, &m_ColorTexture);
            m_ColorTexture = 0;
        }
        if (m_RBO)
        {
            glDeleteRenderbuffers(1, &m_RBO);
            m_RBO = 0;
        }
    }

    /**
     * �����ӿڳߴ磨����FBO�ؽ���
     * @param width �¿��
     * @param height �¸߶�
     */
    void RendererLayer::ResizeViewport(uint32_t width, uint32_t height)
    {
        // ������Ч�ߴ���ظ�����
        if (width == 0 || height == 0 || (width == m_ViewportWidth && height == m_ViewportHeight))
            return;

        // �ؽ�FBO����������ݺ��
        CreateFramebuffer(width, height);
        m_Camera.AspectRatio = (float)width / (float)height;
    }

    /**
     * ����Ⱦ״̬��׼����Ⱦ��FBO��
     * ���ܣ����浱ǰ��Ⱦ״̬���л���FBO�Ͷ�Ӧ�ӿ�
     */
    void RendererLayer::BindRenderState()
    {
        // ���浱ǰ��Ⱦ״̬�����ں����ָ���
        glGetIntegerv(GL_VIEWPORT, m_SavedState.viewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_SavedState.framebuffer);
        m_SavedState.depthTest = glIsEnabled(GL_DEPTH_TEST);
        m_SavedState.cullFace = glIsEnabled(GL_CULL_FACE);

        // ������Ⱦ��FBO��״̬
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, (GLsizei)m_ViewportWidth, (GLsizei)m_ViewportHeight);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    /**
     * �����Ⱦ״̬���ָ���֮ǰ��״̬��
     * ���ܣ�ȷ��������Ⱦ����ImGui������FBO״̬Ӱ��
     */
    void RendererLayer::UnbindRenderState()
    {
        // �ָ��󶨵�֡����
        glBindFramebuffer(GL_FRAMEBUFFER, m_SavedState.framebuffer);
        // �ָ��ӿ�
        glViewport(
            m_SavedState.viewport[0], m_SavedState.viewport[1],
            m_SavedState.viewport[2], m_SavedState.viewport[3]
        );
        // �ָ���Ȳ��Ժͱ����޳�״̬
        if (!m_SavedState.depthTest)
            glDisable(GL_DEPTH_TEST);
        if (!m_SavedState.cullFace)
            glDisable(GL_CULL_FACE);
    }

    /**
     * ÿ֡��Ⱦ����
     * ���̣�������� -> ��ȡ����Ⱦ���� -> ��Ⱦ��FBO -> �ָ�״̬
     */
    void RendererLayer::OnUpdate(float deltaTime)
    {
        // ����������������롢������ͼ����
        m_Camera.OnUpdate(deltaTime);

        // ��ȡ��ǰ�����
        auto& sceneMgr = Application::GetSceneManager();
        auto* activeScene = sceneMgr.GetActiveScene();
        if (!activeScene)
        {
            ITR_ERROR("No active Scene");
            return;
        }
        auto& ecs = activeScene->GetECS();

        // ��ECS��ȡ����Ⱦ���ݣ���������ͱ任��
        m_RenderableData.clear();
        m_RenderableData = RenderSystem::GetRenderables(ecs);

        // ��FBO��Ч����Ⱦ��FBO
        if (m_FBO)
        {
            BindRenderState();

            // �����ɫ����Ȼ���
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ����ɫ���������������
            m_Shader->Bind();
            glm::mat4 view = m_Camera.GetViewMat();
            glm::mat4 proj = m_Camera.GetProjectionMat();
            m_Shader->SetUniformMat4("view", view);
            m_Shader->SetUniformMat4("projection", proj);

            // ��������Ⱦ���п���Ⱦ����
            for (const auto& r : m_RenderableData)
            {
                if (!r.mesh) continue; // ������Ч����

                // ����ģ�;���ÿ������ı任��
                glm::mat4 model = r.transform.GetModelMatrix();
                m_Shader->SetUniformMat4("model", model);
                // ��������
                RenderCommand::Draw(*r.mesh, *m_Shader);
            }

            // �����ɫ��
            m_Shader->UnBind();
            // �ָ���Ⱦ״̬
            UnbindRenderState();
        }
    }

    /**
     * �¼������������ڳߴ�仯��
     */
    void RendererLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        std::cout << "rendererlayer event" << std::endl;

        // �󶨴��ڳߴ�仯�¼�������
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(RendererLayer::OnWindowResized));
    }

    /**
     * ���ڳߴ�仯�¼�����
     * ���ܣ���������ݺ�ȣ����޸�FBO��FBO�ߴ���ImGui�ӿڿ��ƣ�
     */
    bool RendererLayer::OnWindowResized(WindowResizeEvent& e)
    {
        m_Camera.AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
        return false; // �������¼�����ԭ��ϵͳ�߼�����
    }
}