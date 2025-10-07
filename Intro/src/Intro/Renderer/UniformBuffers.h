#pragma once
#include "UBO.h"
#include "RenderConstant.h"
#include "Camears/Camera.h"
#include "Intro/ECS/Components.h"
#include "Intro/ECS/ECS.h"
#include <array>

namespace Intro{
	struct CameraUBOData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 viewPos;
		float time;
		glm::vec3 pad;
	};

    struct DirLightGPU {
        glm::vec4 direction; // wδʹ��
        glm::vec4 color;     // rgb=��ɫ, w=ǿ��
    };
    struct PointLightGPU {
        glm::vec4 position;  // xyz=λ��, w=��Χ
        glm::vec4 color;     // rgb=��ɫ, w=ǿ��
    };
    struct SpotLightGPU {
        glm::vec4 position;  // xyz=λ��, w=��Χ
        glm::vec4 direction; // xyz=����, w=��Բ׶����
        glm::vec4 color;     // rgb=��ɫ, w=ǿ��
        glm::vec4 params;    // x=��Բ׶����, y=��Բ׶����, z=��Χ
    };

    struct LightsUBOData
    {
        int numDir;
        int numPoint;
        int numSpot;
        glm::vec2 pad;
        std::array<DirLightGPU, MAX_DIR_LIGHTS> dirLights;
        std::array<PointLightGPU, MAX_POINT_LIGHTS> pointLights;
        std::array<SpotLightGPU, MAX_SPOT_LIGHTS> spotLights;

    };

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
        void OnUpdate(ECS& ecs);
    };

}