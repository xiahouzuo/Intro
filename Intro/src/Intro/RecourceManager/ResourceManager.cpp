#include "itrpch.h"
#include "ResourceManager.h"
#include "Intro/Log.h"
#include <filesystem>
#include <fstream>

namespace Intro {

    ResourceManager& ResourceManager::Get() {
        static ResourceManager instance;
        return instance;
    }

    // 替换 ResourceManager::ScanAssetsDirectory 的实现
    void ResourceManager::ScanAssetsDirectory(const std::string& rootPath) {
        // 调试日志：记录调用输入
        ITR_INFO("ScanAssetsDirectory called with rootPath='{}'", rootPath);

        if (!rootPath.empty()) {
            m_AssetsRoot = rootPath;
        }

        // 解析绝对路径并打印（方便定位）
        std::filesystem::path assetsPath(m_AssetsRoot);
        auto absAssetsPath = std::filesystem::absolute(assetsPath);
        bool exists = std::filesystem::exists(absAssetsPath);

        ITR_INFO("ResourceManager: m_AssetsRoot='{}', absolute='{}', exists={}",
            m_AssetsRoot, absAssetsPath.string(), exists ? "true" : "false");

        // 如果上一次扫描的路径与本次相同，则跳过（避免重复覆盖）
        if (!m_LastScannedPath.empty()) {
            std::filesystem::path last(m_LastScannedPath);
            if (std::filesystem::equivalent(last, absAssetsPath)) {
                ITR_INFO("ScanAssetsDirectory: same as last scanned path '{}', skipping rebuild.", absAssetsPath.string());
                return;
            }
        }

        // 如果目录不存在，记录警告并创建（但是不要继续构建空树）
        if (!exists) {
            ITR_WARN("Assets directory does not exist: {} -- creating directories", absAssetsPath.string());
            std::error_code ec;
            std::filesystem::create_directories(absAssetsPath, ec);
            if (ec) {
                ITR_ERROR("Failed to create assets directory '{}': {}", absAssetsPath.string(), ec.message());
            }
            // 更新 last scanned to avoid immediate duplicate build attempts
            m_LastScannedPath = absAssetsPath.string();
            return;
        }

        // 构建根节点（把 path 记录为绝对路径字符串）
        m_FileTreeRoot = std::make_shared<ResourceFileNode>(
            ResourceFileInfo{ "Assets", absAssetsPath.string(), ResourceType::Directory, "", 0, 0, true }
        );

        // 遍历构建子树
        BuildFileTree(absAssetsPath, m_FileTreeRoot);

        ITR_INFO("File tree built with {} root items (path='{}')", m_FileTreeRoot->children.size(), absAssetsPath.string());

        // 记录最后扫描路径
        m_LastScannedPath = absAssetsPath.string();

        if (m_FileTreeUpdatedCallback) {
            m_FileTreeUpdatedCallback();
        }
    }


    void ResourceManager::BuildFileTree(const std::filesystem::path& currentPath,
        std::shared_ptr<ResourceFileNode> parentNode) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
                std::string filename = entry.path().filename().string();
                if (filename.empty() || filename[0] == '.') {
                    continue;
                }

                auto fileInfo = CreateFileInfo(entry.path());
                auto node = std::make_shared<ResourceFileNode>(fileInfo);

                // 直接设置父节点，不使用 AddChild
                node->parent = parentNode;
                parentNode->children.push_back(node);

                if (entry.is_directory()) {
                    BuildFileTree(entry.path(), node);
                }
            }
        }
        catch (const std::exception& e) {
            ITR_ERROR("Error building file tree: {}", e.what());
        }
    }

    ResourceType ResourceManager::DetermineResourceType(const std::filesystem::path& path) {
        if (std::filesystem::is_directory(path)) {
            return ResourceType::Directory;
        }

        std::string extension = path.extension().string();
        for (char& c : extension) c = std::tolower(c);

        if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || extension == ".glb") {
            return ResourceType::Model;
        }

        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga") {
            return ResourceType::Texture;
        }

        if (extension == ".vert" || extension == ".frag" || extension == ".glsl") {
            return ResourceType::Shader;
        }

        if (extension == ".mat") {
            return ResourceType::Material;
        }

        if (extension == ".scene") {
            return ResourceType::Scene;
        }

        return ResourceType::Unknown;
    }

    ResourceFileInfo ResourceManager::CreateFileInfo(const std::filesystem::path& path) {
        ResourceFileInfo info;
        info.name = path.filename().string();
        info.path = path.string();
        info.type = DetermineResourceType(path);
        info.extension = path.extension().string();
        info.isDirectory = std::filesystem::is_directory(path);

        if (!info.isDirectory) {
            try {
                info.fileSize = std::filesystem::file_size(path);
                // 简化：不处理文件时间
                info.lastModified = 0;
            }
            catch (...) {
                // 忽略错误
            }
        }

        return info;
    }

    void ResourceManager::RefreshFileTree() {
        m_LastScannedPath.clear();
        ScanAssetsDirectory(m_AssetsRoot);
    }

    std::shared_ptr<Model> ResourceManager::LoadModelFromNode(std::shared_ptr<ResourceFileNode> node) {
        if (!node || node->info.type != ResourceType::Model) return nullptr;
        if (node->info.model) return node->info.model;

        auto model = LoadModel(node->info.path);
        node->info.model = model;
        return model;
    }

    std::shared_ptr<Texture> ResourceManager::LoadTextureFromNode(std::shared_ptr<ResourceFileNode> node) {
        if (!node || node->info.type != ResourceType::Texture) return nullptr;
        if (node->info.texture) return node->info.texture;

        auto texture = LoadTexture(node->info.path);
        node->info.texture = texture;
        return texture;
    }

    // 原有的兼容方法
    std::shared_ptr<Model> ResourceManager::LoadModel(const std::string& path) {
        std::string fullPath = ResolveAssetPath(path);

        auto it = m_Models.find(fullPath);
        if (it != m_Models.end()) return it->second;

        try {
            if (!std::filesystem::exists(fullPath)) {
                ITR_ERROR("Model file not found: {}", fullPath);
                return nullptr;
            }

            auto model = std::make_shared<Model>(fullPath);
            if (model->GetMeshes().empty()) {
                ITR_ERROR("Failed to load model: {}", fullPath);
                return nullptr;
            }

            m_Models[fullPath] = model;
            return model;
        }
        catch (const std::exception& e) {
            ITR_ERROR("Exception loading model {}: {}", fullPath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& path) {
        std::string fullPath = ResolveAssetPath(path);

        auto it = m_Textures.find(fullPath);
        if (it != m_Textures.end()) return it->second;

        try {
            if (!std::filesystem::exists(fullPath)) {
                ITR_ERROR("Texture file not found: {}", fullPath);
                return nullptr;
            }

            auto texture = std::make_shared<Texture>(fullPath);
            m_Textures[fullPath] = texture;
            return texture;
        }
        catch (const std::exception& e) {
            ITR_ERROR("Exception loading texture {}: {}", fullPath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name,
        const std::string& vertexPath, const std::string& fragmentPath) {
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end()) return it->second;

        try {
            std::string vertexFullPath = ResolveAssetPath(vertexPath);
            std::string fragmentFullPath = ResolveAssetPath(fragmentPath);

            if (!std::filesystem::exists(vertexFullPath) || !std::filesystem::exists(fragmentFullPath)) {
                ITR_ERROR("Shader files not found");
                return nullptr;
            }

            auto shader = std::make_shared<Shader>(vertexFullPath.c_str(), fragmentFullPath.c_str());
            m_Shaders[name] = shader;
            return shader;
        }
        catch (const std::exception& e) {
            ITR_ERROR("Exception loading shader {}: {}", name, e.what());
            return nullptr;
        }
    }

    // 其他原有方法
    std::string ResourceManager::ResolveAssetPath(const std::string& relativePath) const {
        std::filesystem::path path(relativePath);
        if (path.is_absolute()) return relativePath;
        return (std::filesystem::path(m_AssetsRoot) / path).lexically_normal().string();
    }

    void ResourceManager::Initialize() {
        if (m_Initialized) return;
        ITR_INFO("ResourceManager initializing...");
        CreateDefaultResources();

        try {
            ScanAssetsDirectory(m_AssetsRoot);
        }
        catch (const std::exception& e) {
            ITR_ERROR("Failed to scan assets directory during Initialize: {}", e.what());
        }

        m_Initialized = true;
    }

    void ResourceManager::Shutdown() {
        if (!m_Initialized) return;
        ITR_INFO("ResourceManager shutting down...");
        UnloadAllModels();
        UnloadAllTextures();
        UnloadAllShaders();
        RemoveAllMaterials();
        m_DefaultMaterial.reset();
        m_DefaultShader.reset();
        m_Initialized = false;
    }

    void ResourceManager::CreateDefaultResources() {
        m_DefaultShader = LoadShader("default",
            "E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/tempShader.vert",
            "E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/tempShader.frag");

        if (m_DefaultShader) {
            m_DefaultMaterial = CreateMaterial("default", m_DefaultShader);
            if (m_DefaultMaterial) {
                m_DefaultMaterial->SetShininess(32.0f);
                m_DefaultMaterial->SetAmbient(glm::vec3(0.2f));
            }
        }
    }

    std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& name,
        std::shared_ptr<Shader> shader) {
        auto it = m_Materials.find(name);
        if (it != m_Materials.end()) return it->second;

        if (!shader) return nullptr;

        auto material = std::make_shared<Material>(shader);
        m_Materials[name] = material;
        return material;
    }

    std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& name) {
        auto it = m_Materials.find(name);
        return (it != m_Materials.end()) ? it->second : nullptr;
    }

    // 清理方法
    void ResourceManager::UnloadAllModels() { m_Models.clear(); }
    void ResourceManager::UnloadAllTextures() { m_Textures.clear(); }
    void ResourceManager::UnloadAllShaders() { m_Shaders.clear(); }
    void ResourceManager::RemoveAllMaterials() { m_Materials.clear(); }

    void ResourceManager::PrintResourceStats() const {
        ITR_INFO("=== Resource Statistics ===");
        ITR_INFO("Models: {}", m_Models.size());
        ITR_INFO("Textures: {}", m_Textures.size());
        ITR_INFO("Shaders: {}", m_Shaders.size());
        ITR_INFO("Materials: {}", m_Materials.size());
    }

    void ResourceManager::ClearUnusedResources() {
        // 简单实现：清除所有未使用的资源
        size_t modelsCleared = 0, texturesCleared = 0;

        for (auto it = m_Models.begin(); it != m_Models.end(); ) {
            if (it->second.use_count() == 1) {
                it = m_Models.erase(it);
                modelsCleared++;
            }
            else {
                ++it;
            }
        }

        for (auto it = m_Textures.begin(); it != m_Textures.end(); ) {
            if (it->second.use_count() == 1) {
                it = m_Textures.erase(it);
                texturesCleared++;
            }
            else {
                ++it;
            }
        }

        if (modelsCleared > 0 || texturesCleared > 0) {
            ITR_INFO("Cleared {} models and {} textures", modelsCleared, texturesCleared);
        }
    }

} // namespace Intro