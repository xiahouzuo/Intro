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

        // 矩阵接口（派生类必须实现）
        virtual glm::mat4 GetViewMat() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::quat GetRotation() const = 0;
        // Fov 在 Camera 内部以 弧度 (radians) 存储，因此直接使用 Fov（不要再调用 glm::radians）
        virtual glm::mat4 GetProjectionMat() const {
            return glm::perspective(Fov, AspectRatio, NearClip, FarClip);
        }
        virtual float GetFov() { return Fov; }
        virtual float GetNearClip() { return NearClip; }
        virtual float GetFarClip() { return FarClip; }
        virtual glm::vec3 GetFront() const = 0;
        virtual void SetPosition(glm::vec3 position) = 0;
        virtual void SetRotation(glm::quat rotation) = 0;
        virtual void SetPerspective(float radins, float AspectRatio, float nearClip, float farClip) = 0;

        virtual void OnUpdate(float deltaTime) = 0;

        // 常用设置
        float GetAspectRatio() { return AspectRatio; }

        // 注意：SetFov 直接设置内部 Fov（单位为 弧度）
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

        void SetNearClip(float nearClip)
        {
            NearClip = nearClip;
        }

        void SetFarClip(float farClip)
        {
            FarClip = farClip;
        }

        float AspectRatio = 16.0f / 9.0f;
    protected:
        // 通用参数
        // 存储单位：弧度（radians）
        float Fov = glm::radians(45.0f);
        float NearClip = 0.1f;
        float FarClip = 1000.0f;
    };

} // namespace Intro
