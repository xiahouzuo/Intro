#pragma once
#pragma once
#include "Intro/Core.h"
#include "IScript.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace Intro {

    class ITR_API ScriptRegistry {
    public:
        using ScriptFactory = std::function<std::shared_ptr<IScript>()>;

        static ScriptRegistry& Get();

        // 注册脚本类
        template<typename T>
        void RegisterScript(const std::string& className) {
            static_assert(std::is_base_of_v<IScript, T>, "T must derive from IScript");
            m_Factories[className] = []() -> std::shared_ptr<IScript> {
                return std::make_shared<T>();
                };
        }

        // 创建脚本实例
        std::shared_ptr<IScript> CreateScript(const std::string& className);

        // 检查脚本类是否已注册
        bool IsScriptRegistered(const std::string& className) const;

        // 获取所有已注册的脚本类名
        std::vector<std::string> GetRegisteredScripts() const;

    private:
        ScriptRegistry() = default;
        std::unordered_map<std::string, ScriptFactory> m_Factories;
    };

} // namespace Intro