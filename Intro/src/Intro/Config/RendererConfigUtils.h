// RendererConfigUtils.h
#pragma once
#include "Intro/Core.h"
#include "Intro/Log.h"
#include "Config.h"
#include "Intro/Renderer/Renderer.h"

namespace Intro {

    class ITR_API RendererConfigUtils {
    public:
        // 将 GraphicsConfig 转换为 RendererConfig
        static RendererConfig ToRendererConfig(const GraphicsConfig& graphicsConfig) {
            RendererConfig config;
            config.viewportWidth = graphicsConfig.ViewportWidth;
            config.viewportHeight = graphicsConfig.ViewportHeight;
            config.enableMSAA = graphicsConfig.EnableMSAA;
            config.msaaSamples = graphicsConfig.MSaaSamples;
            config.enableHDR = graphicsConfig.EnableHDR;
            config.enableGammaCorrection = graphicsConfig.EnableGammaCorrection;
            return config;
        }

        // 验证渲染器配置的有效性
        static bool ValidateRendererConfig(const RendererConfig& config) {
            bool valid = true;

            if (config.viewportWidth < 64 || config.viewportWidth > 16384) {
                ITR_WARN("Viewport width out of range: {}", config.viewportWidth);
                valid = false;
            }

            if (config.viewportHeight < 64 || config.viewportHeight > 16384) {
                ITR_WARN("Viewport height out of range: {}", config.viewportHeight);
                valid = false;
            }

            if (config.msaaSamples != 1 && config.msaaSamples != 2 &&
                config.msaaSamples != 4 && config.msaaSamples != 8) {
                ITR_WARN("Invalid MSAA samples: {}", config.msaaSamples);
                valid = false;
            }

            return valid;
        }
    };

}