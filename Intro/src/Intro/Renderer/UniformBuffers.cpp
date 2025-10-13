#include "itrpch.h"
#include "UniformBuffers.h"
#include "Intro/ECS/ECS.h"
#include "Intro/Math/Transform.h" // ����Transform��GetPosition��GetRotation����

namespace Intro {
    void LightsUBO::OnUpdate(ECS& ecs) {
        LightsUBOData data{};
        auto view = ecs.GetRegistry().view<TransformComponent, LightComponent>();

        // ���ü�����
        data.numDir = 0;
        data.numPoint = 0;
        data.numSpot = 0;

        for (auto [entity, tf, light] : view.each()) {
            switch (light.Type) {
            case LightType::Directional:
                if (data.numDir < MAX_DIR_LIGHTS) {
                    auto& dirLight = data.dirLights[data.numDir];

                    // ��������ռ䷽��
                    glm::vec3 localDirection = glm::normalize(light.Direction);
                    glm::vec3 worldDirection = tf.transform.rotation * localDirection;
                    dirLight.direction = glm::vec4(worldDirection, 0.0f);

                    // **�ؼ����ԣ���ϸ�����ɫ��Ϣ**
                    ITR_INFO("=== Light Color Debug ===");
                    ITR_INFO("Original Color: ({},{},{})", light.Color.r, light.Color.g, light.Color.b);
                    ITR_INFO("Intensity: {}", light.Intensity);

                    // ���Բ�ͬ����ɫ���
                    glm::vec3 testColor = light.Color * light.Intensity;
                    ITR_INFO("Final Color (Color * Intensity): ({},{},{})",
                        testColor.r, testColor.g, testColor.b);

                    // ǿ������Ϊ����ɫ���в���
                    // dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

                    // ����ʹ��ԭʼ����
                    dirLight.color = glm::vec4(testColor, 1.0f);

                    ITR_INFO("UBO Color: ({},{},{},{})",
                        dirLight.color.r, dirLight.color.g, dirLight.color.b, dirLight.color.a);

                    data.numDir++;
                }
                break;
            case LightType::Point:
                if (data.numPoint < MAX_POINT_LIGHTS) {
                    auto& pointLight = data.pointLights[data.numPoint];
                    pointLight.position = glm::vec4(tf.transform.position, light.Range);
                    pointLight.color = glm::vec4(light.Color * light.Intensity, 1.0f);
                    data.numPoint++;
                }
                break;
            case LightType::Spot:
                if (data.numSpot < MAX_SPOT_LIGHTS) {
                    auto& spotLight = data.spotLights[data.numSpot];
                    spotLight.position = glm::vec4(tf.transform.position, light.Range);

                    glm::vec3 worldDirection = tf.transform.rotation * glm::normalize(light.Direction);
                    spotLight.direction = glm::vec4(worldDirection, 0.0f);
                    spotLight.color = glm::vec4(light.Color * light.Intensity, 1.0f);
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

        ITR_INFO("UBO Update - Directional Lights: {}", data.numDir);
        UpdateSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
    }
}