#pragma once
#include "Intro/Core.h"
#include "Intro/Renderer/Mesh.h"
#include <string>
#include <vector>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Intro {

	class ITR_API Model
	{
	public:
		Model(const std::string& modelPath);

		void Draw() const;

		const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return m_Meshes; }
		const std::string& GetPath() const { return m_ModelPath; }

	private:
		std::string m_ModelPath;
		std::vector<std::shared_ptr<Mesh>> m_Meshes;

		// 核心递归函数：处理Assimp的节点（节点可能包含多个Mesh）
		void ProcessNode(aiNode* node, const aiScene* scene);
		// 核心转换函数：将Assimp的aiMesh转换为引擎的Mesh
		std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

}