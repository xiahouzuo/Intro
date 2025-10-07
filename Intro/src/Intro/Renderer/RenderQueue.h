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
			// ������벢����͸�����壨������+���룩
			for (auto& item : opaque) {
				glm::vec3 pos = glm::vec3(item.transform[3]);
				item.distance = glm::distance2(pos, cameraPos);
			}
			std::sort(opaque.begin(), opaque.end(), [](const RenderItem& a, const RenderItem& b) {
				if (a.material != b.material)
					return a.material < b.material; // ������ָ�����򣨼򵥷��飩
				return a.distance < b.distance; // ǰ������
				});

			// ����͸�����壨�����뷴��
			for (auto& item : transparent) {
				glm::vec3 pos = glm::vec3(item.transform[3]);
				item.distance = glm::distance2(pos, cameraPos);
			}
			std::sort(transparent.begin(), transparent.end(), [](const RenderItem& a, const RenderItem& b) {
				return a.distance > b.distance; // ��������
				});
		}
	};
}