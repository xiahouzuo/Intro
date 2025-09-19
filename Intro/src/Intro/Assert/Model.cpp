// src/Intro/Asset/Model.cpp
#include "itrpch.h"
#include "Model.h"
#include "Intro/Renderer/RendererLayer.h"  // 确保能访问OpenGL接口
#include <iostream>

namespace Intro {

    Model::Model(const std::string& modelPath) : m_ModelPath(modelPath) {
        // 1. Assimp导入器初始化 + 后处理（关键！解决模型数据格式问题）
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            modelPath,
            aiProcess_Triangulate       // 强制将模型面转为三角形（OpenGL只认三角形）
            | aiProcess_FlipUVs         // 翻转UV的Y轴（Assimp与OpenGL UV原点不同）
            | aiProcess_GenNormals      // 自动生成法线（如果模型没有）
            | aiProcess_ConvertToLeftHanded  // 转换为左手坐标系（适配OpenGL）
        );

        // 2. 错误检查
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp加载失败: " << importer.GetErrorString() << std::endl;
            return;
        }

        // 3. 递归处理根节点
        ProcessNode(scene->mRootNode, scene);
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        // 先处理当前节点的所有Mesh
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];  // 节点的Mesh索引对应场景的Mesh
            m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        // 再递归处理子节点
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // 1. 转换顶点数据（位置、法线、UV）
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            // 位置：Assimp的aiVector3D → glm::vec3
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            // 法线：如果模型有法线，直接转换；否则Assimp已通过GenNormals生成
            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            }

            // UV：取第一套UV（多数模型只有一套）
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);  // 无UV则设为(0,0)
            }

            vertices.push_back(vertex);
        }

        // 2. 转换索引数据（Assimp的面索引 → 引擎的索引数组）
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // 3. 返回引擎的Mesh对象（用智能指针管理生命周期）
        return std::make_shared<Mesh>(vertices, indices);
    }

    void Model::Draw() const {
        // 遍历所有Mesh并渲染
        for (const auto& mesh : m_Meshes) {
            mesh->Draw();
        }
    }

}