// Intro/Intro/src/Intro/Renderer/RenderPass.cpp
#include "itrpch.h"
#include "RenderPass.h"

namespace Intro {

    RenderPass::RenderPass(const RenderPassSpecification& spec)
        : m_Specification(spec) {

        // 初始化公有成员以便于访问
        framebuffer = m_Specification.targetFramebuffer;
        shader = m_Specification.shader;
        clearColor = m_Specification.clearColor;
        clear = m_Specification.clear;
        depthTest = m_Specification.depthTest;

        ITR_INFO("Created RenderPass: {}", m_Specification.debugName);
    }

}