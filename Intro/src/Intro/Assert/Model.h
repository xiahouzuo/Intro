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

		// ���ĵݹ麯��������Assimp�Ľڵ㣨�ڵ���ܰ������Mesh��
		void ProcessNode(aiNode* node, const aiScene* scene);
		// ����ת����������Assimp��aiMeshת��Ϊ�����Mesh
		std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

}