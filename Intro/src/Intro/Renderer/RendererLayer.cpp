// RendererLayer.cpp
#include "itrpch.h"
#include "RendererLayer.h"
#include "imgui.h"
#include "Intro/Application.h"
#include "Intro/ECS/SceneManager.h"
#include "RenderCommand.h"
#include "UBO.h"
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

        m_CameraUBO = std::make_unique<CameraUBO>();
        m_LightsUBO = std::make_unique<LightsUBO>();

        auto shaderCopy = std::make_unique<Shader>(*m_Shader); // ����ԭ Shader ������

        // 2. ������������Ȩת�Ƹ� shared_ptr
        std::shared_ptr<Shader> sharedShader = std::move(shaderCopy);

        // 3. �� shared_ptr ���� Material��ԭ m_Shader ����Ч��
        m_DefaultMaterial = std::make_shared<Material>(sharedShader);
        m_DefaultMaterial->SetShininess(32.0f);

        // ��RendererLayer::OnAttach�����
        m_Shader->Bind();
        m_Shader->SetUniformInt("material_diffuse", 0);
        m_Shader->SetUniformInt("material_specular", 1);
        m_Shader->SetUniformFloat("material_shininess", 32.0f);
        m_Shader->SetUniformVec3("u_AmbientColor", glm::vec3(0.1f));
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
        m_Camera.SetAspectRatio((float)width / (float)height);
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
    void RendererLayer::OnUpdate(float deltaTime) {
        // 1. ���������ʱ��
        m_Camera.OnUpdate(deltaTime);
        m_Time += deltaTime;

        // 2. ��ȡ������� ECS
        auto& sceneMgr = Application::GetSceneManager();
        auto* activeScene = sceneMgr.GetActiveScene();
        if (!activeScene) {
            ITR_ERROR("No active Scene");
            return;
        }
        auto& ecs = activeScene->GetECS();

        // 3. ���� UBO������͹�Դ�����ϴ��� GPU��
        m_CameraUBO->OnUpdate(m_Camera, m_Time); // ��� UBO����ͼ��ͶӰ�����λ��
        m_LightsUBO->OnUpdate(ecs);             // ��Դ UBO���ռ����������й�Դ

        m_Shader->Bind();
        // ���°�UBO�������Ҫ��
        m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
        m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

        // 4. �ռ���������Ⱦ����ԭ���� m_RenderableData��
        m_RenderQueue.Clear();
        // �ռ����п���Ⱦ���壨MeshComponent �� ModelComponent��
        RenderSystem::CollectRenderables(ecs, m_RenderQueue, m_Camera.GetPosition()); // ��������� Position ��Ա
        // �����λ�����򣨲�͸����ǰ����͸������ǰ��
        m_RenderQueue.Sort(m_Camera.GetPosition());

        // 5. �� FBO ��Ч����Ⱦ�� FBO
        if (m_FBO) {
            BindRenderState(); // ��� FBO ���߼�

            // �������
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 6. ��Ⱦ��͸�����壨�����ʷ��飬����״̬�л���
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND); // ��͸�����岻��Ҫ���
            std::shared_ptr<Material> lastMaterial = nullptr;

            for (const auto& item : m_RenderQueue.opaque) {
                // ʹ��Ĭ�ϲ��ʣ��������û��ָ�����ʣ�
                auto& material = item.material ? item.material : m_DefaultMaterial;

                // ���ʱ仯ʱ�����°󶨣��Ż���
                if (material != lastMaterial) {
                    material->Bind(); // �󶨲��ʣ����� shader ������
                    lastMaterial = material;
                }

                // ����ģ�;���ÿ�����������任��
                // ע�⣺����ֱ��ʹ�� item.transform������ CollectRenderables �м���ã�
                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                // ��������
                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // 7. ��Ⱦ͸�����壨��ǰ���򣬿�����ϣ�
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ��׼ alpha ���
            glDepthMask(GL_FALSE); // ͸�����岻д����Ȼ��壨�����ڵ�����͸�����壩

            for (const auto& item : m_RenderQueue.transparent) {
                auto& material = item.material ? item.material : m_DefaultMaterial;
                material->Bind();

                // ����ģ�;���
                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                // ��������
                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // 8. �ָ���Ⱦ״̬
            glDepthMask(GL_TRUE); // �ָ����д��
            glDisable(GL_BLEND);
            lastMaterial = nullptr; // ���ò��ʰ�״̬

            // �����ɫ���� FBO
            m_Shader->UnBind(); // ��ʹ�����󶨵Ĳ��� shader ���
            UnbindRenderState(); // ��� FBO ����߼�
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
        m_Camera.SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
        return false; // �������¼�����ԭ��ϵͳ�߼�����
    }
}