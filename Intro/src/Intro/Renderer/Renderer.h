#pragma once

#include "Intro/Core.h"
#include "RenderCommand.h"

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Intro {

	struct RendererConfig
	{
		uint32_t viewportWidth = 1280;
		uint32_t viewportHeight = 720;
		bool enableMSAA = false;
		uint32_t msaaSamples = 4;
		bool enableHDR = false;
		bool enableGammaCorrection = true;
	};

	class ITR_API Renderer {
	public:

		//�������ڿ���
		static void Init();//
		static void Shutdown();

		//ÿ֡����
		static void BeginFrame();
		static void EndFrame();

		// ��Ⱦͨ���������ڶ�ͨ��/����
		static void BeginRenderPass(const std::shared_ptr<class RenderPass>& renderPass);
		static void EndRenderPass();

		//�ύ
		// �ύ Mesh���� shader������ transform��Ȼ���� RenderCommand ���� draw��
		static void Submit(const std::shared_ptr<class Shader>& shader,
							const std::shared_ptr<class Mesh>& mesh,
							const glm::mat4& transfrom = glm::mat4(1.0f));

		// �ύ Model������ Model ������ Mesh ���ύ��
		static void Submit(const std::shared_ptr<class Shader>& shader,
			const std::shared_ptr<class Model>& model,
			const glm::mat4& transfrom = glm::mat4(1.0f));

		// ���ڴ�����ڣ�����һ������ shader���ú���������� FBO ����ɫ����󶨵���Ԫ����Ⱦȫ���ı��Σ�
		static void PostProcess(const std::shared_ptr<class Shader>& postProcessShader);
		
		static std::shared_ptr<class Shader> GetShader(const std::string& name);
		static std::shared_ptr<class Framebuffer> GetMainFramebuffer();

		static void SetConfig(const RendererConfig& config);
		static const RendererConfig& GetConfig();

		//ͳ��
		struct Statistics {
			uint32_t drawCalls = 0;
			uint32_t triangleCount = 0;
			uint32_t vertexCount = 0;
			void Reset() { drawCalls = triangleCount = vertexCount = 0; }
		};
		static const Statistics& GetStats();
		static void ResetStats();


	private:
		//���ύ���е��õ�GPU
		static void FlushBatch();

		// �������ݽṹ���洢Ҫ���Ƶ� shader/mesh/transform
		struct BatchData {
			std::shared_ptr<Shader> shader;
			std::shared_ptr<Mesh> mesh;
			glm::mat4 transform;
		};

		static std::vector<BatchData> s_BatchQueue;
		static Statistics s_Stats;
		static RendererConfig s_Config;
		static std::unique_ptr<class ShaderLibrary> s_ShaderLibrary;
		static std::shared_ptr<Framebuffer> s_MainFramebuffer;
		static std::shared_ptr<Framebuffer> s_PostProcessFramebuffer;
		static bool s_Initialized;
	};

}