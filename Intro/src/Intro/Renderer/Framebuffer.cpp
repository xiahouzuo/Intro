#include "itrpch.h"
#include "Framebuffer.h"
#include <glad/glad.h>

namespace Intro {

    // 将枚举映射为实际的 OpenGL 格式
    static GLenum TextureTarget(bool multisampled) {
        return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    }

    static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count) {
        glGenTextures(count, outID);
    }

    static void BindTexture(bool multisampled, uint32_t id) {
        glBindTexture(TextureTarget(multisampled), id);
    }

    // 根据格式返回 OpenGL 内部格式与数据类型
    static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format,
        uint32_t width, uint32_t height, int index) {
        bool multisampled = samples > 1;
        if (multisampled) {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
        }
        else {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
    }

    static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType,
        uint32_t width, uint32_t height) {
        bool multisampled = samples > 1;
        if (multisampled) {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else {
            glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
    }

    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec)
            : m_Specification(spec) {
            Invalidate();
        }

        ~OpenGLFramebuffer() override {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);
        }

        void Invalidate() {
            if (m_RendererID) {
                glDeleteFramebuffers(1, &m_RendererID);
                glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
                glDeleteTextures(1, &m_DepthAttachment);

                m_ColorAttachments.clear();
                m_DepthAttachment = 0;
            }

            glGenFramebuffers(1, &m_RendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

            bool multisampled = m_Specification.samples > 1;

            std::vector<FramebufferTextureSpecification> colorAttachmentSpecs;
            FramebufferTextureSpecification depthAttachmentSpec;

            for (auto& spec : m_Specification.attachments) {
                if (spec.textureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
                    depthAttachmentSpec = spec;
                else
                    colorAttachmentSpecs.push_back(spec);
            }

            if (!colorAttachmentSpecs.empty()) {
                m_ColorAttachments.resize(colorAttachmentSpecs.size());
                CreateTextures(multisampled, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());
                for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
                    BindTexture(multisampled, m_ColorAttachments[i]);
                    switch (colorAttachmentSpecs[i].textureFormat) {
                    case FramebufferTextureFormat::RGBA8:
                        AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_RGBA8, GL_RGBA,
                            m_Specification.width, m_Specification.height, (int)i);
                        break;
                    }
                }
            }

            if (depthAttachmentSpec.textureFormat != FramebufferTextureFormat::None) {
                CreateTextures(multisampled, &m_DepthAttachment, 1);
                BindTexture(multisampled, m_DepthAttachment);
                AttachDepthTexture(m_DepthAttachment, m_Specification.samples, GL_DEPTH24_STENCIL8,
                    GL_DEPTH_STENCIL_ATTACHMENT,
                    m_Specification.width, m_Specification.height);
            }

            if (m_ColorAttachments.size() > 1) {
                GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
                glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
            }
            else if (m_ColorAttachments.empty()) {
                glDrawBuffer(GL_NONE);
            }

            ITR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete!");

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void Bind() override {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        }

        void Unbind() override {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void Resize(uint32_t width, uint32_t height) override {
            if (width == 0 || height == 0) return;
            m_Specification.width = width;
            m_Specification.height = height;
            Invalidate();
        }

        void BindColorTexture(uint32_t slot, uint32_t index) const override {
            glBindTextureUnit(slot, m_ColorAttachments[index]);
        }

        uint32_t GetColorAttachmentRendererID(uint32_t index) const override {
            return m_ColorAttachments[index];
        }

        const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0;
    };

    std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
        return std::make_shared<OpenGLFramebuffer>(spec);
    }

} // namespace Intro
