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
        m_Shader = std::make_unique<Shader>(
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/tempShader.vert",
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/tempShader.frag"
        );

        m_ViewportWidth = window.GetWidth();
        m_ViewportHeight = window.GetHeight();
    }

    RendererLayer::~RendererLayer()
    {
        m_Shader.reset();
        DestroyFramebuffer();
    }

    void RendererLayer::OnAttach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        CreateFramebuffer(m_ViewportWidth, m_ViewportHeight);

        m_CameraUBO = std::make_unique<CameraUBO>();
        m_LightsUBO = std::make_unique<LightsUBO>();

        // 创建默认材质（使用 shader 副本）
        auto shaderCopy = std::make_unique<Shader>(*m_Shader);
        std::shared_ptr<Shader> sharedShader = std::move(shaderCopy);

        m_DefaultMaterial = std::make_shared<Material>(sharedShader);
        m_DefaultMaterial->SetShininess(32.0f);

        // 可做一次默认 uniform 设置（非必需，但安全）
        m_Shader->Bind();
        m_Shader->SetUniformInt("material_diffuse", 0);
        m_Shader->SetUniformInt("material_specular", 1);
        m_Shader->SetUniformFloat("material_shininess", 32.0f);
        m_Shader->SetUniformVec3("u_AmbientColor", glm::vec3(0.2f));
    }

    void RendererLayer::CreateFramebuffer(uint32_t width, uint32_t height)
    {
        DestroyFramebuffer();

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        glGenTextures(1, &m_ColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &m_RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            ITR_ERROR("RendererLayer: Framebuffer is not complete! Error code: 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RendererLayer::DestroyFramebuffer()
    {
        if (m_FBO) {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        if (m_ColorTexture) {
            glDeleteTextures(1, &m_ColorTexture);
            m_ColorTexture = 0;
        }
        if (m_RBO) {
            glDeleteRenderbuffers(1, &m_RBO);
            m_RBO = 0;
        }
    }

    void RendererLayer::ResizeViewport(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || (width == m_ViewportWidth && height == m_ViewportHeight))
            return;

        CreateFramebuffer(width, height);
        m_Camera.SetAspectRatio((float)width / (float)height);
    }

    void RendererLayer::BindRenderState()
    {
        glGetIntegerv(GL_VIEWPORT, m_SavedState.viewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_SavedState.framebuffer);
        m_SavedState.depthTest = glIsEnabled(GL_DEPTH_TEST);
        m_SavedState.cullFace = glIsEnabled(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, (GLsizei)m_ViewportWidth, (GLsizei)m_ViewportHeight);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void RendererLayer::UnbindRenderState()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_SavedState.framebuffer);
        glViewport(m_SavedState.viewport[0], m_SavedState.viewport[1], m_SavedState.viewport[2], m_SavedState.viewport[3]);
        if (!m_SavedState.depthTest) glDisable(GL_DEPTH_TEST);
        if (!m_SavedState.cullFace) glDisable(GL_CULL_FACE);
    }

    void RendererLayer::OnUpdate(float deltaTime) {

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            ITR_ERROR("OpenGL error before rendering: 0x%x", error);
        }

        m_Camera.OnUpdate(deltaTime);
        m_Time += deltaTime;

        auto& sceneMgr = Application::GetSceneManager();
        auto* activeScene = sceneMgr.GetActiveScene();
        if (!activeScene) {
            ITR_ERROR("No active Scene");
            return;
        }
        auto& ecs = activeScene->GetECS();

        m_CameraUBO->OnUpdate(m_Camera, m_Time);
        m_LightsUBO->OnUpdate(ecs);

        // Ensure UBOs are bound to expected binding points (safety)
        m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
        m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

        // Bind a global/default shader for UBO/global uniforms if needed
        m_Shader->Bind();

        // Collect renderables
        m_RenderQueue.Clear();
        RenderSystem::CollectRenderables(ecs, m_RenderQueue, m_Camera.GetPosition());
        m_RenderQueue.Sort(m_Camera.GetPosition());

        if (m_FBO) {
            BindRenderState();

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);

            std::shared_ptr<Material> lastMaterial = nullptr;

            for (const auto& item : m_RenderQueue.opaque) {
                auto& material = item.material ? item.material : m_DefaultMaterial;

                if (material != lastMaterial) {
                    // Bind material (ensures shader and textures are bound)
                    material->Bind();

                    // Also ensure shader-level global uniforms are updated for this material's shader
                    auto matShaderPtr = material->GetShader();
                    if (matShaderPtr) {
                        matShaderPtr->Bind();
                        matShaderPtr->SetUniformFloat("material_shininess", material->GetShininess());
                        matShaderPtr->SetUniformVec3("u_AmbientColor", material->GetAmbient());
                    }

                    lastMaterial = material;
                }

                // Set transform
                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                // Draw
                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // Transparent pass
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            for (const auto& item : m_RenderQueue.transparent) {
                auto& material = item.material ? item.material : m_DefaultMaterial;
                material->Bind();

                m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
                m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // restore
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            lastMaterial = nullptr;

            m_Shader->UnBind();
            UnbindRenderState();

            error = glGetError();
            if (error != GL_NO_ERROR) {
                ITR_ERROR("OpenGL error after rendering: 0x%x", error);
            }
        }
    }

    void RendererLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(RendererLayer::OnWindowResized));
    }

    bool RendererLayer::OnWindowResized(WindowResizeEvent& e)
    {
        m_Camera.SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
        return false;
    }

} // namespace Intro
