// File: ResourceManager.h
#pragma once

#include "Intro/Core.h"
#include "Intro/Renderer/Model.h"
#include "Intro/Renderer/Shader.h"
#include "Intro/Renderer/Texture.h"
#include "Intro/Renderer/Material.h"
#include "ResourceFileTree.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <filesystem>
#include <functional>

namespace Intro {

    class ITR_API ResourceManager
    {
    public:
        static ResourceManager& Get();

        // ���ÿ������ƶ�
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;

        // ��ʼ��/�ر�
        void Initialize();
        void Shutdown();

        // �ļ�������
        void ScanAssetsDirectory(const std::string& rootPath = "");
        std::shared_ptr<ResourceFileNode> GetFileTree() const { return m_FileTreeRoot; }
        void RefreshFileTree();

        // ͨ���ļ����ڵ��ȡ��Դ
        std::shared_ptr<Model> LoadModelFromNode(std::shared_ptr<ResourceFileNode> node);
        std::shared_ptr<Texture> LoadTextureFromNode(std::shared_ptr<ResourceFileNode> node);

        // ��ͳ·����ʽ�����ּ��ݣ�
        std::shared_ptr<Model> LoadModel(const std::string& path);
        std::shared_ptr<Texture> LoadTexture(const std::string& path);
        std::shared_ptr<Shader> LoadShader(const std::string& name,
            const std::string& vertexPath,
            const std::string& fragmentPath);

        // ��ȡ��Դ
        std::shared_ptr<Model> GetModel(const std::string& path);
        std::shared_ptr<Texture> GetTexture(const std::string& path);
        std::shared_ptr<Shader> GetShader(const std::string& name);

        // ���ʹ���
        std::shared_ptr<Material> CreateMaterial(const std::string& name,
            std::shared_ptr<Shader> shader);
        std::shared_ptr<Material> GetMaterial(const std::string& name);

        // Ĭ����Դ
        std::shared_ptr<Shader> GetDefaultShader() const { return m_DefaultShader; }
        std::shared_ptr<Material> GetDefaultMaterial() const { return m_DefaultMaterial; }

        // ��Դͳ�ƺ�����
        void PrintResourceStats() const;
        void ClearUnusedResources();

        // ·������
        void SetAssetsRoot(const std::string& path) { m_AssetsRoot = path; }
        std::string ResolveAssetPath(const std::string& relativePath) const;

        // ������
        void UnloadAllModels();
        void UnloadAllTextures();
        void UnloadAllShaders();
        void RemoveAllMaterials();

        // �¼��ص�
        using FileTreeUpdatedCallback = std::function<void()>;
        void SetFileTreeUpdatedCallback(FileTreeUpdatedCallback callback) {
            m_FileTreeUpdatedCallback = callback;
        }

        // ͳ����Ϣ
        size_t GetModelCount() const { return m_Models.size(); }
        size_t GetTextureCount() const { return m_Textures.size(); }
        size_t GetShaderCount() const { return m_Shaders.size(); }
        size_t GetMaterialCount() const { return m_Materials.size(); }

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        // �ļ�������
        void BuildFileTree(const std::filesystem::path& currentPath,
            std::shared_ptr<ResourceFileNode> parentNode);
        ResourceType DetermineResourceType(const std::filesystem::path& path);
        ResourceFileInfo CreateFileInfo(const std::filesystem::path& path);

        // Ĭ����Դ��ʼ��
        void CreateDefaultResources();

        // ��Դ�洢
        std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;

        // �ļ���
        std::shared_ptr<ResourceFileNode> m_FileTreeRoot;
        std::string m_AssetsRoot = "E:/MyEngine/Intro//Intro/src/Intro/assets/";

        // Ĭ����Դ
        std::shared_ptr<Shader> m_DefaultShader;
        std::shared_ptr<Material> m_DefaultMaterial;

        // �ص�
        FileTreeUpdatedCallback m_FileTreeUpdatedCallback;

        bool m_Initialized = false;
        std::string m_LastScannedPath;
    };

} // namespace Intro