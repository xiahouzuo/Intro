#pragma once
#include "itrpch.h"
#include "Mesh.h"
#include "Material.h"
#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <algorithm>


namespace Intro {
	struct RenderItem {
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		glm::mat4 transform;
		float distance;
		bool transparent;
	};

	class ITR_API RenderQueue
	{
	public:
		std::vector<RenderItem> opaque;
		std::vector<RenderItem> transparent;

		void Clear() {
			opaque.clear();
			transparent.clear();
		}

		void Sort(const glm::vec3& cameraPos)
		{
			// 计算距离并排序不透明物体（按材质+距离）
			for (auto& item : opaque) {
				glm::vec3 pos = glm::vec3(item.transform[3]);
				item.distance = glm::distance2(pos, cameraPos);
			}
			std::sort(opaque.begin(), opaque.end(), [](const RenderItem& a, const RenderItem& b) {
				if (a.material != b.material)
					return a.material < b.material; // 按材质指针排序（简单分组）
				return a.distance < b.distance; // 前向排序
				});

			// 排序透明物体（按距离反向）
			for (auto& item : transparent) {
				glm::vec3 pos = glm::vec3(item.transform[3]);
				item.distance = glm::distance2(pos, cameraPos);
			}
			std::sort(transparent.begin(), transparent.end(), [](const RenderItem& a, const RenderItem& b) {
				return a.distance > b.distance; // 后向排序
				});
		}
	};
}