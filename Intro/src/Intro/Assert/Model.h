#pragma once
#include "Intro/Core.h"
#include "Intro/Renderer/Mesh.h"
#include "Intro/Renderer/Texture.h"
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

        void Draw(Shader& shader) const;

        const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return m_Meshes; }
        const std::string& GetPath() const { return m_ModelPath; }

    private:
        std::string m_ModelPath;
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
        std::string m_Directory;

        void ProcessNode(aiNode* node, const aiScene* scene);
        std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };

}