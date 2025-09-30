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

    // 几何体类型枚举
    enum class ShapeType {
        Null,
        Cube,
        Sphere,
        Plane
    };

    /**
     * RendererLayer 负责场景渲染
     * 职责：管理相机、FBO离屏渲染、场景绘制、提供渲染结果纹理
     */
    class ITR_API RendererLayer : public Layer
    {
    public:
        // 构造函数：接收窗口引用（用于初始尺寸）
        RendererLayer(const Window& window);
        ~RendererLayer() override;

        // 层生命周期函数
        void OnAttach() override;   // 初始化渲染资源（FBO、着色器等）
        void OnUpdate(float deltaTime) override;  // 每帧渲染场景
        void OnEvent(Event& event) override;      // 处理事件（不修改原有系统）

        // 调整视口尺寸（供ImGuiLayer调用）
        void ResizeViewport(uint32_t width, uint32_t height);
        // 获取场景渲染结果纹理ID（供ImGuiLayer显示）
        GLuint GetSceneTextureID() const { return m_ColorTexture; }
        // 获取相机（只读/读写）
        const FreeCamera& GetCamera() const { return m_Camera; }
        FreeCamera& GetCamera() { return m_Camera; }

    private:
        // 窗口尺寸变化事件处理
        bool OnWindowResized(WindowResizeEvent& e);

        // FBO相关函数
        void CreateFramebuffer(uint32_t width, uint32_t height); // 创建离屏渲染帧缓冲
        void DestroyFramebuffer();                              // 销毁FBO资源
        void BindRenderState();                                 // 绑定渲染状态（FBO、视口等）
        void UnbindRenderState();                               // 解绑并恢复渲染状态

    private:
        const Window& m_Window;        // 窗口引用（获取尺寸等信息）
        FreeCamera m_Camera;           // 自由相机（控制视图）
        std::unique_ptr<Shader> m_Shader; // 基础着色器

        std::vector<RenderSystem::RenderableData> m_RenderableData; // 待渲染数据列表

        // FBO相关资源
        GLuint m_FBO = 0;              // 帧缓冲对象
        GLuint m_ColorTexture = 0;     // 颜色附着纹理（渲染结果）
        GLuint m_RBO = 0;              // 深度模板缓冲
        uint32_t m_ViewportWidth = 1280;  // 视口宽度
        uint32_t m_ViewportHeight = 720;  // 视口高度

        // 保存渲染状态（用于恢复）
        struct RenderState {
            GLint viewport[4];         // 视口参数
            GLint framebuffer;         // 当前绑定的帧缓冲
            GLboolean depthTest;       // 深度测试状态
            GLboolean cullFace;        // 背面剔除状态
        } m_SavedState;
    };
}