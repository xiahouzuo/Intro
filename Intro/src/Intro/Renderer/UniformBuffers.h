#pragma once
#include "UBO.h"
#include "RenderConstant.h"
#include "Camears/Camera.h"
#include "Intro/ECS/Components.h"
#include "Intro/ECS/ECS.h"
#include "Intro/Math/Transform.h"
#include <array>

namespace Intro{
    struct alignas(16) CameraUBOData {
        glm::mat4 view;   // 64
        glm::mat4 proj;   // 64
        glm::vec4 viewPos; // 16
        float time;        // 4
        float _pad0;       // 4
        float _pad1;       // 4
        float _pad2;       // 4   // fill to 16 bytes block
    };
    static_assert(sizeof(CameraUBOData) == 160, "");
    // 使用显式对齐并确保 std140 语义
    struct alignas(16) DirLightGPU {
        glm::vec4 direction; // 16 bytes
        glm::vec4 color;     // 16 bytes (rgb=color, w=intensity)
        // total 32
    };

    struct alignas(16) PointLightGPU {
        glm::vec4 position;  // xyz=pos, w=range
        glm::vec4 color;     // rgb=color, w=intensity
        // total 32
    };

    struct alignas(16) SpotLightGPU {
        glm::vec4 position;  // xyz=pos, w=range
        glm::vec4 direction; // xyz=direction, w=outerCos (semantic)
        glm::vec4 color;     // rgb=color, w=intensity
        glm::vec4 params;    // x=innerCos, y=unused, z=unused, w=unused (or as you like)
        // total 64
    };

    struct alignas(16) LightsUBOData
    {
        // GLSL: int numDir; int numPoint; int numSpot; vec2 pad;
        // std140 把前三个 int 放在前 12 字节，为了让后面的 vec2 按 8 字节对齐并最终让 next array 起始在 16 的倍数上，
        // 我们显式加 4 字节填充，使前 16 字节整齐。
        int numDir;    // 4
        int numPoint;  // 4
        int numSpot;   // 4
        int _pad0;     // 4  <-- explicit pad to make next member start at offset 16

        // shader had vec2 pad; in std140 it will occupy 8 bytes within a 16-byte slot.
        // We store it in a vec4 to occupy the full slot and keep alignment explicit and stable.
        glm::vec4 pad; // use .xy if you need only vec2, but occupy 16 bytes for safety.

        // Now the lights arrays begin at offset 32 (std140 expectation).
        std::array<DirLightGPU, MAX_DIR_LIGHTS> dirLights;   // 4 * 32 = 128
        std::array<PointLightGPU, MAX_POINT_LIGHTS> pointLights; // 8 * 32 = 256
        std::array<SpotLightGPU, MAX_SPOT_LIGHTS> spotLights; // 4 * 64 = 256
        // Total expected size: 32 (header) + 128 + 256 + 256 = 672 bytes


    };
        static_assert(sizeof(DirLightGPU) == 32, "DirLightGPU must be 32 bytes (std140)");
        static_assert(sizeof(PointLightGPU) == 32, "PointLightGPU must be 32 bytes (std140)");
        static_assert(sizeof(SpotLightGPU) == 64, "SpotLightGPU must be 64 bytes (std140)");
        static_assert(sizeof(LightsUBOData) == 672, "LightsUBOData must be 672 bytes to match shader std140 layout");

    class ITR_API CameraUBO : public UBO
    {
    public:
        CameraUBO()
            :UBO(GL_UNIFORM_BUFFER, sizeof(CameraUBOData))
        {
            BindBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING);
        }

        void OnUpdate(const Camera& camera, float time)
        {
            CameraUBOData data;
            data.view = camera.GetViewMat();
            data.proj = camera.GetProjectionMat();
            data.viewPos = glm::vec4(static_cast<const Camera&>(camera).GetPosition(), 1.0f);

            data.time = time;
            UpdateSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
        }

    };

    class ITR_API LightsUBO : public UBO
    {
    public:
        LightsUBO() : UBO(GL_UNIFORM_BUFFER, sizeof(LightsUBOData)) {
            BindBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING);
        }
        void LightsUBO::OnUpdate(ECS& ecs) {
            LightsUBOData data{};
            auto view = ecs.GetRegistry().view<TransformComponent, LightComponent>();

            // 重置计数器
            data.numDir = 0;
            data.numPoint = 0;
            data.numSpot = 0;

            for (auto [entity, tf, light] : view.each()) {
                switch (light.Type) {
                case LightType::Directional:
                    if (data.numDir < MAX_DIR_LIGHTS) {
                        auto& dirLight = data.dirLights[data.numDir];

                        // 计算世界空间方向
                        glm::vec3 localDirection = glm::normalize(light.Direction);
                        glm::vec3 worldDirection = tf.transform.rotation * localDirection;
                        dirLight.direction = glm::vec4(worldDirection, 0.0f);

                        // **关键调试：详细输出颜色信息**
                        //ITR_INFO("=== Light Color Debug ===");
                        //ITR_INFO("Original Color: ({},{},{})", light.Color.r, light.Color.g, light.Color.b);
                        //ITR_INFO("Intensity: {}", light.Intensity);

                        // 测试不同的颜色组合
                        glm::vec3 testColor = light.Color * light.Intensity;
                        //ITR_INFO("Final Color (Color * Intensity): ({},{},{})",
                        //    testColor.r, testColor.g, testColor.b);

                        // 强制设置为纯白色进行测试
                        // dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

                        // 或者使用原始设置
                        dirLight.color = glm::vec4(testColor, 1.0f);

                        //ITR_INFO("UBO Color: ({},{},{},{})",
                        //    dirLight.color.r, dirLight.color.g, dirLight.color.b, dirLight.color.a);

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

            //ITR_INFO("UBO Update - Directional Lights: {}", data.numDir);
            UpdateSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
        }
    };

}