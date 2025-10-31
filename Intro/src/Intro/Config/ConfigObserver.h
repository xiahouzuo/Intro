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

        // ע�����ñ���ص�
        void RegisterGraphicsConfigCallback(std::function<void(const GraphicsConfig&)> callback) {
            m_GraphicsCallbacks.push_back(callback);
        }

        // �����ñ��ʱ���ã�ͨ����UI����������ʱ��
        void OnGraphicsConfigChanged(const GraphicsConfig& newConfig) {
            // ������Ⱦ������
            RendererConfig rendererConfig = RendererConfigUtils::ToRendererConfig(newConfig);
            if (RendererConfigUtils::ValidateRendererConfig(rendererConfig)) {
                Renderer::SetConfig(rendererConfig);
                ITR_INFO("Renderer config updated");
            }

            // ֪ͨ����ע��Ļص�
            for (auto& callback : m_GraphicsCallbacks) {
                callback(newConfig);
            }
        }

    private:
        ConfigObserver() = default;
        std::vector<std::function<void(const GraphicsConfig&)>> m_GraphicsCallbacks;
    };

}