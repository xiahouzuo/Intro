// Renderer.cpp
#include "itrpch.h"
#include "Renderer.h"
#include "UniformBuffers.h"   // ����˵����Renderer ������Ҫ��ȫ�� UBO ������camera / lights��
#include "Intro/Application.h"
#include "Framebuffer.h"
#include "Intro/RecourceManager/ShaderLibrary.h"
#include "RenderPass.h"
#include <glad/glad.h>

namespace Intro {

    // ��̬��Ա���壨��ʼ����
    std::vector<Renderer::BatchData> Renderer::s_BatchQueue;
    Renderer::Statistics Renderer::s_Stats;
    RendererConfig Renderer::s_Config;
    std::unique_ptr<ShaderLibrary> Renderer::s_ShaderLibrary;
    std::shared_ptr<Framebuffer> Renderer::s_MainFramebuffer;
    std::shared_ptr<Framebuffer> Renderer::s_PostProcessFramebuffer;
    bool Renderer::s_Initialized = false;

    // Init: ����������Ⱦ��ϵͳ
    // - ���� ShaderLibrary�����㼯�м��� shader��
    // - ������֡���������֡���壨���� config��
    // - ���û��� GL ״̬����Ȳ��ԡ�����ü���sRGB ��ѡ��
    void Renderer::Init() {
        if (s_Initialized) return;

        ITR_INFO("Initializing Renderer...");

        // ���� shader ��ʵ��������������Ԥ���س��� shader��
        s_ShaderLibrary = std::make_unique<ShaderLibrary>();

        // ���ò�������֡���壺������ɫ + ���ģ��
        FramebufferSpecification mainFBSpec;
        mainFBSpec.width = s_Config.viewportWidth;
        mainFBSpec.height = s_Config.viewportHeight;
        mainFBSpec.samples = s_Config.enableMSAA ? s_Config.msaaSamples : 1;
        mainFBSpec.attachments = {
            { FramebufferTextureFormat::RGBA8, "Color" },
            { FramebufferTextureFormat::DEPTH24STENCIL8, "DepthStencil" }
        };

        s_MainFramebuffer = Framebuffer::Create(mainFBSpec);

        // �����õ� framebuffer��ͨ��ֻ��һ����ɫ������
        FramebufferSpecification postProcessFBSpec;
        postProcessFBSpec.width = s_Config.viewportWidth;
        postProcessFBSpec.height = s_Config.viewportHeight;
        postProcessFBSpec.attachments = {
            { FramebufferTextureFormat::RGBA8, "Color" }
        };

        s_PostProcessFramebuffer = Framebuffer::Create(postProcessFBSpec);

        // ���û����� GL ״̬����Ⱦ��ͳһ����
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        if (s_Config.enableGammaCorrection) {
            // ���� sRGB ֡�����д�루���ʹ������->sRGB ������
            glEnable(GL_FRAMEBUFFER_SRGB);
        }

        s_Initialized = true;
        ITR_INFO("Renderer initialized successfully");
    }

    // Shutdown: �����ڴ�/��Դ���������״̬
    void Renderer::Shutdown() {
        s_BatchQueue.clear();
        s_ShaderLibrary.reset();
        s_MainFramebuffer.reset();
        s_PostProcessFramebuffer.reset();
        s_Initialized = false;
        ITR_INFO("Renderer shutdown");
    }

    // BeginFrame: ÿ֡��ʼʱ����
    // - Reset ͳ��
    // - ���� FBO����ȾĿ�꣩
    // - ���� viewport ����� buffer��Ĭ�� color + depth��
    void Renderer::BeginFrame() {
        ResetStats();

        // ����֡���壬�����������Ⱦ�ύ��д����� FBO������ BeginRenderPass �ı䣩
        if (s_MainFramebuffer)
            s_MainFramebuffer->Bind();

        glViewport(0, 0, s_Config.viewportWidth, s_Config.viewportHeight);

        // Ĭ��������ɫ���ɸĳ����� RenderPass �� clearColor
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // EndFrame: ÿ֡����ʱ����
    // - Flush �ύ���У�ִ��ʵ�ʻ��ƣ�
    // - �����ִ�к�������PostProcess �ȣ�
    // - ������� FBO������Ⱦ������� swap chain ���ߺ���
    void Renderer::EndFrame() {
        // ִ����������
        FlushBatch();

        // ���ڴ���ռλ������û����� PostProcess ��ʹ�� post process FBO��
        // ���磺Renderer::PostProcess(postProcessShader);

        // �����֡���壨�ص�Ĭ��֡���壩
        if (s_MainFramebuffer)
            s_MainFramebuffer->Unbind();
    }

    // BeginRenderPass: �� RenderPass �������л���ȾĿ���������߼�
    void Renderer::BeginRenderPass(const std::shared_ptr<RenderPass>& renderPass) {
        if (!renderPass) return;

        // ��� RenderPass ���� framebuffer����󶨲����� viewport
        if (renderPass->framebuffer) {
            renderPass->framebuffer->Bind();
            glViewport(0, 0, renderPass->framebuffer->GetSpecification().width,
                renderPass->framebuffer->GetSpecification().height);
        }

        // ��ѡ�������� renderPass ָ����ɫ��
        if (renderPass->clear) {
            glClearColor(renderPass->clearColor.r, renderPass->clearColor.g,
                renderPass->clearColor.b, renderPass->clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        // ��Ȳ��Կ��ص�Ҳ������������� renderPass->depthTest ����
        if (renderPass->depthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void Renderer::EndRenderPass() {
        // �ڽ���ͨ��ʱ�� flush ��ͨ�������Σ��Ա�֤��Ⱦ˳��
        FlushBatch();
        // �����Ϊ���Ը�ϸ���������Ҫ���ض� framebuffer ���
        // ���ﲻ�Զ� Unbind��Leave to caller ��ͳһ�� EndFrame �д���
    }

    // Submit(Mesh): ������ Mesh + shader + transform �ύ������
    // - ���ڴ˴��������ƣ����ں�������/����
    void Renderer::Submit(const std::shared_ptr<Shader>& shader,
        const std::shared_ptr<Mesh>& mesh,
        const glm::mat4& transform) {
        if (!shader || !mesh) return;

        // ����Ⱦ���������
        s_BatchQueue.push_back({ shader, mesh, transform });

        // ����ͳ�ƣ�������ʹ�� mesh ��ʵ�ʶ���/������Ϣ��
        // ע�⣺GetVertices/GetIndices ��Щ������Ҫ������Ŀ�� Mesh �Ľӿ�һ��
        s_Stats.vertexCount += static_cast<uint32_t>(mesh->GetVertices().size());
        s_Stats.triangleCount += static_cast<uint32_t>(mesh->GetIndices().size() / 3);
    }

    // Submit(Model): ���� model �� meshes Ȼ���������� Submit
    void Renderer::Submit(const std::shared_ptr<Shader>& shader,
        const std::shared_ptr<Model>& model,
        const glm::mat4& transform) {
        if (!shader || !model) return;

        // Model::GetMeshes() �践�� shared_ptr<Mesh> ������������Ŀʵ��ǩ�����룩
        for (const auto& mesh : model->GetMeshes()) {
            Submit(shader, mesh, transform);
        }
    }

    // FlushBatch: �����Ѷ��л��Ƶ� GPU
    // - �����������ƣ�ÿ��� shader������ transform uniform������ RenderCommand::Draw��
    // - �Ż����������������Լ��� shader/�����л����� shader id������ȷ�Ͱ��
    void Renderer::FlushBatch() {
        if (s_BatchQueue.empty()) return;

        for (const auto& batch : s_BatchQueue) {
            // �� shader������ Shader ���� Bind / SetUniformMat4 �Ƚӿڣ�
            batch.shader->Bind();

            // ���� transform uniform��������� shader ��һ�� "u_Transform"��
            batch.shader->SetUniformMat4("u_Transform", batch.transform);

            // RenderCommand ���� VAO �󶨺� glDrawElements / DrawArrays��ǩ������룩
            // ע�⣺RenderCommand::Draw �Ĳ���������������Ŀ�е�ʵ��һ��
            RenderCommand::Draw(*batch.mesh, *batch.shader);

            s_Stats.drawCalls++;
        }

        // ��ն��У��ȴ���һ֡�ύ
        s_BatchQueue.clear();
    }

    // PostProcess: һ���򵥵ĺ�������ʾ��
    // - ���� FBO ����ɫ������Ϊ����Ԫ 0��ʹ�ô���� postProcessShader ����һ��ȫ���ı��ε� post FBO
    // - ע�⣺��Ҫʵ�ֲ�����һ��ȫ�� VAO��δ�����ڱ��ļ��У�������ֻ��������
    void Renderer::PostProcess(const std::shared_ptr<Shader>& postProcessShader) {
        if (!postProcessShader || !s_MainFramebuffer || !s_PostProcessFramebuffer) return;

        // �󶨺��ڴ���Ŀ�� FBO�����ƽ��д��� FBO��
        s_PostProcessFramebuffer->Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        // �󶨺��� shader ����������Ԫ
        postProcessShader->Bind();
        postProcessShader->SetUniformInt("u_ScreenTexture", 0);

        // ���� FBO ����ɫ�����󶨵�����Ԫ 0
        s_MainFramebuffer->BindColorTexture(0);

        // ��Ⱦȫ���ı��Σ�����Ҫʵ��һ�� ScreenQuad::Draw() �����ƺ�����
        // e.g. ScreenQuad::Get().Draw();
        // ����������һ��ȫ�� quad �� VAO/VBO�������Ѿ���ʼ����
        // �����û��ʵ�� ScreenQuad����ο���ע�е�ʵ�ֽ��顣

        // ������ FBO���ѽ��д��Ĭ�� FBO ���ں��������м���ʹ�ã�
        s_PostProcessFramebuffer->Unbind();
    }

    // �� shader ���а����ֻ�ȡ shader���������ڻ���ԣ�
    std::shared_ptr<Shader> Renderer::GetShader(const std::string& name) {
        ITR_CORE_ASSERT(s_ShaderLibrary, "ShaderLibrary not initialized!");
        return s_ShaderLibrary->Get(name);
    }

    std::shared_ptr<Framebuffer> Renderer::GetMainFramebuffer() {
        return s_MainFramebuffer;
    }

    // �ı����ã����細�� resize��
    // - ���Ѿ���ʼ��������Ҫ���� Resize ֡����
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
