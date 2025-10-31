#pragma once
// Config.h
#pragma once

#include "Intro/Core.h"    // ITR_API
#include <string>
#include <unordered_map>
#include <optional>
#include <glm/glm.hpp>

namespace Intro {

    enum class GraphicsQuality {
        Low = 0,
        Medium = 1,
        High = 2,
        Ultra = 3
    };

    enum class AntiAliasing {
        None = 0,
        MSAA2x = 2,
        MSAA4x = 4,
        MSAA8x = 8
    };



    struct WindowConfig {
        int Width = 1920;
        int Height = 1080;
        bool Fullscreen = false;
        bool VSync = true;
        int RefreshRate = 60;
        std::string Title = "Intro Engine";
    };

    struct GraphicsConfig {
        GraphicsQuality Quality = GraphicsQuality::High;
        AntiAliasing AA = AntiAliasing::MSAA4x;
        bool Shadows = true;
        bool Bloom = true;
        bool SSAO = true;
        float Gamma = 2.2f;
        float Exposure = 1.0f;
        int MaxFPS = 0; // 0 = unlimited

        // ������Ⱦ������
        bool EnableMSAA = true;
        uint32_t MSaaSamples = 4;
        bool EnableHDR = false;
        bool EnableGammaCorrection = true;
        uint32_t ViewportWidth = 1920;
        uint32_t ViewportHeight = 1080;

        // ���ڴ�������
        bool EnablePostProcessing = true;
        float BloomThreshold = 1.0f;
        float BloomIntensity = 0.8f;
        float VignetteIntensity = 0.3f;
        float ChromaticAberration = 0.0f;
    };

    struct InputConfig {
        float MouseSensitivity = 0.1f;
        bool InvertMouseY = false;
        std::unordered_map<std::string, int> ActionMappings = {
            {"MoveForward", 87},      // 'W'
            {"MoveBackward", 83},     // 'S'
            {"MoveLeft", 65},         // 'A'
            {"MoveRight", 68},        // 'D'
            {"Jump", 32},             // Space
            {"Sprint", 340},          // Left Shift (example GLFW)
            {"Crouch", 341},          // Left Control (example GLFW)
            {"Interact", 69},         // 'E'
            {"Menu", 27}              // Escape
        };
    };

    struct EditorConfig {
        bool ShowGrid = true;
        bool ShowGizmos = true;
        float GridSize = 10.0f;
        glm::vec3 GridColor = glm::vec3(0.5f);
        glm::vec3 BackgroundColor = glm::vec3(0.1f);
        bool AutoSave = true;
        int AutoSaveInterval = 300; // seconds
    };

    class ITR_API Config {
    public:
        // ��ȡ����
        static Config& Get();

        // Load/Save
        // ��� filepath Ϊ�գ�Load() �᳢��Ĭ��·��������Ŀ¼���ȣ�����������ʹ���û�Ŀ¼��
        bool Load(const std::string& filepath = "");
        // Save: filepath ���򱣴浽�ϴμ���/Ĭ��·��
        bool Save(const std::string& filepath = "");

        // Defaults / reset
        void ResetToDefaults();

        // Accessors (���ñ��� UI �޸�ֱ����Ч)
        WindowConfig& GetWindowConfig() { return m_WindowConfig; }
        GraphicsConfig& GetGraphicsConfig() { return m_GraphicsConfig; }
        InputConfig& GetInputConfig() { return m_InputConfig; }
        EditorConfig& GetEditorConfig() { return m_EditorConfig; }

        // Key mapping helpers
        // ���� keycode�����δ�ҵ����� -1
        int  GetKeyMapping(const std::string& action) const;
        void SetKeyMapping(const std::string& action, int keycode);
        // �� keycode ����Ϊ�ɶ����ƣ����� 87 -> "W"��
        std::string GetKeyName(int keycode) const;
        // �����û��Ѻ��ַ����� keycode������ "W" -> 87, "Space" -> 32��
        std::optional<int> ParseKeyString(const std::string& keyStr) const;

        // Validate settings and clamp/fix invalid values
        bool Validate();

        // dirty flag: true = modified since last save
        bool IsDirty() const;
        void MarkDirty(bool dirty = true);

        void MarkDirtyInternal(bool dirty);

        void ResetToDefaultsInternal() {
            m_WindowConfig = WindowConfig();
            m_GraphicsConfig = GraphicsConfig();
            m_InputConfig = InputConfig();
            m_EditorConfig = EditorConfig();
            MarkDirtyInternal(true);
        }

    private:
        Config();
        ~Config();

        // ��ֹ����/�ƶ�
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;
        Config(Config&&) = delete;
        Config& operator=(Config&&) = delete;

        void LoadInternal(const std::string& filepath);
        bool SaveInternal(const std::string& filepath);

    private:
        // ����
        WindowConfig m_WindowConfig;
        GraphicsConfig m_GraphicsConfig;
        InputConfig m_InputConfig;
        EditorConfig m_EditorConfig;

        // implementation details hidden in cpp with Pimpl (if needed later)
        struct Impl;
        Impl* m_Impl;
    };

} // namespace Intro
