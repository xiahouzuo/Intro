#pragma once
#pragma once
#include "Intro/Core.h"
#include <memory>
#include <vector>
#include <string>

// ˵����Framebuffer �Ƕ� OpenGL FBO��Frame Buffer Object���ķ�װ��
// ���ã�
// - ����Ⱦ���������Զ���Ŀ������������ֱ�ӵ���Ļ��
// - ֧�ֶ฽�� (color0, color1...) �����/ģ�帽����
// - Renderer ʹ������ʵ�ֶ�ͨ����Ⱦ��������Ӱ��ͼ�ȡ�

namespace Intro {

    // FramebufferTextureFormat��ö�����п��ܵ������ʽ����
    enum class FramebufferTextureFormat {
        None = 0,

        // ��ɫ������ʽ
        RGBA8,
        RGBA16F,   // �߶�̬ HDR
        RED_INTEGER, // ����ѡȡ/ID ��Ⱦ
        // ���+ģ�帽��
        DEPTH24STENCIL8
    };

    // FramebufferTextureSpecification�����������Ĺ��˵��
    struct FramebufferTextureSpecification {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format, const std::string& debugName = "")
            : textureFormat(format), name(debugName) {
        }
        FramebufferTextureFormat textureFormat = FramebufferTextureFormat::None;
        std::string name; // ���ڵ���/UI ��ʾ
    };

    // FramebufferSpecification������֡���������
    struct FramebufferSpecification {
        uint32_t width = 0, height = 0;           // �ߴ�
        uint32_t samples = 1;                     // MSAA ������
        std::vector<FramebufferTextureSpecification> attachments; // �����б�
        bool swapChainTarget = false;             // �Ƿ���������Ĭ�� false��
    };

    class ITR_API Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        // ����ɫ����ĳ������Ԫ�����ں���
        virtual void BindColorTexture(uint32_t slot = 0, uint32_t index = 0) const = 0;

        // ��ȡ��ɫ��������Ⱦ ID��OpenGL ���� ID��
        virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

        // ��ȡ��ǰ���Renderer ������Ե����ֱ��ʣ�
        virtual const FramebufferSpecification& GetSpecification() const = 0;

        // ��̬�������������ز�ͬ API �ľ���ʵ�֣�OpenGLFramebuffer��
        static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Intro
