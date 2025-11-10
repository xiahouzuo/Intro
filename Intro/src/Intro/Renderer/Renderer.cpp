// Renderer.cpp
#include "itrpch.h"
#include "Renderer.h"
#include "UniformBuffers.h"   // 用于说明：Renderer 可能需要与全局 UBO 合作（camera / lights）
#include "Intro/Application.h"
#include "Framebuffer.h"
#include "Intro/RecourceManager/ShaderLibrary.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "Model.h"
#include <glad/glad.h>

namespace Intro {

    static bool s_MainFramebufferBoundByRenderer = false;

    // 静态成员定义（初始化）
    std::vector<Renderer::BatchData> Renderer::s_BatchQueue;
    Renderer::Statistics Renderer::s_Stats;
    RendererConfig Renderer::s_Config;
    std::unique_ptr<ShaderLibrary> Renderer::s_ShaderLibrary;
    std::shared_ptr<Framebuffer> Renderer::s_MainFramebuffer;
    std::shared_ptr<Framebuffer> Renderer::s_PostProcessFramebuffer;
    bool Renderer::s_Initialized = false;

    // Init: 负责启动渲染子系统
    // - 创建 ShaderLibrary（方便集中加载 shader）
    // - 创建主帧缓冲与后处理帧缓冲（根据 config）
    // - 设置基础 GL 状态（深度测试、背面裁剪、sRGB 可选）
    void Renderer::Init() {
        if (s_Initialized) return;

        ITR_INFO("Initializing Renderer...");

        // 创建 shader 库实例（可以在这里预加载常用 shader）
        s_ShaderLibrary = std::make_unique<ShaderLibrary>();

        // 配置并创建主帧缓冲：包含颜色 + 深度模板
        FramebufferSpecification mainFBSpec;
        mainFBSpec.width = s_Config.viewportWidth;
        mainFBSpec.height = s_Config.viewportHeight;
        mainFBSpec.samples = s_Config.enableMSAA ? s_Config.msaaSamples : 1;
        mainFBSpec.attachments = {
            { FramebufferTextureFormat::RGBA8, "Color" },
            { FramebufferTextureFormat::DEPTH24STENCIL8, "DepthStencil" }
        };

        s_MainFramebuffer = Framebuffer::Create(mainFBSpec);

        // 后处理用的 framebuffer（通常只有一个颜色附件）
        FramebufferSpecification postProcessFBSpec;
        postProcessFBSpec.width = s_Config.viewportWidth;
        postProcessFBSpec.height = s_Config.viewportHeight;
        postProcessFBSpec.attachments = {
            { FramebufferTextureFormat::RGBA8, "Color" }
        };

        s_PostProcessFramebuffer = Framebuffer::Create(postProcessFBSpec);

        // 设置基本的 GL 状态（渲染器统一管理）
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        s_Initialized = true;
        ITR_INFO("Renderer initialized successfully");
    }

    // Shutdown: 清理内存/资源句柄并重置状态
    void Renderer::Shutdown() {
        s_BatchQueue.clear();
        s_ShaderLibrary.reset();
        s_MainFramebuffer.reset();
        s_PostProcessFramebuffer.reset();
        s_Initialized = false;
        ITR_INFO("Renderer shutdown");
    }

    // BeginFrame: 每帧开始时调用
    // - Reset 统计
    // - 绑定主 FBO（渲染目标）
    // - 设置 viewport 并清除 buffer（默认 color + depth）
    void Renderer::BeginFrame() {
        ResetStats();

        // 检查当前绑定的帧缓冲
        GLint currentlyBound = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentlyBound);

        // 只有当当前没有绑定任何 FBO（即绑定到默认帧缓冲）时，由 Renderer 绑定并管理 s_MainFramebuffer
        s_MainFramebufferBoundByRenderer = false;
        if (currentlyBound == 0) {
            if (s_MainFramebuffer)
                s_MainFramebuffer->Bind();
            glViewport(0, 0, s_Config.viewportWidth, s_Config.viewportHeight);

            // 由 Renderer 清除当前（此时是主 FBO 或默认 FBO）
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            s_MainFramebufferBoundByRenderer = true;
        }
        else {
            // 已经有外部 FBO 被绑定（例如 RendererLayer::BindRenderState）
            // 不去绑定/覆盖它，但仍然清除当前绑定的缓冲（通常外部 Bind 已设置合适 viewport）
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }

    // EndFrame: 每帧结束时调用
    // - Flush 提交队列（执行实际绘制）
    // - 这里可执行后处理链（PostProcess 等）
    // - 最后解绑主 FBO（将渲染结果交给 swap chain 或者后处理）
    void Renderer::EndFrame() {
        // 执行批量绘制
        FlushBatch();
        // 解绑主帧缓冲，但仅当本帧是由 Renderer 绑定时才解绑
        if (s_MainFramebuffer && s_MainFramebufferBoundByRenderer) {
            s_MainFramebuffer->Unbind();
        }

    }

    // BeginRenderPass: 用 RenderPass 对象来切换渲染目标与清屏逻辑
    void Renderer::BeginRenderPass(const std::shared_ptr<RenderPass>& renderPass) {
        if (!renderPass) return;

        // 如果 RenderPass 含有 framebuffer，则绑定并设置 viewport
        if (renderPass->framebuffer) {
            renderPass->framebuffer->Bind();
            glViewport(0, 0, renderPass->framebuffer->GetSpecification().width,
                renderPass->framebuffer->GetSpecification().height);
        }

        // 可选清屏（由 renderPass 指定颜色）
        if (renderPass->clear) {
            glClearColor(renderPass->clearColor.r, renderPass->clearColor.g,
                renderPass->clearColor.b, renderPass->clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        // 深度测试开关等也可以在这里根据 renderPass->depthTest 控制
        if (renderPass->depthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void Renderer::EndRenderPass() {
        // 在结束通道时先 flush 本通道的批次（以保证渲染顺序）
        FlushBatch();
        // 解绑行为可以更细化（如果需要把特定 framebuffer 解绑）
        // 这里不自动 Unbind，Leave to caller 或统一在 EndFrame 中处理
    }

    // Submit(Mesh): 将单个 Mesh + shader + transform 提交到队列
    // - 不在此处立即绘制，便于后续排序/合批
    void Renderer::Submit(const std::shared_ptr<Shader>& shader,
        const std::shared_ptr<Mesh>& mesh,
        const glm::mat4& transform) {
        if (!shader || !mesh) return;

        // 将渲染项推入队列
        s_BatchQueue.push_back({ shader, mesh, transform });

        // 更新统计（尽可能使用 mesh 的实际顶点/索引信息）
        // 注意：GetVertices/GetIndices 这些函数名要与你项目中 Mesh 的接口一致
        s_Stats.vertexCount += static_cast<uint32_t>(mesh->GetVertices().size());
        s_Stats.triangleCount += static_cast<uint32_t>(mesh->GetIndices().size() / 3);
    }

    // Submit(Model): 遍历 model 的 meshes 然后调用上面的 Submit
    void Renderer::Submit(const std::shared_ptr<Shader>& shader,
        const std::shared_ptr<Model>& model,
        const glm::mat4& transform) {
        if (!shader || !model) return;

        // Model::GetMeshes() 需返回 shared_ptr<Mesh> 的容器（与项目实际签名对齐）
        for (const auto& mesh : model->GetMeshes()) {
            Submit(shader, mesh, transform);
        }
    }

    // FlushBatch: 真正把队列绘制到 GPU
    // - 这里简单逐项绘制（每项绑定 shader、设置 transform uniform、调用 RenderCommand::Draw）
    // - 优化方向：在这里排序以减少 shader/材质切换（按 shader id、纹理等分桶）
    void Renderer::FlushBatch() {
        if (s_BatchQueue.empty()) return;

        for (const auto& batch : s_BatchQueue) {
            // 绑定 shader（假设 Shader 类有 Bind / SetUniformMat4 等接口）
            batch.shader->Bind();

            // 设置 transform uniform（名称需和 shader 里一致 "u_Transform"）
            batch.shader->SetUniformMat4("u_Transform", batch.transform);

            // RenderCommand 负责 VAO 绑定和 glDrawElements / DrawArrays（签名需对齐）
            // 注意：RenderCommand::Draw 的参数类型需与你项目中的实现一致
            RenderCommand::Draw(*batch.mesh, *batch.shader);

            s_Stats.drawCalls++;
        }

        // 清空队列，等待下一帧提交
        s_BatchQueue.clear();
    }

    // PostProcess: 一个简单的后处理流程示例
    // - 把主 FBO 的颜色附件绑定为纹理单元 0，使用传入的 postProcessShader 绘制一个全屏四边形到 post FBO
    // - 注意：需要实现并缓存一个全屏 VAO（未包含在本文件中），这里只给出流程
    void Renderer::PostProcess(const std::shared_ptr<Shader>& postProcessShader) {
        if (!postProcessShader || !s_MainFramebuffer || !s_PostProcessFramebuffer) return;

        // 绑定后期处理目标 FBO（绘制结果写入此 FBO）
        s_PostProcessFramebuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定后处理 shader 并设置纹理单元
        postProcessShader->Bind();
        postProcessShader->SetUniformInt("u_ScreenTexture", 0);

        // 将主 FBO 的颜色附件绑定到纹理单元 0
        s_MainFramebuffer->BindColorTexture(0);

        // 渲染全屏四边形（你需要实现一个 ScreenQuad::Draw() 或类似函数）
        // e.g. ScreenQuad::Get().Draw();
        // 这里假设存在一个全屏 quad 的 VAO/VBO，并且已经初始化好
        // 如果你没有实现 ScreenQuad，请参考备注中的实现建议。

        // 解绑后处理 FBO（把结果写回默认 FBO 或在后续步骤中继续使用）
        s_PostProcessFramebuffer->Unbind();
    }

    // 从 shader 库中按名字获取 shader（若不存在会断言）
    std::shared_ptr<Shader> Renderer::GetShader(const std::string& name) {
        ITR_CORE_ASSERT(s_ShaderLibrary, "ShaderLibrary not initialized!");
        return s_ShaderLibrary->Get(name);
    }

    std::shared_ptr<Framebuffer> Renderer::GetMainFramebuffer() {
        return s_MainFramebuffer;
    }

    // 改变配置（例如窗口 resize）
    // - 若已经初始化，则需要重新 Resize 帧缓冲
    void Renderer::SetConfig(const RendererConfig& config) {
        s_Config = config;
        if (s_Initialized) {
            if (s_MainFramebuffer)
                s_MainFramebuffer->Resize(config.viewportWidth, config.viewportHeight);
            if (s_PostProcessFramebuffer)
                s_PostProcessFramebuffer->Resize(config.viewportWidth, config.viewportHeight);
        }
    }

    const RendererConfig& Renderer::GetConfig() {
        return s_Config;
    }

    const Renderer::Statistics& Renderer::GetStats() {
        return s_Stats;
    }

    void Renderer::ResetStats() {
        s_Stats.Reset();
    }

} // namespace Intro
