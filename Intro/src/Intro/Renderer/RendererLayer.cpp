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
        // 加载基础着色器（实际项目中路径应配置在资源系统）
        m_Shader = std::make_unique<Shader>(
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.vert",
            "E:/MyEngine/Intro/Intro/src/Intro/Assert/Shaders/BasicShader.frag"
        );

        // 初始化视口尺寸为窗口尺寸
        m_ViewportWidth = window.GetWidth();
        m_ViewportHeight = window.GetHeight();
    }

    RendererLayer::~RendererLayer()
    {
        // 释放资源
        m_Shader.reset();
        DestroyFramebuffer();
    }

    /**
     * 初始化渲染资源
     * 注意：需在OpenGL上下文创建后调用
     */
    void RendererLayer::OnAttach()
    {
        // 启用深度测试和背面剔除
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // 创建初始FBO
        CreateFramebuffer(m_ViewportWidth, m_ViewportHeight);

        m_CameraUBO = std::make_unique<CameraUBO>();
        m_LightsUBO = std::make_unique<LightsUBO>();

        auto shaderCopy = std::make_unique<Shader>(*m_Shader); // 复制原 Shader 的内容

        // 2. 将副本的所有权转移给 shared_ptr
        std::shared_ptr<Shader> sharedShader = std::move(shaderCopy);

        // 3. 用 shared_ptr 构造 Material（原 m_Shader 仍有效）
        m_DefaultMaterial = std::make_shared<Material>(sharedShader);
        m_DefaultMaterial->SetShininess(32.0f);

        // 在RendererLayer::OnAttach中添加
        m_Shader->Bind();
        m_Shader->SetUniformInt("material_diffuse", 0);
        m_Shader->SetUniformInt("material_specular", 1);
        m_Shader->SetUniformFloat("material_shininess", 32.0f);
        m_Shader->SetUniformVec3("u_AmbientColor", glm::vec3(0.1f));
    }


    /**
     * 创建离屏渲染帧缓冲（FBO）
     * 功能：将场景渲染到纹理，供ImGui显示
     * @param width 帧缓冲宽度
     * @param height 帧缓冲高度
     */
    void RendererLayer::CreateFramebuffer(uint32_t width, uint32_t height)
    {
        // 先销毁旧的FBO资源
        DestroyFramebuffer();

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // 1. 创建颜色纹理（存储渲染结果）
        glGenTextures(1, &m_ColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
        // 分配纹理内存（初始为空）
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // 设置纹理过滤和环绕方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // 2. 创建深度模板缓冲（RBO，高效存储深度和模板数据）
        glGenRenderbuffers(1, &m_RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // 3. 创建并配置FBO
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // 附加颜色纹理和深度模板缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

        // 检查FBO完整性（调试用）
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            ITR_ERROR("RendererLayer: Framebuffer is not complete! Error code: 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }

        // 解绑FBO，避免影响后续渲染
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
     * 销毁FBO相关资源
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
     * 调整视口尺寸（触发FBO重建）
     * @param width 新宽度
     * @param height 新高度
     */
    void RendererLayer::ResizeViewport(uint32_t width, uint32_t height)
    {
        // 过滤无效尺寸和重复调用
        if (width == 0 || height == 0 || (width == m_ViewportWidth && height == m_ViewportHeight))
            return;

        // 重建FBO并更新相机纵横比
        CreateFramebuffer(width, height);
        m_Camera.SetAspectRatio((float)width / (float)height);
    }

    /**
     * 绑定渲染状态（准备渲染到FBO）
     * 功能：保存当前渲染状态，切换到FBO和对应视口
     */
    void RendererLayer::BindRenderState()
    {
        // 保存当前渲染状态（用于后续恢复）
        glGetIntegerv(GL_VIEWPORT, m_SavedState.viewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_SavedState.framebuffer);
        m_SavedState.depthTest = glIsEnabled(GL_DEPTH_TEST);
        m_SavedState.cullFace = glIsEnabled(GL_CULL_FACE);

        // 设置渲染到FBO的状态
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, (GLsizei)m_ViewportWidth, (GLsizei)m_ViewportHeight);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    /**
     * 解绑渲染状态（恢复到之前的状态）
     * 功能：确保后续渲染（如ImGui）不受FBO状态影响
     */
    void RendererLayer::UnbindRenderState()
    {
        // 恢复绑定的帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, m_SavedState.framebuffer);
        // 恢复视口
        glViewport(
            m_SavedState.viewport[0], m_SavedState.viewport[1],
            m_SavedState.viewport[2], m_SavedState.viewport[3]
        );
        // 恢复深度测试和背面剔除状态
        if (!m_SavedState.depthTest)
            glDisable(GL_DEPTH_TEST);
        if (!m_SavedState.cullFace)
            glDisable(GL_CULL_FACE);
    }

    /**
     * 每帧渲染场景
     * 流程：更新相机 -> 获取待渲染数据 -> 渲染到FBO -> 恢复状态
     */
    void RendererLayer::OnUpdate(float deltaTime) {
        // 1. 更新相机和时间
        m_Camera.OnUpdate(deltaTime);
        m_Time += deltaTime;

        // 2. 获取活动场景和 ECS
        auto& sceneMgr = Application::GetSceneManager();
        auto* activeScene = sceneMgr.GetActiveScene();
        if (!activeScene) {
            ITR_ERROR("No active Scene");
            return;
        }
        auto& ecs = activeScene->GetECS();

        // 3. 更新 UBO（相机和光源数据上传到 GPU）
        m_CameraUBO->OnUpdate(m_Camera, m_Time); // 相机 UBO：视图、投影、相机位置
        m_LightsUBO->OnUpdate(ecs);             // 光源 UBO：收集场景中所有光源

        m_Shader->Bind();
        // 重新绑定UBO（如果需要）
        m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
        m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

        // 4. 收集并排序渲染项（替代原来的 m_RenderableData）
        m_RenderQueue.Clear();
        // 收集所有可渲染物体（MeshComponent 和 ModelComponent）
        RenderSystem::CollectRenderables(ecs, m_RenderQueue, m_Camera.GetPosition()); // 假设相机有 Position 成员
        // 按相机位置排序（不透明：前到后；透明：后到前）
        m_RenderQueue.Sort(m_Camera.GetPosition());

        // 5. 若 FBO 有效，渲染到 FBO
        if (m_FBO) {
            BindRenderState(); // 你的 FBO 绑定逻辑

            // 清除缓冲
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 6. 渲染不透明物体（按材质分组，减少状态切换）
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND); // 不透明物体不需要混合
            std::shared_ptr<Material> lastMaterial = nullptr;

            for (const auto& item : m_RenderQueue.opaque) {
                // 使用默认材质（如果物体没有指定材质）
                auto& material = item.material ? item.material : m_DefaultMaterial;

                // 材质变化时才重新绑定（优化）
                if (material != lastMaterial) {
                    material->Bind(); // 绑定材质（包含 shader 和纹理）
                    lastMaterial = material;
                }

                // 设置模型矩阵（每个物体的世界变换）
                // 注意：这里直接使用 item.transform（已在 CollectRenderables 中计算好）
                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                // 绘制网格
                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // 7. 渲染透明物体（后到前排序，开启混合）
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 标准 alpha 混合
            glDepthMask(GL_FALSE); // 透明物体不写入深度缓冲（避免遮挡后续透明物体）

            for (const auto& item : m_RenderQueue.transparent) {
                auto& material = item.material ? item.material : m_DefaultMaterial;
                material->Bind();

                // 设置模型矩阵
                material->GetShader()->SetUniformMat4("u_Transform", item.transform);

                // 绘制网格
                if (item.mesh) {
                    RenderCommand::Draw(*item.mesh, *material->GetShader());
                }
            }

            // 8. 恢复渲染状态
            glDepthMask(GL_TRUE); // 恢复深度写入
            glDisable(GL_BLEND);
            lastMaterial = nullptr; // 重置材质绑定状态

            // 解绑着色器和 FBO
            m_Shader->UnBind(); // 或使用最后绑定的材质 shader 解绑
            UnbindRenderState(); // 你的 FBO 解绑逻辑
        }
    }
    /**
     * 事件处理（仅处理窗口尺寸变化）
     */
    void RendererLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        std::cout << "rendererlayer event" << std::endl;

        // 绑定窗口尺寸变化事件处理函数
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(RendererLayer::OnWindowResized));
    }

    /**
     * 窗口尺寸变化事件处理
     * 功能：更新相机纵横比（不修改FBO，FBO尺寸由ImGui视口控制）
     */
    bool RendererLayer::OnWindowResized(WindowResizeEvent& e)
    {
        m_Camera.SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
        return false; // 不拦截事件，按原有系统逻辑传播
    }
}