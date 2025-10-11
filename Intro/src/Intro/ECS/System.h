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
            Transform transform;                // 值拷贝（在渲染阶段安全）
            std::shared_ptr<Mesh> mesh;         // 持有资源，保证 Mesh 在渲染期间存活
        };

        // 注意：接受非 const ECS&（需要调用非 const registry API）
        static std::vector<RenderableData> GetRenderables(ECS& ecs) {
            std::vector<RenderableData> result;
            auto& reg = ecs.GetRegistry(); // 非 const

            result.reserve(256);

            // 1) 遍历拥有 TransformComponent 和 MeshComponent 的实体
            // 使用 view.each() 并用结构化绑定取得 entity 与组件引用，
            // 这是 EnTT 上下文中最通用也最清晰的遍历方式。
            auto meshView = reg.view<TransformComponent, MeshComponent>();
            for (auto [entity, tfComp, meshComp] : meshView.each()) {
                // tfComp, meshComp 是引用（或 proxy 引用），直接使用
                if (meshComp.mesh) {
                    result.push_back({ tfComp.transform, meshComp.mesh });
                }
            }

            // 2) 遍历拥有 TransformComponent 和 ModelComponent 的实体
            auto modelView = reg.view<TransformComponent, ModelComponent>();
            for (auto [entity, tfComp, modelComp] : modelView.each()) {
                if (modelComp.model) {
                    // 假设 Model::GetMeshes() 返回容器 of shared_ptr<Mesh>
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
                item.transform = tf.transform.GetModelMatrix(); // 假设Transform有GetMatrix方法
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
