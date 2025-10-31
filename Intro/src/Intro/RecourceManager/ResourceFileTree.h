// File: ResourceFileTree.h
#pragma once
#include "Intro/Core.h"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace Intro {

    enum class ResourceType {
        Unknown,
        Directory,
        Model,
        Texture,
        Shader,
        Material,
        Scene
    };

    struct ResourceFileInfo {
        std::string name;
        std::string path;
        ResourceType type;
        std::string extension;
        uint64_t fileSize = 0;
        std::time_t lastModified = 0;
        bool isDirectory = false;

        std::shared_ptr<class Model> model;
        std::shared_ptr<class Texture> texture;
        std::shared_ptr<class Shader> shader;
        std::shared_ptr<class Material> material;
    };

    class ITR_API ResourceFileNode {
    public:
        ResourceFileInfo info;
        std::vector<std::shared_ptr<ResourceFileNode>> children;
        std::weak_ptr<ResourceFileNode> parent;

        ResourceFileNode(const ResourceFileInfo& fileInfo) : info(fileInfo) {}

        bool HasChildren() const { return !children.empty(); }
    };

} // namespace Intro