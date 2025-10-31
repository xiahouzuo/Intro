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

		//生命周期控制
		static void Init();//
		static void Shutdown();

		//每帧调用
		static void BeginFrame();
		static void EndFrame();

		// 渲染通道（可用于多通道/后处理）
		static void BeginRenderPass(const std::shared_ptr<class RenderPass>& renderPass);
		static void EndRenderPass();

		//提交
		// 提交 Mesh（绑定 shader，设置 transform，然后由 RenderCommand 真正 draw）
		static void Submit(const std::shared_ptr<class Shader>& shader,
							const std::shared_ptr<class Mesh>& mesh,
							const glm::mat4& transfrom = glm::mat4(1.0f));

		// 提交 Model（遍历 Model 的所有 Mesh 并提交）
		static void Submit(const std::shared_ptr<class Shader>& shader,
			const std::shared_ptr<class Model>& model,
			const glm::mat4& transfrom = glm::mat4(1.0f));

		// 后期处理入口：传入一个后处理 shader（该函数负责把主 FBO 的颜色纹理绑定到单元并渲染全屏四边形）
		static void PostProcess(const std::shared_ptr<class Shader>& postProcessShader);
		
		static std::shared_ptr<class Shader> GetShader(const std::string& name);
		static std::shared_ptr<class Framebuffer> GetMainFramebuffer();

		static void SetConfig(const RendererConfig& config);
		static const RendererConfig& GetConfig();

		//统计
		struct Statistics {
			uint32_t drawCalls = 0;
			uint32_t triangleCount = 0;
			uint32_t vertexCount = 0;
			void Reset() { drawCalls = triangleCount = vertexCount = 0; }
		};
		static const Statistics& GetStats();
		static void ResetStats();


	private:
		//把提交队列调用到GPU
		static void FlushBatch();

		// 批次数据结构：存储要绘制的 shader/mesh/transform
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