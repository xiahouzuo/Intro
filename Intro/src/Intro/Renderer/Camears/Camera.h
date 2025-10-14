#pragma once
#include "Intro/Core.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Intro/Log.h"
#include "Intro/Window.h"

namespace Intro {

    class ITR_API Camera
    {
    public:
        Camera() = default;
        Camera(const Window& window) { SetAspectRatio((float)window.GetWidth() / (float)window.GetHeight()); }
        virtual ~Camera() = default;

        // ����ӿڣ����������ʵ�֣�
        virtual glm::mat4 GetViewMat() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::mat4 GetProjectionMat() const {
            return glm::perspective(glm::radians(Fov), AspectRatio, NearClip, FarClip);
        }
        virtual glm::vec3 GetFront() const = 0;

        virtual void OnUpdate(float deltaTime) = 0;

        // ��������
        float GetAspectRatio() { return AspectRatio; }

        void SetFov(float f) { Fov = f; }

        void SetAspectRatio(float ar) {
            if (std::isnan(ar) || std::isinf(ar) || ar <= 0.0f) {
                ITR_CORE_WARN("Invalid aspect ratio ({0}), using default 16:9", ar);
                AspectRatio = 16.0f / 9.0f;
            }
            else {
                AspectRatio = ar;
            }
        }

        float AspectRatio = 16.0f / 9.0f;
    protected:
        // ͨ�ò���
        float Fov = 45.0f;
        float NearClip = 0.1f;
        float FarClip = 1000.0f;
    };

} // namespace Intro
