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

        // 禁用拷贝和移动
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;

        // 初始化/关闭
        void Initialize();
        void Shutdown();

        // 文件树管理
        void ScanAssetsDirectory(const std::string& rootPath = "");
        std::shared_ptr<ResourceFileNode> GetFileTree() const { return m_FileTreeRoot; }
        void RefreshFileTree();

        // 通过文件树节点获取资源
        std::shared_ptr<Model> LoadModelFromNode(std::shared_ptr<ResourceFileNode> node);
        std::shared_ptr<Texture> LoadTextureFromNode(std::shared_ptr<ResourceFileNode> node);

        // 传统路径方式（保持兼容）
        std::shared_ptr<Model> LoadModel(const std::string& path);
        std::shared_ptr<Texture> LoadTexture(const std::string& path);
        std::shared_ptr<Shader> LoadShader(const std::string& name,
            const std::string& vertexPath,
            const std::string& fragmentPath);

        // 获取资源
        std::shared_ptr<Model> GetModel(const std::string& path);
        std::shared_ptr<Texture> GetTexture(const std::string& path);
        std::shared_ptr<Shader> GetShader(const std::string& name);

        // 材质管理
        std::shared_ptr<Material> CreateMaterial(const std::string& name,
            std::shared_ptr<Shader> shader);
        std::shared_ptr<Material> GetMaterial(const std::string& name);

        // 默认资源
        std::shared_ptr<Shader> GetDefaultShader() const { return m_DefaultShader; }
        std::shared_ptr<Material> GetDefaultMaterial() const { return m_DefaultMaterial; }

        // 资源统计和清理
        void PrintResourceStats() const;
        void ClearUnusedResources();

        // 路径工具
        void SetAssetsRoot(const std::string& path) { m_AssetsRoot = path; }
        std::string ResolveAssetPath(const std::string& relativePath) const;

        // 清理方法
        void UnloadAllModels();
        void UnloadAllTextures();
        void UnloadAllShaders();
        void RemoveAllMaterials();

        // 事件回调
        using FileTreeUpdatedCallback = std::function<void()>;
        void SetFileTreeUpdatedCallback(FileTreeUpdatedCallback callback) {
            m_FileTreeUpdatedCallback = callback;
        }

        // 统计信息
        size_t GetModelCount() const { return m_Models.size(); }
        size_t GetTextureCount() const { return m_Textures.size(); }
        size_t GetShaderCount() const { return m_Shaders.size(); }
        size_t GetMaterialCount() const { return m_Materials.size(); }

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        // 文件树构建
        void BuildFileTree(const std::filesystem::path& currentPath,
            std::shared_ptr<ResourceFileNode> parentNode);
        ResourceType DetermineResourceType(const std::filesystem::path& path);
        ResourceFileInfo CreateFileInfo(const std::filesystem::path& path);

        // 默认资源初始化
        void CreateDefaultResources();

        // 资源存储
        std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;

        // 文件树
        std::shared_ptr<ResourceFileNode> m_FileTreeRoot;
        std::string m_AssetsRoot = "E:/MyEngine/Intro//Intro/src/Intro/assets/";

        // 默认资源
        std::shared_ptr<Shader> m_DefaultShader;
        std::shared_ptr<Material> m_DefaultMaterial;

        // 回调
        FileTreeUpdatedCallback m_FileTreeUpdatedCallback;

        bool m_Initialized = false;
        std::string m_LastScannedPath;
    };

} // namespace Intro