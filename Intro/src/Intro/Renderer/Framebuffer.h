#pragma once
#pragma once
#include "Intro/Core.h"
#include <memory>
#include <vector>
#include <string>

// 说明：Framebuffer 是对 OpenGL FBO（Frame Buffer Object）的封装。
// 作用：
// - 把渲染结果输出到自定义目标纹理，而不是直接到屏幕。
// - 支持多附件 (color0, color1...) 与深度/模板附件。
// - Renderer 使用它来实现多通道渲染、后处理、阴影贴图等。

namespace Intro {

    // FramebufferTextureFormat：枚举所有可能的纹理格式类型
    enum class FramebufferTextureFormat {
        None = 0,

        // 颜色附件格式
        RGBA8,
        RGBA16F,   // 高动态 HDR
        RED_INTEGER, // 用于选取/ID 渲染
        // 深度+模板附件
        DEPTH24STENCIL8
    };

    // FramebufferTextureSpecification：单个附件的规格说明
    struct FramebufferTextureSpecification {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format, const std::string& debugName = "")
            : textureFormat(format), name(debugName) {
        }
        FramebufferTextureFormat textureFormat = FramebufferTextureFormat::None;
        std::string name; // 用于调试/UI 显示
    };

    // FramebufferSpecification：整个帧缓冲的描述
    struct FramebufferSpecification {
        uint32_t width = 0, height = 0;           // 尺寸
        uint32_t samples = 1;                     // MSAA 样本数
        std::vector<FramebufferTextureSpecification> attachments; // 附件列表
        bool swapChainTarget = false;             // 是否是主屏（默认 false）
    };

    class ITR_API Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        // 绑定颜色纹理到某个纹理单元（用于后处理）
        virtual void BindColorTexture(uint32_t slot = 0, uint32_t index = 0) const = 0;

        // 获取颜色附件的渲染 ID（OpenGL 纹理 ID）
        virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

        // 获取当前规格（Renderer 会调用以调整分辨率）
        virtual const FramebufferSpecification& GetSpecification() const = 0;

        // 静态创建函数：返回不同 API 的具体实现（OpenGLFramebuffer）
        static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Intro
