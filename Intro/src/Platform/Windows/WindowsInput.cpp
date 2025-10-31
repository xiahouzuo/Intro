#include "itrpch.h"
#include "WindowsInput.h"
#include "Intro/Application.h"

#include <GLFW/glfw3.h>

namespace Intro {

    Input* Input::s_Instance = new WindowsInput();

    // ԭ�е�ʵ�ֱ��ֲ���
    bool WindowsInput::IsKeyPressedImpl(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WindowsInput::IsMouseButtonPressedImpl(int button) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> WindowsInput::GetMousePositionImpl() {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float)xpos, (float)ypos };
    }

    float WindowsInput::GetMouseXImpl() {
        auto [x, y] = GetMousePosition();
        return x;
    }

    float WindowsInput::GetMouseYImpl() {
        auto [x, y] = GetMousePosition();
        return y;
    }

    // === ����ʵ�� ===

    bool WindowsInput::IsKeyJustPressedImpl(int keycode) {
        auto currentIt = m_KeyCurrentState.find(keycode);
        auto previousIt = m_KeyPreviousState.find(keycode);
        return (currentIt != m_KeyCurrentState.end() && currentIt->second) &&
            (previousIt != m_KeyPreviousState.end() && !previousIt->second);
    }

    bool WindowsInput::IsKeyJustReleasedImpl(int keycode) {
        auto currentIt = m_KeyCurrentState.find(keycode);
        auto previousIt = m_KeyPreviousState.find(keycode);
        return (currentIt != m_KeyCurrentState.end() && !currentIt->second) &&
            (previousIt != m_KeyPreviousState.end() && previousIt->second);
    }

    bool WindowsInput::IsMouseButtonJustPressedImpl(int button) {
        auto currentIt = m_MouseCurrentState.find(button);
        auto previousIt = m_MousePreviousState.find(button);
        return (currentIt != m_MouseCurrentState.end() && currentIt->second) &&
            (previousIt != m_MousePreviousState.end() && !previousIt->second);
    }

    bool WindowsInput::IsMouseButtonJustReleasedImpl(int button) {
        auto currentIt = m_MouseCurrentState.find(button);
        auto previousIt = m_MousePreviousState.find(button);
        return (currentIt != m_MouseCurrentState.end() && !currentIt->second) &&
            (previousIt != m_MousePreviousState.end() && previousIt->second);
    }

    float WindowsInput::GetMouseScrollDeltaImpl() {
        return m_MouseScrollDelta;
    }

    void WindowsInput::SetCursorModeImpl(CursorMode mode) {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        m_CursorMode = mode;

        switch (mode) {
        case CursorMode::Normal:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case CursorMode::Hidden:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case CursorMode::Disabled:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        }
    }

    CursorMode WindowsInput::GetCursorModeImpl() {
        return m_CursorMode;
    }

    void WindowsInput::AddActionMappingImpl(const std::string& name, int keycode) {
        m_ActionMappings[name] = keycode;
    }

    void WindowsInput::RemoveActionMappingImpl(const std::string& name) {
        m_ActionMappings.erase(name);
    }

    bool WindowsInput::GetActionImpl(const std::string& name) {
        auto it = m_ActionMappings.find(name);
        if (it != m_ActionMappings.end()) {
            return IsKeyPressedImpl(it->second);
        }
        return false;
    }

    bool WindowsInput::GetActionJustPressedImpl(const std::string& name) {
        auto it = m_ActionMappings.find(name);
        if (it != m_ActionMappings.end()) {
            return IsKeyJustPressedImpl(it->second);
        }
        return false;
    }

    bool WindowsInput::GetActionJustReleasedImpl(const std::string& name) {
        auto it = m_ActionMappings.find(name);
        if (it != m_ActionMappings.end()) {
            return IsKeyJustReleasedImpl(it->second);
        }
        return false;
    }

    void WindowsInput::UpdateImpl() {
        // ���°���״̬������ǰ״̬���Ƶ���һ֡״̬
        m_KeyPreviousState = m_KeyCurrentState;
        m_MousePreviousState = m_MouseCurrentState;

        // ���µ�ǰ����״̬
        for (auto& [keycode, _] : m_KeyCurrentState) {
            m_KeyCurrentState[keycode] = IsKeyPressedImpl(keycode);
        }

        // ���µ�ǰ��갴ť״̬
        for (auto& [button, _] : m_MouseCurrentState) {
            m_MouseCurrentState[button] = IsMouseButtonPressedImpl(button);
        }

        // ��������������
        m_MouseScrollDelta = 0.0f;
    }

}