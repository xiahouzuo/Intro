#include "itrpch.h"
#include "ShaderLibrary.h"

namespace Intro {
    void ShaderLibrary::Add(const std::string& name, const std::shared_ptr<Shader>& shader) {
        ITR_CORE_ASSERT(!Exists(name), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const std::shared_ptr<Shader>& shader) {
        auto& name = "UnnamedShader"; // 可以从着色器文件中提取名称
        Add(name, shader);
    }

    std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& filepath) {
        auto shader = std::make_shared<Shader>(filepath.c_str(), filepath.c_str());
        Add(shader);
        return shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath) {
        auto shader = std::make_shared<Shader>(filepath.c_str(), filepath.c_str());
        Add(name, shader);
        return shader;
    }

    std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name) {
        ITR_CORE_ASSERT(Exists(name), "Shader not found!");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const {
        return m_Shaders.find(name) != m_Shaders.end();
    }

    void ShaderLibrary::ReloadAll() {
        for (auto& [name, shader] : m_Shaders) {
            // 这里可以实现着色器的热重载
            ITR_INFO("Reloading shader: {}", name);
        }
    }
}