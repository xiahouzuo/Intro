// ConfigObserver.h
#pragma once
#include "Config.h"
#include "RendererConfigUtils.h"
#include "Intro/Renderer/Renderer.h"
#include <functional>
#include <vector>

namespace Intro {

    class ITR_API ConfigObserver {
    public:
        static ConfigObserver& Get() {
            static ConfigObserver instance;
            return instance;
        }

        // 注册配置变更回调
        void RegisterGraphicsConfigCallback(std::function<void(const GraphicsConfig&)> callback) {
            m_GraphicsCallbacks.push_back(callback);
        }

        // 当配置变更时调用（通常在UI或配置重载时）
        void OnGraphicsConfigChanged(const GraphicsConfig& newConfig) {
            // 更新渲染器配置
            RendererConfig rendererConfig = RendererConfigUtils::ToRendererConfig(newConfig);
            if (RendererConfigUtils::ValidateRendererConfig(rendererConfig)) {
                Renderer::SetConfig(rendererConfig);
                ITR_INFO("Renderer config updated");
            }

            // 通知所有注册的回调
            for (auto& callback : m_GraphicsCallbacks) {
                callback(newConfig);
            }
        }

    private:
        ConfigObserver() = default;
        std::vector<std::function<void(const GraphicsConfig&)>> m_GraphicsCallbacks;
    };

}