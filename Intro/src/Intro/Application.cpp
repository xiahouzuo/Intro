#include "itrpch.h"
#include "Application.h"
#include "Log.h"
#include "Intro/Input.h"
#include "Intro/ECS/Components.h"
#include "Intro/Config/Config.h"
#include "Intro/Config/RendererConfigUtils.h"
#include "Intro/RecourceManager/ResourceManager.h"
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>

namespace Intro {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;
	SceneManager* Application::s_SceneManager = nullptr;
	ShaderLibrary* Application::s_ShaderLibrary = nullptr;

	Application::Application() {
		ITR_CORE_ASSERT(!s_Instance, "Application is already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		// 加载配置
		Config& config = Config::Get();
		if (!config.Load()) {
			ITR_WARN("Failed to load config, using defaults");
		}
		config.Get().GetGraphicsConfig().EnableGammaCorrection = false;

		// 先指定正确的 assets 根（使用绝对路径或构造出的绝对路径），再初始化 ResourceManager
		std::string assetsPath = "E:/MyEngine/Intro/Intro/src/Intro/assets/";

		// 如果有 SetAssetsRoot 函数，优先使用它（不改变现有逻辑，只设置根）
		ResourceManager::Get().SetAssetsRoot(assetsPath);

		// 然后初始化资源管理器（内部会根据 m_AssetsRoot 扫描）
		ResourceManager::Get().Initialize();

		// 设置文件树更新回调
		ResourceManager::Get().SetFileTreeUpdatedCallback([]() {
			ITR_INFO("Resource file tree updated");
			});

		s_SceneManager = new SceneManager;

		// 使用配置初始化渲染器
		auto& graphicsConfig = config.GetGraphicsConfig();
		RendererConfig rendererConfig = RendererConfigUtils::ToRendererConfig(graphicsConfig);

		// 覆盖视口大小为窗口实际大小（如果需要）
		rendererConfig.viewportWidth = m_Window->GetWidth();
		rendererConfig.viewportHeight = m_Window->GetHeight();

		if (!RendererConfigUtils::ValidateRendererConfig(rendererConfig)) {
			ITR_WARN("Invalid renderer config, using defaults");
			// 可以在这里设置安全的默认值
			rendererConfig.viewportWidth = m_Window->GetWidth();
			rendererConfig.viewportHeight = m_Window->GetHeight();
			rendererConfig.enableMSAA = true;
			rendererConfig.msaaSamples = 4;
		}

		Renderer::SetConfig(rendererConfig);
		Renderer::Init();

		s_ShaderLibrary = new ShaderLibrary;

		auto lineShader = std::make_shared<Shader>(
			"E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/lineShader.vert",
			"E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/lineShader.frag"
		);
		s_ShaderLibrary->Add("lineShader", lineShader);
		
		auto defaultShader = std::make_shared<Shader>(
			"E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/tempShader.vert",
			"E:/MyEngine/Intro/Intro/src/Intro/assets/shaders/tempShader.frag"
		);
		s_ShaderLibrary->Add("defaultShader", defaultShader);

		defaultMaterial = std::make_shared<Material>(defaultShader);

		Scene& defaultScene = s_SceneManager->CreateScene<Scene>("defaultScene");
		//临时模型
		//std::shared_ptr<Model> model;
		//model = std::make_shared<Model>("E:/MyEngine/Intro/Intro/src/Intro/Assert/models/backpack.obj");
		//GameObject backpack = defaultScene.CreateGameObject("defaultModel");
		//backpack.AddComponent<ModelComponent>(model);
		//backpack.AddComponent<MaterialComponent>(defaultMaterial);

		// 创建明确的默认方向光
		entt::entity lightEntity = defaultScene.CreateEntity();
		defaultScene.GetECS().AddComponent<TagComponent>(lightEntity, "Main Directional Light");

		// 设置方向光的变换 - 明确指向下方
		TransformComponent lightTransform;
		lightTransform.transform.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 指向下方
		defaultScene.GetECS().AddComponent<TransformComponent>(lightEntity, lightTransform);

		LightComponent light;
		light.Type = LightType::Directional;
		light.Color = glm::vec3(1.0f, 1.0f, 1.0f); // 白色
		light.Intensity = 0.5f;
		light.Direction = glm::vec3(0.0f, 0.0f, -1.0f); // 局部空间向前
		defaultScene.GetECS().AddComponent<LightComponent>(lightEntity, light);

		defaultScene.SetActive(true);

		s_SceneManager->SetActiveSceneByName("defaultScene");
	}

	Application::~Application() {
		if (s_SceneManager)
		{
			delete s_SceneManager;
			s_SceneManager = nullptr;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Run() {
		float lastFrameTime = 0.0f; // 上一帧的时间

		while (m_Running)
		{
			float currentTime = (float)glfwGetTime(); // 假设使用 GLFW，需要包含 GLFW/glfw3.h
			float deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			Input::Update();

			if (s_SceneManager)
			{
				s_SceneManager->OnUpdate(deltaTime);
			}

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(deltaTime); // 传递 deltaTime

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end();it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.m_Handled)
				break;
		}


		//ITR_CORE_INFO("{0}", e.ToString());
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}



}