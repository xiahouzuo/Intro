// Config.cpp
#include "itrpch.h"
#include "Config.h"
#include "Intro/Log.h"      // ITR_INFO/WARN/ERROR
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <mutex>
#include <chrono>

using json = nlohmann::json;

namespace Intro {

    // ------------------------------
    // Impl definition (Pimpl-ish)
    // ------------------------------
    struct Config::Impl {
        std::string CurrentFilepath;
        bool IsDirty = false;
        std::mutex Mutex; // protect save/load and IsDirty
        int ConfigVersion = 1;
    };

    // ------------------------------
    // Utility helpers
    // ------------------------------
    static std::string ToLower(std::string s) {
        for (char& c : s) c = static_cast<char>(::tolower(c));
        return s;
    }

    // Returns a good default path (user directory) for config
    static std::string GetUserConfigPath() {
        try {
#ifdef _WIN32
            char* appdata = std::getenv("APPDATA");
            std::string base = (appdata ? appdata : ".");
            std::filesystem::path p = std::filesystem::path(base) / "IntroEngine";
#elif defined(__APPLE__)
            char* home = std::getenv("HOME");
            std::string base = (home ? home : ".");
            std::filesystem::path p = std::filesystem::path(base) / "Library" / "Application Support" / "IntroEngine";
#else // linux / unix
            char* home = std::getenv("HOME");
            std::string base = (home ? home : ".");
            std::filesystem::path p = std::filesystem::path(base) / ".config" / "introengine";
#endif
            std::filesystem::create_directories(p);
            p /= "config.json";
            return p.string();
        }
        catch (...) {
            return std::string("config.json");
        }
    }

    // If working directory has config file, prefer it (developer convenience)
    static std::string GetDefaultConfigPathPreferCwd() {
        std::string cwd = std::filesystem::current_path().string();
        std::filesystem::path p = std::filesystem::path(cwd) / "config.json";
        if (std::filesystem::exists(p)) return p.string();
        return GetUserConfigPath();
    }

    // Atomic write: write to tmp then rename
    static bool AtomicWriteFile(const std::string& path, const std::string& data) {
        try {
            std::filesystem::path p(path);
            std::filesystem::path dir = p.parent_path();
            if (!dir.empty())
                std::filesystem::create_directories(dir);

            std::string tmp = path + ".tmp";
            {
                std::ofstream ofs(tmp, std::ios::binary);
                if (!ofs.is_open()) return false;
                ofs << data;
                ofs.flush();
                if (!ofs.good()) return false;
            }

            std::error_code ec;
            std::filesystem::rename(tmp, path, ec);
            if (ec) {
                // fallback: try remove+rename
                std::filesystem::remove(path, ec);
                if (!ec) {
                    std::filesystem::rename(tmp, path, ec);
                }
            }
            return !ec;
        }
        catch (...) {
            return false;
        }
    }

    // Backup corrupt file
    static void BackupCorruptFile(const std::string& path) {
        try {
            std::string stamp = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::filesystem::path p(path);
            std::filesystem::path dest = p;
            dest += ".corrupt." + stamp;
            std::error_code ec;
            std::filesystem::rename(p, dest, ec);
            if (!ec) {
                ITR_WARN("Moved corrupt config '{}' to '{}'", path, dest.string());
            }
            else {
                ITR_WARN("Failed to move corrupt config '{}'", path);
            }
        }
        catch (...) {
            // ignore
        }
    }

    // Simple key name -> keycode parse support
    // Supports: single character ("W","a"), digits ("1"), named keys like "Space","Enter","Escape","LeftShift" etc.
    static std::optional<int> ParseKeyStringToCode(const std::string& s) {
        if (s.empty()) return std::nullopt;
        std::string t = ToLower(s);

        // single char letters and digits
        if (t.size() == 1) {
            char c = t[0];
            if (c >= 'a' && c <= 'z') return static_cast<int>(std::toupper(c));
            if (c >= '0' && c <= '9') return static_cast<int>(c);
        }

        // some common names
        if (t == "space") return 32;
        if (t == "enter" || t == "return") return 13;
        if (t == "escape" || t == "esc") return 27;
        if (t == "backspace") return 8;
        if (t == "tab") return 9;
        if (t == "leftshift" || t == "shift") return 340; // example GLFW-like
        if (t == "leftctrl" || t == "lctrl" || t == "ctrl") return 341;
        if (t == "leftalt" || t == "alt") return 342;
        if (t == "up") return 265;
        if (t == "down") return 264;
        if (t == "left") return 263;
        if (t == "right") return 262;
        if (t == "f1") return 290;
        if (t == "f2") return 291;
        // ... you can extend this table with GLFW keycodes or your engine key codes

        // numeric string to code
        bool allDigit = true;
        for (char c : t) if (!std::isdigit((unsigned char)c)) { allDigit = false; break; }
        if (allDigit) {
            try { return std::stoi(t); }
            catch (...) {}
        }
        return std::nullopt;
    }

    // Inverse: some keycode to readable name
    static std::string KeyCodeToName(int code) {
        if (code >= 'A' && code <= 'Z') {
            std::string s; s.push_back(static_cast<char>(code)); return s;
        }
        if (code >= '0' && code <= '9') {
            std::string s; s.push_back(static_cast<char>(code)); return s;
        }
        switch (code) {
        case 32: return "Space";
        case 13: return "Enter";
        case 27: return "Escape";
        case 8:  return "Backspace";
        case 9:  return "Tab";
        case 340: return "LeftShift";
        case 341: return "LeftCtrl";
        case 342: return "LeftAlt";
        case 265: return "Up";
        case 264: return "Down";
        case 263: return "Left";
        case 262: return "Right";
            // add more as needed
        default: return std::to_string(code);
        }
    }

    // ------------------------------
    // Config implementation
    // ------------------------------
    Config& Config::Get() {
        static Config instance;
        return instance;
    }

    Config::Config()
        : m_Impl(new Impl())
    {
        // defaults already set by member initializers
    }

    Config::~Config() {
        // ensure save on destruction? no automatic save here to avoid surprising writes
        delete m_Impl;
        m_Impl = nullptr;
    }

    bool Config::IsDirty() const {
        std::lock_guard<std::mutex> lock(m_Impl->Mutex);
        return m_Impl->IsDirty;
    }

    void Config::MarkDirty(bool dirty) {
        std::lock_guard<std::mutex> lock(m_Impl->Mutex);
        m_Impl->IsDirty = dirty;
    }

    int Config::GetKeyMapping(const std::string& action) const {
        auto it = m_InputConfig.ActionMappings.find(action);
        if (it != m_InputConfig.ActionMappings.end()) return it->second;
        return -1;
    }

    void Config::SetKeyMapping(const std::string& action, int keycode) {
        m_InputConfig.ActionMappings[action] = keycode;
        MarkDirty(true);
    }

    std::string Config::GetKeyName(int keycode) const {
        return KeyCodeToName(keycode);
    }

    std::optional<int> Config::ParseKeyString(const std::string& keyStr) const {
        return ParseKeyStringToCode(keyStr);
    }

    bool Config::Validate() {
        bool valid = true;

        // window size sane bounds
        if (m_WindowConfig.Width < 640) { ITR_WARN("Window width too small ({}), clamping to 640", m_WindowConfig.Width); m_WindowConfig.Width = 640; valid = false; }
        if (m_WindowConfig.Height < 480) { ITR_WARN("Window height too small ({}), clamping to 480", m_WindowConfig.Height); m_WindowConfig.Height = 480; valid = false; }
        if (m_WindowConfig.Width > 10000) { ITR_WARN("Window width too large ({}), clamping to 10000", m_WindowConfig.Width); m_WindowConfig.Width = 10000; valid = false; }
        if (m_WindowConfig.Height > 10000) { ITR_WARN("Window height too large ({}), clamping to 10000", m_WindowConfig.Height); m_WindowConfig.Height = 10000; valid = false; }

        // gamma/exposure
        if (m_GraphicsConfig.Gamma < 1.0f || m_GraphicsConfig.Gamma > 5.0f) {
            ITR_WARN("Gamma out of range ({}), clamping to 1.0-5.0", m_GraphicsConfig.Gamma);
            m_GraphicsConfig.Gamma = glm::clamp(m_GraphicsConfig.Gamma, 1.0f, 5.0f);
            valid = false;
        }
        if (m_GraphicsConfig.Exposure < 0.01f || m_GraphicsConfig.Exposure > 10.0f) {
            ITR_WARN("Exposure out of range ({}), clamping to 0.01-10", m_GraphicsConfig.Exposure);
            m_GraphicsConfig.Exposure = glm::clamp(m_GraphicsConfig.Exposure, 0.01f, 10.0f);
            valid = false;
        }

        // mouse sensitivity
        if (m_InputConfig.MouseSensitivity < 0.001f) { m_InputConfig.MouseSensitivity = 0.001f; valid = false; }
        if (m_InputConfig.MouseSensitivity > 10.0f) { m_InputConfig.MouseSensitivity = 10.0f; valid = false; }

        return valid;
    }

    void Config::MarkDirtyInternal(bool dirty) {
        m_Impl->IsDirty = dirty;
    }

    void Config::ResetToDefaults() {
        std::lock_guard<std::mutex> lock(m_Impl->Mutex);

        m_WindowConfig = WindowConfig();
        m_GraphicsConfig = GraphicsConfig();
        m_InputConfig = InputConfig();
        m_EditorConfig = EditorConfig();
        m_Impl->IsDirty = true;
    }

    bool Config::Load(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(m_Impl->Mutex);

        std::string path = filepath.empty() ? GetDefaultConfigPathPreferCwd() : filepath;
        m_Impl->CurrentFilepath = path;

        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            ITR_WARN("Config file '{}' not found - using defaults", path);
            
            ResetToDefaultsInternal();  // 使用内部方法，不重复加锁

            return false;
        }

        try {
            json j;
            ifs >> j;

            // optional versioning
            if (j.contains("ConfigVersion")) {
                m_Impl->ConfigVersion = j["ConfigVersion"].get<int>();
            }

            if (j.contains("Window")) {
                auto& w = j["Window"];
                m_WindowConfig.Width = w.value("Width", m_WindowConfig.Width);
                m_WindowConfig.Height = w.value("Height", m_WindowConfig.Height);
                m_WindowConfig.Fullscreen = w.value("Fullscreen", m_WindowConfig.Fullscreen);
                m_WindowConfig.VSync = w.value("VSync", m_WindowConfig.VSync);
                m_WindowConfig.RefreshRate = w.value("RefreshRate", m_WindowConfig.RefreshRate);
                m_WindowConfig.Title = w.value("Title", m_WindowConfig.Title);
            }

            if (j.contains("Graphics")) {
                auto& g = j["Graphics"];
                int quality = g.value("Quality", static_cast<int>(m_GraphicsConfig.Quality));
                m_GraphicsConfig.Quality = static_cast<GraphicsQuality>(quality);
                int aa = g.value("AntiAliasing", static_cast<int>(m_GraphicsConfig.AA));
                m_GraphicsConfig.AA = static_cast<AntiAliasing>(aa);
                m_GraphicsConfig.Shadows = g.value("Shadows", m_GraphicsConfig.Shadows);
                m_GraphicsConfig.Bloom = g.value("Bloom", m_GraphicsConfig.Bloom);
                m_GraphicsConfig.SSAO = g.value("SSAO", m_GraphicsConfig.SSAO);
                m_GraphicsConfig.Gamma = g.value("Gamma", m_GraphicsConfig.Gamma);
                m_GraphicsConfig.Exposure = g.value("Exposure", m_GraphicsConfig.Exposure);
                m_GraphicsConfig.MaxFPS = g.value("MaxFPS", m_GraphicsConfig.MaxFPS);

                m_GraphicsConfig.EnableMSAA = g.value("EnableMSAA", m_GraphicsConfig.EnableMSAA);
                m_GraphicsConfig.MSaaSamples = g.value("MSaaSamples", m_GraphicsConfig.MSaaSamples);
                m_GraphicsConfig.EnableHDR = g.value("EnableHDR", m_GraphicsConfig.EnableHDR);
                m_GraphicsConfig.EnableGammaCorrection = g.value("EnableGammaCorrection", m_GraphicsConfig.EnableGammaCorrection);
                m_GraphicsConfig.ViewportWidth = g.value("ViewportWidth", m_GraphicsConfig.ViewportWidth);
                m_GraphicsConfig.ViewportHeight = g.value("ViewportHeight", m_GraphicsConfig.ViewportHeight);

                // 后期处理配置
                m_GraphicsConfig.EnablePostProcessing = g.value("EnablePostProcessing", m_GraphicsConfig.EnablePostProcessing);
                m_GraphicsConfig.BloomThreshold = g.value("BloomThreshold", m_GraphicsConfig.BloomThreshold);
                m_GraphicsConfig.BloomIntensity = g.value("BloomIntensity", m_GraphicsConfig.BloomIntensity);
                m_GraphicsConfig.VignetteIntensity = g.value("VignetteIntensity", m_GraphicsConfig.VignetteIntensity);
                m_GraphicsConfig.ChromaticAberration = g.value("ChromaticAberration", m_GraphicsConfig.ChromaticAberration);
            
            }

            if (j.contains("Input")) {
                auto& in = j["Input"];
                m_InputConfig.MouseSensitivity = in.value("MouseSensitivity", m_InputConfig.MouseSensitivity);
                m_InputConfig.InvertMouseY = in.value("InvertMouseY", m_InputConfig.InvertMouseY);

                if (in.contains("ActionMappings")) {
                    m_InputConfig.ActionMappings.clear();
                    for (auto& it : in["ActionMappings"].items()) {
                        const std::string action = it.key();
                        const json& val = it.value();
                        // allow either number or string
                        if (val.is_number_integer()) {
                            m_InputConfig.ActionMappings[action] = val.get<int>();
                        }
                        else if (val.is_string()) {
                            std::string s = val.get<std::string>();
                            auto code = ParseKeyStringToCode(s);
                            if (code) m_InputConfig.ActionMappings[action] = *code;
                            else {
                                ITR_WARN("Unknown key string '{}' for action '{}', skipping", s, action);
                            }
                        }
                    }
                }
            }

            if (j.contains("Editor")) {
                auto& ed = j["Editor"];
                m_EditorConfig.ShowGrid = ed.value("ShowGrid", m_EditorConfig.ShowGrid);
                m_EditorConfig.ShowGizmos = ed.value("ShowGizmos", m_EditorConfig.ShowGizmos);
                m_EditorConfig.GridSize = ed.value("GridSize", m_EditorConfig.GridSize);
                if (ed.contains("GridColor") && ed["GridColor"].is_array() && ed["GridColor"].size() >= 3) {
                    auto& c = ed["GridColor"];
                    m_EditorConfig.GridColor = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());
                }
                if (ed.contains("BackgroundColor") && ed["BackgroundColor"].is_array() && ed["BackgroundColor"].size() >= 3) {
                    auto& c = ed["BackgroundColor"];
                    m_EditorConfig.BackgroundColor = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());
                }
                m_EditorConfig.AutoSave = ed.value("AutoSave", m_EditorConfig.AutoSave);
                m_EditorConfig.AutoSaveInterval = ed.value("AutoSaveInterval", m_EditorConfig.AutoSaveInterval);
            }

            // validate after load
            Validate();

            ITR_INFO("Config loaded from '{}'", path);
            m_Impl->IsDirty = false;
            return true;
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed parsing config '{}' : {}", path, e.what());
            // backup corrupt file
            BackupCorruptFile(path);
            ResetToDefaultsInternal();  // 使用内部方法
            return false;
        }
    }

    bool Config::Save(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(m_Impl->Mutex);
        return SaveInternal(filepath);
    }

    // Config.cpp - 完整的 SaveInternal 实现
    bool Config::SaveInternal(const std::string& filepath) {
        // 实际的保存逻辑，假设已经持有锁
        std::string path = filepath.empty() ?
            (m_Impl->CurrentFilepath.empty() ? GetDefaultConfigPathPreferCwd() : m_Impl->CurrentFilepath)
            : filepath;
        m_Impl->CurrentFilepath = path;

        try {
            json j;
            j["ConfigVersion"] = m_Impl->ConfigVersion;

            // Window 配置
            j["Window"] = {
                {"Width", m_WindowConfig.Width},
                {"Height", m_WindowConfig.Height},
                {"Fullscreen", m_WindowConfig.Fullscreen},
                {"VSync", m_WindowConfig.VSync},
                {"RefreshRate", m_WindowConfig.RefreshRate},
                {"Title", m_WindowConfig.Title}
            };

            // Graphics 配置
            j["Graphics"] = {
                {"Quality", static_cast<int>(m_GraphicsConfig.Quality)},
                {"AntiAliasing", static_cast<int>(m_GraphicsConfig.AA)},
                {"Shadows", m_GraphicsConfig.Shadows},
                {"Bloom", m_GraphicsConfig.Bloom},
                {"SSAO", m_GraphicsConfig.SSAO},
                {"Gamma", m_GraphicsConfig.Gamma},
                {"Exposure", m_GraphicsConfig.Exposure},
                {"MaxFPS", m_GraphicsConfig.MaxFPS},
                // 新增渲染器配置字段
                {"EnableMSAA", m_GraphicsConfig.EnableMSAA},
                {"MSaaSamples", m_GraphicsConfig.MSaaSamples},
                {"EnableHDR", m_GraphicsConfig.EnableHDR},
                {"EnableGammaCorrection", m_GraphicsConfig.EnableGammaCorrection},
                {"ViewportWidth", m_GraphicsConfig.ViewportWidth},
                {"ViewportHeight", m_GraphicsConfig.ViewportHeight},
                // 后期处理配置
                {"EnablePostProcessing", m_GraphicsConfig.EnablePostProcessing},
                {"BloomThreshold", m_GraphicsConfig.BloomThreshold},
                {"BloomIntensity", m_GraphicsConfig.BloomIntensity},
                {"VignetteIntensity", m_GraphicsConfig.VignetteIntensity},
                {"ChromaticAberration", m_GraphicsConfig.ChromaticAberration}
            };

            // Input 映射：尽可能使用键名
            json actionMap = json::object();
            for (auto& kv : m_InputConfig.ActionMappings) {
                const std::string& action = kv.first;
                int keycode = kv.second;
                // 优先使用友好名称表示字母/数字/已知键，否则写数字
                std::string name = KeyCodeToName(keycode);
                actionMap[action] = name;
            }
            j["Input"] = {
                {"MouseSensitivity", m_InputConfig.MouseSensitivity},
                {"InvertMouseY", m_InputConfig.InvertMouseY},
                {"ActionMappings", actionMap}
            };

            // Editor 配置
            j["Editor"] = {
                {"ShowGrid", m_EditorConfig.ShowGrid},
                {"ShowGizmos", m_EditorConfig.ShowGizmos},
                {"GridSize", m_EditorConfig.GridSize},
                {"GridColor", {m_EditorConfig.GridColor.r, m_EditorConfig.GridColor.g, m_EditorConfig.GridColor.b}},
                {"BackgroundColor", {m_EditorConfig.BackgroundColor.r, m_EditorConfig.BackgroundColor.g, m_EditorConfig.BackgroundColor.b}},
                {"AutoSave", m_EditorConfig.AutoSave},
                {"AutoSaveInterval", m_EditorConfig.AutoSaveInterval}
            };

            std::string out = j.dump(4);

            if (!AtomicWriteFile(path, out)) {
                ITR_ERROR("Failed to write config to '{}'", path);
                return false;
            }

            ITR_INFO("Config saved to '{}'", path);
            m_Impl->IsDirty = false;
            return true;
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed to serialize config: {}", e.what());
            return false;
        }
    }

} // namespace Intro
