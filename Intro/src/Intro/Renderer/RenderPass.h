
#pragma once
#include "Framebuffer.h"
#include "Shader.h"
#include <memory>
#include <glm/glm.hpp>

namespace Intro {

    struct RenderPassSpecification {
        std::shared_ptr<Framebuffer> targetFramebuffer;
        std::shared_ptr<Shader> shader;
        glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        bool clear = true;
        bool depthTest = true;
        std::string debugName;

        RenderPassSpecification() = default;
        RenderPassSpecification(
            std::shared_ptr<Framebuffer> fb,
            std::shared_ptr<Shader> shader = nullptr,
            const glm::vec4& clearColor = { 0.1f, 0.1f, 0.1f, 1.0f },
            bool clear = true,
            bool depthTest = true,
            const std::string& name = "UnnamedPass"
        ) : targetFramebuffer(fb), shader(shader), clearColor(clearColor),
            clear(clear), depthTest(depthTest), debugName(name) {
        }
    };

    class ITR_API RenderPass {
    public:
        RenderPass(const RenderPassSpecification& spec);

        const RenderPassSpecification& GetSpecification() const { return m_Specification; }

        std::shared_ptr<Framebuffer> GetTargetFramebuffer() const { return m_Specification.targetFramebuffer; }
        std::shared_ptr<Shader> GetShader() const { return m_Specification.shader; }

        // 为了方便访问
        std::shared_ptr<Framebuffer> framebuffer;
        std::shared_ptr<Shader> shader;
        glm::vec4 clearColor;
        bool clear;
        bool depthTest;

        // 辅助方法
        void SetClearColor(const glm::vec4& color) {
            clearColor = color;
            m_Specification.clearColor = color;
        }

        void SetShader(const std::shared_ptr<Shader>& newShader) {
            shader = newShader;
            m_Specification.shader = newShader;
        }

    private:
        RenderPassSpecification m_Specification;
    };

}