#include "itrpch.h"
#include "ScriptRegistry.h"
#include "Intro/Log.h"

namespace Intro {

    ScriptRegistry& ScriptRegistry::Get() {
        static ScriptRegistry instance;
        return instance;
    }

    std::shared_ptr<IScript> ScriptRegistry::CreateScript(const std::string& className) {
        auto it = m_Factories.find(className);
        if (it != m_Factories.end()) {
            return it->second();
        }
        ITR_WARN("Script class not registered: {}", className);
        return nullptr;
    }

    bool ScriptRegistry::IsScriptRegistered(const std::string& className) const {
        return m_Factories.find(className) != m_Factories.end();
    }

    std::vector<std::string> ScriptRegistry::GetRegisteredScripts() const {
        std::vector<std::string> scripts;
        for (const auto& [name, factory] : m_Factories) {
            scripts.push_back(name);
        }
        return scripts;
    }

} // namespace Intro