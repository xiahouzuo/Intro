// ECS/Systems.h
#pragma once

#include "ECS.h"
#include "Components.h"
#include "Intro/Renderer/RenderQueue.h"
#include <vector>
#include <memory>
#include "Intro/Core.h"

namespace Intro {

    class ITR_API RenderSystem {
    public:
        struct RenderableData {
            Transform transform;                // ֵ����������Ⱦ�׶ΰ�ȫ��
            std::shared_ptr<Mesh> mesh;         // ������Դ����֤ Mesh ����Ⱦ�ڼ���
        };

        // ע�⣺���ܷ� const ECS&����Ҫ���÷� const registry API��
        static std::vector<RenderableData> GetRenderables(ECS& ecs) {
            std::vector<RenderableData> result;
            auto& reg = ecs.GetRegistry(); // �� const

            result.reserve(256);

            // 1) ����ӵ�� TransformComponent �� MeshComponent ��ʵ��
            // ʹ�� view.each() ���ýṹ����ȡ�� entity ��������ã�
            // ���� EnTT ����������ͨ��Ҳ�������ı�����ʽ��
            auto meshView = reg.view<TransformComponent, MeshComponent>();
            for (auto [entity, tfComp, meshComp] : meshView.each()) {
                // tfComp, meshComp �����ã��� proxy ���ã���ֱ��ʹ��
                if (meshComp.mesh) {
                    result.push_back({ tfComp.transform, meshComp.mesh });
                }
            }

            // 2) ����ӵ�� TransformComponent �� ModelComponent ��ʵ��
            auto modelView = reg.view<TransformComponent, ModelComponent>();
            for (auto [entity, tfComp, modelComp] : modelView.each()) {
                if (modelComp.model) {
                    // ���� Model::GetMeshes() �������� of shared_ptr<Mesh>
                    for (const auto& meshPtr : modelComp.model->GetMeshes()) {
                        if (meshPtr) {
                            result.push_back({ tfComp.transform, meshPtr });
                        }
                    }
                }
            }

            return result;
        }

        static void CollectRenderables(ECS& ecs, RenderQueue& queue, const glm::vec3& cameraPos) {
            auto meshView = ecs.GetRegistry().view<TransformComponent, MeshComponent, MaterialComponent>();
            for (auto [entity, tf, meshComp, matComp] : meshView.each()) {
                if (!meshComp.mesh || !matComp.material) continue;
                RenderItem item;
                item.mesh = meshComp.mesh;
                item.material = matComp.material;
                item.transform = tf.transform.GetModelMatrix(); // ����Transform��GetMatrix����
                item.transparent = matComp.Transparent;
                if (item.transparent)
                    queue.transparent.push_back(item);
                else
                    queue.opaque.push_back(item);
            }


            auto modelView = ecs.GetRegistry().view<TransformComponent, ModelComponent, MaterialComponent>();
            for (auto [entity, tf, modelComp, matComp] : modelView.each()) {
                if (!modelComp.model || !matComp.material) continue;
                for (const auto& meshPtr : modelComp.model->GetMeshes()) {
                    RenderItem item;
                    item.mesh = meshPtr;
                    item.material = matComp.material;
                    item.transform = tf.transform.GetModelMatrix();
                    item.transparent = matComp.Transparent;
                    if (item.transparent)
                        queue.transparent.push_back(item);
                    else
                        queue.opaque.push_back(item);
                }
            }
        }
    };

} // namespace Intro
