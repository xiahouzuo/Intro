// src/Intro/Asset/Model.cpp
#include "itrpch.h"
#include "Model.h"
#include "Intro/Renderer/RendererLayer.h"  // ȷ���ܷ���OpenGL�ӿ�
#include <iostream>

namespace Intro {

    Model::Model(const std::string& modelPath) : m_ModelPath(modelPath) {
        // 1. Assimp��������ʼ�� + �����ؼ������ģ�����ݸ�ʽ���⣩
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            modelPath,
            aiProcess_Triangulate       // ǿ�ƽ�ģ����תΪ�����Σ�OpenGLֻ�������Σ�
            | aiProcess_FlipUVs         // ��תUV��Y�ᣨAssimp��OpenGL UVԭ�㲻ͬ��
            | aiProcess_GenNormals      // �Զ����ɷ��ߣ����ģ��û�У�
            | aiProcess_ConvertToLeftHanded  // ת��Ϊ��������ϵ������OpenGL��
        );

        // 2. ������
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp����ʧ��: " << importer.GetErrorString() << std::endl;
            return;
        }

        // 3. �ݹ鴦����ڵ�
        ProcessNode(scene->mRootNode, scene);
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        // �ȴ���ǰ�ڵ������Mesh
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];  // �ڵ��Mesh������Ӧ������Mesh
            m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        // �ٵݹ鴦���ӽڵ�
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 1. ת���������ݣ�λ�á����ߡ�UV��
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            // λ�ã�Assimp��aiVector3D �� glm::vec3
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            // ���ߣ����ģ���з��ߣ�ֱ��ת��������Assimp��ͨ��GenNormals����
            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            }

            // UV��ȡ��һ��UV������ģ��ֻ��һ�ף�
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);  // ��UV����Ϊ(0,0)
            }

            vertices.push_back(vertex);
        }

        // 2. ת���������ݣ�Assimp�������� �� ������������飩
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // 3. ���������Mesh����������ָ������������ڣ�
        return std::make_shared<Mesh>(vertices, indices);
    }

    void Model::Draw() const {
        // ��������Mesh����Ⱦ
        for (const auto& mesh : m_Meshes) {
            mesh->Draw();
        }
    }

}