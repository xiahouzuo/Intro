// ECS/Components.h
#pragma once
#include "Intro/Math/Transform.h" // ��� Transform ��
#include "Intro/Renderer/Mesh.h"  // ��� Mesh ��
#include "Intro/Renderer/Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

namespace Intro {

    
    class Model;

    // �任���������ʵ�嶼������
    struct TransformComponent {
        Transform transform; // ֱ���������е� Transform ��

        TransformComponent(
            const glm::vec3& pos = glm::vec3(0.0f),
            const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            const glm::vec3& scale = glm::vec3(1.0f))
            : transform(pos, rot, scale) {
        }
    };


    // ���������ֻ����Ҫ����Ⱦ��ʵ�����
    struct MeshComponent {
        std::shared_ptr<Mesh> mesh; // ���� Mesh ������ָ��

        MeshComponent(std::shared_ptr<Mesh> meshPtr = nullptr)
            : mesh(std::move(meshPtr)) {
        }
    };

    // ģ��������������Ҫ�����ⲿģ�ͣ���ѡ��
    struct ModelComponent {
        std::shared_ptr<Model> model; // ���� Model ������ָ��

        ModelComponent(std::shared_ptr<Model> modelPtr = nullptr)
            : model(std::move(modelPtr)) {
        }
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {
        }
    };

} // namespace Intro