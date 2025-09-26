#pragma once
#include "Intro/Core.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Intro/Window.h"

namespace Intro {

    class ITR_API Camera
    {
    public:
        Camera() = default;
        Camera(const Window& window) { AspectRatio = (float)window.GetWidth() / (float)window.GetHeight(); }
        virtual ~Camera() = default;

        // 矩阵接口（派生类必须实现）
        virtual glm::mat4 GetViewMat() const = 0;
        virtual glm::mat4 GetProjectionMat() const {
            return glm::perspective(glm::radians(Fov), AspectRatio, NearClip, FarClip);
        }

        // 运行时调用（处理输入/移动/旋转）
        virtual void OnUpdate(float deltaTime) = 0;

        // 常用设置
        float GetAspectRatio() { return AspectRatio; }
        void SetAspectRatio(float ar) { AspectRatio = ar; }
        void SetFov(float f) { Fov = f; }

        float AspectRatio = 16.0f / 9.0f;
    protected:
        // 通用参数
        float Fov = 45.0f;
        float NearClip = 0.1f;
        float FarClip = 1000.0f;
    };

} // namespace Intro
