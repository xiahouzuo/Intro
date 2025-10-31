#pragma once
#include "Intro/Input.h"
#include <unordered_map>
#include <string>

namespace Intro {

    class WindowsInput : public Input {
    protected:
        // 原有的实现
        virtual bool IsKeyPressedImpl(int keycode) override;
        virtual bool IsMouseButtonPressedImpl(int button) override;
        virtual std::pair<float, float> GetMousePositionImpl() override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;

        // === 新增实现 ===
        virtual bool IsKeyJustPressedImpl(int keycode) override;
        virtual bool IsKeyJustReleasedImpl(int keycode) override;
        virtual bool IsMouseButtonJustPressedImpl(int button) override;
        virtual bool IsMouseButtonJustReleasedImpl(int button) override;
        virtual float GetMouseScrollDeltaImpl() override;
        virtual void SetCursorModeImpl(CursorMode mode) override;
        virtual CursorMode GetCursorModeImpl() override;
        virtual void AddActionMappingImpl(const std::string& name, int keycode) override;
        virtual void RemoveActionMappingImpl(const std::string& name) override;
        virtual bool GetActionImpl(const std::string& name) override;
        virtual bool GetActionJustPressedImpl(const std::string& name) override;
        virtual bool GetActionJustReleasedImpl(const std::string& name) override;
        virtual void UpdateImpl() override;

    private:
        // 状态存储
        std::unordered_map<int, bool> m_KeyCurrentState;
        std::unordered_map<int, bool> m_KeyPreviousState;
        std::unordered_map<int, bool> m_MouseCurrentState;
        std::unordered_map<int, bool> m_MousePreviousState;
        std::unordered_map<std::string, int> m_ActionMappings;

        float m_MouseScrollDelta = 0.0f;
        CursorMode m_CursorMode = CursorMode::Normal;
    };

}