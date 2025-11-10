#include "itrpch.h"
#include "Renderer.h"
#include "RendererLayer.h"
#include "imgui.h"
#include "Intro/Application.h"
#include "Intro/ECS/SceneManager.h"
#include "Intro/RecourceManager/ShaderLibrary.h"
#include "Intro/Physics/PhysicsSystem.h"
#include "RenderCommand.h"
#include "UBO.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Intro {

    RendererLayer::RendererLayer(const Window& window)
        : Layer("Renderer Layer"), m_Window(window), m_EditorCamera(window)
    {
        m_Shader = Application::GetShaderLibrary().Get("defaultShader");

        m_ViewportWidth = window.GetWidth();
        m_ViewportHeight = window.GetHeight();

        m_GameCamera = std::make_unique<FreeCamera>(window);
    }

    RendererLayer::~RendererLayer()
    {
        m_Shader.reset();
        DestroyFramebuffer();

        if (m_ColliderVAO) {
            glDeleteVertexArrays(1, &m_ColliderVAO);
            m_ColliderVAO = 0;
        }
        if (m_ColliderVBO) {
            glDeleteBuffers(1, &m_ColliderVBO);
            m_ColliderVBO = 0;
        }
    }

    void RendererLayer::OnAttach()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        Renderer::SetConfig({
            m_ViewportWidth,
            m_ViewportHeight,
            false,  // enableMSAA
            4,      // msaaSamples
            false,  // enableHDR
            true    // enableGammaCorrection
            });
        Renderer::Init();

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
        m_Shader->SetUniformVec3("u_AmbientColor", glm::vec3(0.03f, 0.03f, 0.03f));

        glGenVertexArrays(1, &m_ColliderVAO);
        glGenBuffers(1, &m_ColliderVBO);

        glBindVertexArray(m_ColliderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_ColliderVBO);

        // 初始分配一些空间
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 1000, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        try {
            std::vector<std::string> skyboxFaces = {
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/right.jpg",
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/left.jpg",
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/top.jpg",
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/bottom.jpg",
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/front.jpg",
                "E:/MyEngine/Intro/Intro/src/Intro/assets/skybox/back.jpg"
            };

            // 检查天空盒文件是否存在
            bool allFilesExist = true;
            for (const auto& face : skyboxFaces) {
                if (!std::filesystem::exists(face)) {
                    ITR_WARN("Skybox texture not found: {}", face);
                    allFilesExist = false;
                }
            }

            if (allFilesExist) {
                m_Skybox = std::make_unique<Skybox>(skyboxFaces);
                m_EnableSkybox = true;
                ITR_INFO("Skybox initialized successfully");
            }
            else {
                ITR_WARN("Some skybox textures missing, skybox disabled");
                m_EnableSkybox = false;
            }
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed to initialize skybox: {}", e.what());
            m_EnableSkybox = false;
        }
    }


    void RendererLayer::OnUpdate(float deltaTime) {
  
    // 检查 OpenGL 错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        ITR_ERROR("OpenGL error before rendering: 0x%x", error);
    }

    // 更新时间和相机
    m_Time += deltaTime;

    if (m_UseEditorCamera) {
        m_EditorCamera.OnUpdate(deltaTime);
    }
    else {
        SyncGameCameraFromScene();
    }

    // 获取活动场景
    auto& sceneMgr = Application::GetSceneManager();
    auto* activeScene = sceneMgr.GetActiveScene();
    if (!activeScene) {
        ITR_ERROR("No active Scene");
        return;
    }
    auto& ecs = activeScene->GetECS();


    Camera& activeCam = GetActiveCamera();
    glm::mat4 viewProjection = activeCam.GetProjectionMat() * activeCam.GetViewMat();
    // 编辑器相机的视锥体总是从当前活动相机（编辑器视图）更新
    m_EditorFrustum.UpdateFromMatrix(viewProjection);

    // 无论当前是否使用编辑器相机，都尝试从场景主相机更新游戏视锥体
    // 这样在编辑器模式下可以正确地可视化游戏相机的视锥体
    Camera* sceneCamera = GetMainCameraFromScene();
    if (sceneCamera) {
        glm::mat4 gameViewProjection = sceneCamera->GetProjectionMat() * sceneCamera->GetViewMat();
        m_GameFrustum.UpdateFromMatrix(gameViewProjection);
    }

    // 更新碰撞体线框数据
    if (m_ShowColliders) {
        PhysicsSystem::DebugDrawColliders(ecs, m_ColliderLines);

        if (!m_ColliderLines.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, m_ColliderVBO);
            glBufferData(GL_ARRAY_BUFFER,
                m_ColliderLines.size() * sizeof(glm::vec3),
                m_ColliderLines.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    // ==================== 更新 UBO ====================
    m_CameraUBO->OnUpdate(activeCam, m_Time);
    m_LightsUBO->OnUpdate(ecs);

    // ==================== 收集渲染项 ====================
    m_RenderQueue.Clear();
    RenderSystem::CollectRenderables(ecs, m_RenderQueue, activeCam.GetPosition());
    m_RenderQueue.Sort(activeCam.GetPosition());


    BindRenderState();

    // ==================== 开始渲染帧 ====================
    Renderer::BeginFrame();

    // ==================== 渲染不透明物体 ====================
    RenderOpaqueObjects();

    // ==================== 渲染透明物体 ====================
    RenderTransparentObjects();

    if (m_EnableSkybox && m_Skybox) {
        RenderSkybox();
    }


    // ==================== 渲染碰撞体线框 ====================
    if (m_ShowColliders && !m_ColliderLines.empty()) {
        RenderColliderWireframes();
    }

    // ==================== 渲染视锥体 ====================
    if (m_ShowFrustum) {
        // 保存状态
        GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
        GLboolean prevBlend = glIsEnabled(GL_BLEND);

        // 设置视锥体渲染状态
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 编辑器模式下：显示游戏相机的视锥体（绿色）
        if (m_UseEditorCamera) {
            if (sceneCamera) {
                RenderFrustum(m_GameFrustum, glm::vec3(0.0f, 1.0f, 0.0f));
                                                         
            }
            else {
                ITR_WARN("No main scene camera available to render frustum");
            }
        }
        else {
            // 游戏模式下：不显示任何视锥体
            ITR_INFO("Game Mode - No frustum rendering");
        }

        // 恢复状态
        if (prevDepthTest) glEnable(GL_DEPTH_TEST);
        if (!prevBlend) glDisable(GL_BLEND);
    }

    // ==================== 结束渲染帧 ====================
    Renderer::EndFrame();
    UnbindRenderState();
    // 检查 OpenGL 错误
    error = glGetError();
    if (error != GL_NO_ERROR) {
        ITR_ERROR("OpenGL error after rendering: {0}", error);
    }
}


    // 新增碰撞体线框渲染函数
    void RendererLayer::RenderColliderWireframes() {
        auto lineShader = Application::GetShaderLibrary().Get("lineShader");
        if (!lineShader) {
            ITR_WARN("Line shader not found for collision debug rendering");
            return;
        }

        // 保存当前状态
        GLint prevPolygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);
        GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
        GLboolean prevBlend = glIsEnabled(GL_BLEND);

        // 设置线框渲染状态
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glLineWidth(2.0f);

        lineShader->Bind();
        lineShader->SetUniformMat4("u_ViewProjection",
            GetActiveCamera().GetProjectionMat() * GetActiveCamera().GetViewMat());

        // 设置线框颜色（红色表示碰撞体）
        lineShader->SetUniformVec3("u_Color", glm::vec3(1.0f, 0.0f, 0.0f));

        // 渲染线框
        glBindVertexArray(m_ColliderVAO);
        glDrawArrays(GL_LINES, 0, (GLsizei)m_ColliderLines.size());
        glBindVertexArray(0);

        lineShader->UnBind();

        // 恢复状态
        glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
        glLineWidth(1.0f);
        if (!prevDepthTest) glDisable(GL_DEPTH_TEST);
        if (prevBlend) glEnable(GL_BLEND);
    }



    void RendererLayer::SetupShaderUniforms(const std::shared_ptr<Shader>& shader) {
        if (!shader) return;

        shader->Bind();

    }

    void RendererLayer::RenderOpaqueObjects() {
        std::shared_ptr<Material> lastMaterial = nullptr;
        bool lastIsPBR = false;

        for (const auto& item : m_RenderQueue.opaque) {
            auto& material = item.material ? item.material : m_DefaultMaterial;
            bool currentIsPBR = item.isPBR;

            if (material != lastMaterial || currentIsPBR != lastIsPBR) {
                // 设置着色器并绑定材质
                SetupShaderUniforms(material->GetShader());
                material->Bind();

                m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
                m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

                lastMaterial = material;
                lastIsPBR = currentIsPBR;
            }

            // 设置物体的变换矩阵（这个通常不是通过 UBO 传递的）
            auto shader = material->GetShader();
            if (shader) {
                shader->SetUniformMat4("u_Transform", item.transform);
            }

            // 使用 Renderer 提交
            Renderer::Submit(material->GetShader(), item.mesh, item.transform);
        }
    }

    void RendererLayer::RenderTransparentObjects() {
        // 保存状态
        GLboolean prevDepthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
        GLboolean prevBlend = glIsEnabled(GL_BLEND);

        // 设置透明物体渲染状态
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        std::shared_ptr<Material> lastMaterial = nullptr;

        for (const auto& item : m_RenderQueue.transparent) {
            auto& material = item.material ? item.material : m_DefaultMaterial;

            if (material != lastMaterial) {
                // 设置着色器并绑定材质
                SetupShaderUniforms(material->GetShader());
                material->Bind();

                m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
                m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);

                lastMaterial = material;
            }

            // 设置物体的变换矩阵
            auto shader = material->GetShader();
            if (shader) {
                shader->SetUniformMat4("u_Transform", item.transform);
            }

            // 使用 Renderer 提交
            Renderer::Submit(material->GetShader(), item.mesh, item.transform);
        }

        // 恢复状态
        glDepthMask(prevDepthMask);
        if (!prevBlend) glDisable(GL_BLEND);
    }

    void RendererLayer::BindMaterial(const std::shared_ptr<Material>& material) {
        material->Bind();

        // 确保UBO绑定（材质绑定可能会切换shader）
        m_CameraUBO->BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
        m_LightsUBO->BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);
    }

    void RendererLayer::CreateFramebuffer(uint32_t width, uint32_t height)
    {
        DestroyFramebuffer();

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        glGenTextures(1, &m_ColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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
        GetActiveCamera().SetAspectRatio((float)width / (float)height);
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

    Camera& RendererLayer::GetActiveCamera() {
        if (m_UseEditorCamera) {
            return m_EditorCamera;
        }
        else {
            // 从场景获取主相机，如果获取失败则使用编辑器相机
            Camera* mainCam = GetMainCameraFromScene();
            return mainCam ? *mainCam : m_EditorCamera;
        }
    }

    const Camera& RendererLayer::GetActiveCamera() const {
        return const_cast<RendererLayer*>(this)->GetActiveCamera();
    }

    Camera* RendererLayer::GetMainCameraFromScene() {
    auto* activeScene = Application::GetSceneManager().GetActiveScene();
    if (!activeScene) return nullptr;

    GameObject mainCamera = activeScene->GetMainCamera();
    if (!mainCamera.IsValid()) {
        mainCamera = activeScene->FindMainCamera();
        if (mainCamera.IsValid()) {
            activeScene->SetMainCamera(mainCamera);
        }
        else {
            return nullptr;
        }
    }

    // 确保有必要的组件
    if (!mainCamera.HasComponent<CameraComponent>() ||
        !mainCamera.HasComponent<TransformComponent>()) {
        return nullptr;
    }

    auto& cameraComp = mainCamera.GetComponent<CameraComponent>();
    auto& transform = mainCamera.GetComponent<TransformComponent>();

    // 确保 m_GameCamera 存在，然后把场景相机组件的数据同步到 m_GameCamera
    if (!m_GameCamera) {
        m_GameCamera = std::make_unique<FreeCamera>(m_Window);
    }

    // 同步位移和旋转
    m_GameCamera->SetPosition(transform.transform.position);
    m_GameCamera->SetRotation(transform.transform.rotation);

    // 同步投影参数（CameraComponent 存储的是度数）
    m_GameCamera->SetPerspective(
        glm::radians(cameraComp.fov),
        (float)m_ViewportWidth / (float)m_ViewportHeight,
        cameraComp.nearClip,
        cameraComp.farClip
    );

    return m_GameCamera.get();
    }


    std::unique_ptr<Camera> RendererLayer::CreateCameraForComponent(CameraComponent& comp, GameObject cameraObject) {
        if (!cameraObject.HasComponent<TransformComponent>()) {
            return nullptr;
        }

        auto& transform = cameraObject.GetComponent<TransformComponent>();

        // 创建新的自由相机
        auto camera = std::make_unique<FreeCamera>(m_Window);

        // 设置位置和旋转
        camera->SetPosition(transform.transform.position);
        camera->SetRotation(transform.transform.rotation);

        // 设置投影参数
        camera->SetPerspective(
            glm::radians(comp.fov),
            (float)m_ViewportWidth / (float)m_ViewportHeight,
            comp.nearClip,
            comp.farClip
        );

        return camera;
    }

    void RendererLayer::SyncCameraWithTransform(Camera& camera, const Transform& transform) {
        camera.SetPosition(transform.position);
        camera.SetRotation(transform.rotation);
    }

    void RendererLayer::SyncGameCameraFromScene() {
        Camera* mainCam = GetMainCameraFromScene();
        if (mainCam && m_GameCamera) {
            // 同步位置和旋转
            m_GameCamera->SetPosition(mainCam->GetPosition());
            m_GameCamera->SetRotation(mainCam->GetRotation());

            // 同步投影参数
            m_GameCamera->SetPerspective(
                mainCam->GetFov(),
                (float)m_ViewportWidth / (float)m_ViewportHeight,
                mainCam->GetNearClip(),
                mainCam->GetFarClip()
            );

            ITR_INFO("Game Camera Synced - Pos: ({:.2f}, {:.2f}, {:.2f})",
                mainCam->GetPosition().x, mainCam->GetPosition().y, mainCam->GetPosition().z);
        }
    }

    void RendererLayer::SetUseEditorCamera(bool useEditor) {
        m_UseEditorCamera = useEditor;

        if (!useEditor) {
            // 切换到游戏相机时，确保场景中有主相机
            auto* activeScene = Application::GetSceneManager().GetActiveScene();
            if (activeScene) {
                GameObject mainCamera = activeScene->FindMainCamera();
                if (!mainCamera.IsValid()) {
                    // 如果没有主相机，创建一个默认的
                    activeScene->CreateDefaultMainCamera();
                }
            }
        }
    }

    // RendererLayer.cpp - 修改视锥体渲染逻辑
    // RendererLayer.cpp - 改进 RenderFrustum 函数
    void RendererLayer::RenderFrustum(const Frustum& frustum, const glm::vec3& color) {
        const auto& corners = frustum.GetCorners();


        auto lineShader = Application::GetShaderLibrary().Get("lineShader");
        if (!lineShader) {
            ITR_WARN("Line shader not found, frustum rendering skipped");
            return;
        }

        lineShader->Bind();

        // 使用活动相机的视图投影矩阵
        Camera& activeCam = GetActiveCamera();
        glm::mat4 viewProj = activeCam.GetProjectionMat() * activeCam.GetViewMat();
        lineShader->SetUniformMat4("u_ViewProjection", viewProj);
        lineShader->SetUniformVec3("u_Color", color);

        // 定义视锥体边（12条边）
        const std::vector<std::pair<int, int>> edges = {
            // 近平面
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            // 远平面  
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            // 连接近远平面
            {0, 4}, {1, 5}, {3, 7}, {2, 6}
        };

        // 收集所有线段顶点
        std::vector<glm::vec3> lineVertices;
        for (const auto& edge : edges) {
            lineVertices.push_back(corners[edge.first]);
            lineVertices.push_back(corners[edge.second]);
        }

        // 创建临时VAO和VBO
        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(glm::vec3),
            lineVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        // 设置线宽
        glLineWidth(2.0f);

        // 渲染所有线段
        glDrawArrays(GL_LINES, 0, (GLsizei)lineVertices.size());

        // 恢复线宽
        glLineWidth(1.0f);

        // 清理
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        lineShader->UnBind();

    }

    void RendererLayer::ReloadSkybox(const std::vector<std::string>& facePaths) {


        if (facePaths.empty()) {
            // 使用默认路径或保持原有路径
            return;
        }

        try {
            m_Skybox = std::make_unique<Skybox>(facePaths);
            ITR_INFO("Skybox reloaded successfully");
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed to reload skybox: {}", e.what());
        }
    }

    void RendererLayer::RenderSkybox() {
        if (!m_EnableSkybox || !m_Skybox) {
            return;
        }

        // 检查OpenGL状态
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            ITR_WARN("OpenGL error before rendering skybox: 0x%x", error);
            glGetError(); // 清除错误状态
        }

        // 渲染天空盒
        Camera& activeCam = GetActiveCamera();
        m_Skybox->Draw(activeCam.GetViewMat(), activeCam.GetProjectionMat());

        // 检查错误
        error = glGetError();
        if (error != GL_NO_ERROR) {
            ITR_ERROR("OpenGL error after rendering skybox: 0x%x", error);
        }
    }

    void RendererLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(RendererLayer::OnWindowResized));
    }

    bool RendererLayer::OnWindowResized(WindowResizeEvent& e)
    {
        GetActiveCamera().SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
        return false;
    }

} // namespace Intro
