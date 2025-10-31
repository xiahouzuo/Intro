#pragma once
#include "Intro/Core.h"
#include "KeyCodes.h"
#include "MouseButtonCodes.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace Intro {

    enum class CursorMode {
        Normal = 0,
        Hidden = 1,
        Disabled = 2
    };

    class ITR_API Input {
    public:
        // 原有的静态函数
        inline static bool IsKeyPressed(int keycode) {
            return s_Instance->IsKeyPressedImpl(keycode);
        }

        inline static bool IsMouseButtonPressed(int button) {
            return s_Instance->IsMouseButtonPressedImpl(button);
        }

        inline static std::pair<float, float> GetMousePosition() {
            return s_Instance->GetMousePositionImpl();
        }

        inline static float GetMouseX() {
            return s_Instance->GetMouseXImpl();
        }

        inline static float GetMouseY() {
            return s_Instance->GetMouseYImpl();
        }

        // === 新增功能 ===

        // 刚按下/刚释放检测
        inline static bool IsKeyJustPressed(int keycode) {
            return s_Instance->IsKeyJustPressedImpl(keycode);
        }

        inline static bool IsKeyJustReleased(int keycode) {
            return s_Instance->IsKeyJustReleasedImpl(keycode);
        }

        inline static bool IsMouseButtonJustPressed(int button) {
            return s_Instance->IsMouseButtonJustPressedImpl(button);
        }

        inline static bool IsMouseButtonJustReleased(int button) {
            return s_Instance->IsMouseButtonJustReleasedImpl(button);
        }

        // 鼠标滚动
        inline static float GetMouseScrollDelta() {
            return s_Instance->GetMouseScrollDeltaImpl();
        }

        // 光标模式
        inline static void SetCursorMode(CursorMode mode) {
            s_Instance->SetCursorModeImpl(mode);
        }

        inline static CursorMode GetCursorMode() {
            return s_Instance->GetCursorModeImpl();
        }

        // 输入映射
        inline static void AddActionMapping(const std::string& name, int keycode) {
            s_Instance->AddActionMappingImpl(name, keycode);
        }

        inline static void RemoveActionMapping(const std::string& name) {
            s_Instance->RemoveActionMappingImpl(name);
        }

        inline static bool GetAction(const std::string& name) {
            return s_Instance->GetActionImpl(name);
        }

        inline static bool GetActionJustPressed(const std::string& name) {
            return s_Instance->GetActionJustPressedImpl(name);
        }

        inline static bool GetActionJustReleased(const std::string& name) {
            return s_Instance->GetActionJustReleasedImpl(name);
        }

        // 内部更新
        inline static void Update() {
            s_Instance->UpdateImpl();
        }

    protected:
        // 原有的虚函数
        virtual bool IsKeyPressedImpl(int keycode) = 0;
        virtual bool IsMouseButtonPressedImpl(int button) = 0;
        virtual std::pair<float, float> GetMousePositionImpl() = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;

        // === 新增虚函数 ===
        virtual bool IsKeyJustPressedImpl(int keycode) = 0;
        virtual bool IsKeyJustReleasedImpl(int keycode) = 0;
        virtual bool IsMouseButtonJustPressedImpl(int button) = 0;
        virtual bool IsMouseButtonJustReleasedImpl(int button) = 0;
        virtual float GetMouseScrollDeltaImpl() = 0;
        virtual void SetCursorModeImpl(CursorMode mode) = 0;
        virtual CursorMode GetCursorModeImpl() = 0;
        virtual void AddActionMappingImpl(const std::string& name, int keycode) = 0;
        virtual void RemoveActionMappingImpl(const std::string& name) = 0;
        virtual bool GetActionImpl(const std::string& name) = 0;
        virtual bool GetActionJustPressedImpl(const std::string& name) = 0;
        virtual bool GetActionJustReleasedImpl(const std::string& name) = 0;
        virtual void UpdateImpl() = 0;

    private:
        static Input* s_Instance;
    };

}