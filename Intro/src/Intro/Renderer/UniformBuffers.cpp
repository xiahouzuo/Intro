#include "itrpch.h"
#include "UniformBuffers.h"
#include "Intro/ECS/ECS.h"
#include "Intro/Math/Transform.h" // 假设Transform有GetPosition和GetRotation方法

namespace Intro {
    void LightsUBO::OnUpdate(ECS& ecs) {
        LightsUBOData data{};
        auto view = ecs.GetRegistry().view<TransformComponent, LightComponent>();

        for (auto [entity, tf, light] : view.each()) {
            switch (light.Type) {
            case LightType::Directional:
                if (data.numDir < MAX_DIR_LIGHTS) {
                    auto& dirLight = data.dirLights[data.numDir];
                    dirLight.direction = glm::vec4(
                        tf.transform.rotation * light.Direction, 0.0f
                    );
                    dirLight.color = glm::vec4(light.Color, light.Intensity);
                    data.numDir++;
                }
                break;
            case LightType::Point:
                if (data.numPoint < MAX_POINT_LIGHTS) {
                    auto& pointLight = data.pointLights[data.numPoint];
                    pointLight.position = glm::vec4(
                        tf.transform.position, light.Range
                    );
                    pointLight.color = glm::vec4(light.Color, light.Intensity);
                    data.numPoint++;
                }
                break;
            case LightType::Spot:
                if (data.numSpot < MAX_SPOT_LIGHTS) {
                    auto& spotLight = data.spotLights[data.numSpot];
                    spotLight.position = glm::vec4(
                        tf.transform.position, light.Range
                    );
                    spotLight.direction = glm::vec4(
                        tf.transform.rotation * light.Direction,
                        glm::cos(glm::radians(light.InnerSpotAngle))
                    );
                    spotLight.color = glm::vec4(light.Color, light.Intensity);
                    spotLight.params = glm::vec4(
                        glm::cos(glm::radians(light.SpotAngle)),
                        glm::cos(glm::radians(light.InnerSpotAngle)),
                        light.Range, 0.0f
                    );
                    data.numSpot++;
                }
                break;
            }
        }
        UpdateSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
    }
}