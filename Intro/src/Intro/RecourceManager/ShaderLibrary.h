#pragma once

#include "Intro/Core.h"
#include "Intro/Renderer/Shader.h"
#include <unordered_map>
#include <string>
#include <memory>


namespace Intro {

	class ITR_API ShaderLibrary
	{
	public:
		void Add(const std::shared_ptr<Shader>& shader);
		void Add(const std::string& name, const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Shader> Load(const std::string& filepath);
		std::shared_ptr<Shader> Load(const std::string& name, const std::string& filepath);

		std::shared_ptr<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;

		void ReloadAll();

	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	};

}
