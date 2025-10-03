#include "itrpch.h"
#include "Model.h"
#include "Intro/Renderer/RendererLayer.h"
#include <iostream>


namespace Intro {

    Model::Model(const std::string& modelPath) : m_ModelPath(modelPath)
    {
        // ʹ�� filesystem ��ȷ����·��
        std::filesystem::path path(modelPath);
        m_Directory = path.parent_path().string();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            modelPath,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenNormals |
            aiProcess_ConvertToLeftHanded
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp���ش���: " << importer.GetErrorString() << std::endl;
            return;
        }

        ProcessNode(scene->mRootNode, scene);
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<std::shared_ptr<Texture>> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            }

            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            std::vector<std::shared_ptr<Texture>> diffuseMaps = LoadMaterialTextures(
                material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<std::shared_ptr<Texture>> specularMaps = LoadMaterialTextures(
                material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return std::make_shared<Mesh>(vertices, indices, textures);
    }

    std::vector<std::shared_ptr<Texture>> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
        std::vector<std::shared_ptr<Texture>> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            std::string texturePath = std::string(str.C_Str());

            // �Ľ���·�������߼�
            std::filesystem::path textureFilePath(texturePath);

            std::string fullPath;

            // ����Ƿ��Ǿ���·��
            if (textureFilePath.is_absolute()) {
                fullPath = texturePath;
            }
            // ����Ƿ��Ѿ������ģ��Ŀ¼��·��
            else if (std::filesystem::exists(m_Directory + "/" + texturePath)) {
                fullPath = m_Directory + "/" + texturePath;
            }
            else {
                // ������ģ��Ŀ¼�в���
                std::filesystem::path modelDir(m_Directory);
                auto potentialPath = modelDir / textureFilePath;
                if (std::filesystem::exists(potentialPath)) {
                    fullPath = potentialPath.string();
                }
                else {
                    // ������Ҳ���������ʹ���ļ�����ģ��Ŀ¼������
                    std::string fileName = textureFilePath.filename().string();
                    auto searchPath = modelDir / fileName;
                    if (std::filesystem::exists(searchPath)) {
                        fullPath = searchPath.string();
                    }
                    else {
                        std::cout << "�޷��ҵ������ļ�: " << texturePath << std::endl;
                        std::cout << "���Ե�·��: " << (modelDir / textureFilePath).string() << std::endl;
                        continue;
                    }
                }
            }

            // ��׼��·�����Ƴ������ ../ ./ �ȣ�
            std::filesystem::path normalizedPath = std::filesystem::path(fullPath).lexically_normal();
            fullPath = normalizedPath.string();

            

            try {
                auto texture = std::make_shared<Texture>(fullPath);
                texture->SetType(typeName);
                textures.push_back(texture);
            }
            catch (const std::exception& e) {
                std::cerr << "�������ʧ��: " << fullPath << " - " << e.what() << std::endl;
            }
        }
        return textures;
    }

    void Model::Draw(Shader& shader) const {
        for (const auto& mesh : m_Meshes) {
            mesh->Draw(shader);
        }
    }
}