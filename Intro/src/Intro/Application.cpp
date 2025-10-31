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

	Application::Application() {
		ITR_CORE_ASSERT(!s_Instance, "Application is already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		// ��������
		Config& config = Config::Get();
		if (!config.Load()) {
			ITR_WARN("Failed to load config, using defaults");
		}

		// ��ָ����ȷ�� assets ����ʹ�þ���·��������ľ���·�������ٳ�ʼ�� ResourceManager
		std::string assetsPath = "E:/MyEngine/Intro/Intro/src/Intro/assets/";

		// ����� SetAssetsRoot ����������ʹ���������ı������߼���ֻ���ø���
		ResourceManager::Get().SetAssetsRoot(assetsPath);

		// Ȼ���ʼ����Դ���������ڲ������ m_AssetsRoot ɨ�裩
		ResourceManager::Get().Initialize();

		// �����ļ������»ص�
		ResourceManager::Get().SetFileTreeUpdatedCallback([]() {
			ITR_INFO("Resource file tree updated");
			});

		s_SceneManager = new SceneManager;

		// ʹ�����ó�ʼ����Ⱦ��
		auto& graphicsConfig = config.GetGraphicsConfig();
		RendererConfig rendererConfig = RendererConfigUtils::ToRendererConfig(graphicsConfig);

		// �����ӿڴ�СΪ����ʵ�ʴ�С�������Ҫ��
		rendererConfig.viewportWidth = m_Window->GetWidth();
		rendererConfig.viewportHeight = m_Window->GetHeight();

		if (!RendererConfigUtils::ValidateRendererConfig(rendererConfig)) {
			ITR_WARN("Invalid renderer config, using defaults");
			// �������������ð�ȫ��Ĭ��ֵ
			rendererConfig.viewportWidth = m_Window->GetWidth();
			rendererConfig.viewportHeight = m_Window->GetHeight();
			rendererConfig.enableMSAA = true;
			rendererConfig.msaaSamples = 4;
		}

		Renderer::SetConfig(rendererConfig);
		Renderer::Init();

		
		defaultShader = std::make_shared<Shader>(
			"E:/MyEngine/Intro/Intro/src/Intro/assert/shaders/tempShader.vert",
			"E:/MyEngine/Intro/Intro/src/Intro/assert/shaders/tempShader.frag"
		);
		defaultMaterial = std::make_shared<Material>(defaultShader);
		//��ʱģ��
		std::shared_ptr<Model> model;
		model = std::make_shared<Model>("E:/MyEngine/Intro/Intro/src/Intro/Assert/models/backpack.obj");
		Scene& defaultScene = s_SceneManager->CreateScene<Scene>("defaultScene");


		// ������ȷ��Ĭ�Ϸ����
		entt::entity lightEntity = defaultScene.CreateEntity();
		defaultScene.GetECS().AddComponent<TagComponent>(lightEntity, "Main Directional Light");

		// ���÷����ı任 - ��ȷָ���·�
		TransformComponent lightTransform;
		lightTransform.transform.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // ָ���·�
		defaultScene.GetECS().AddComponent<TransformComponent>(lightEntity, lightTransform);

		LightComponent light;
		light.Type = LightType::Directional;
		light.Color = glm::vec3(1.0f, 1.0f, 1.0f); // ��ɫ
		light.Intensity = 0.5f;
		light.Direction = glm::vec3(0.0f, 0.0f, -1.0f); // �ֲ��ռ���ǰ
		defaultScene.GetECS().AddComponent<LightComponent>(lightEntity, light);


		entt::entity entity = defaultScene.CreateEntity();
		auto& transformComp = defaultScene.GetECS().AddComponent<TransformComponent>(entity,
			glm::vec3(0.0f, 0.0f, 0.0f),  // λ��
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),  // ��ת����λ��Ԫ����
			glm::vec3(1.0f, 1.0f, 1.0f)   // ����
			);

		auto& modelComp = defaultScene.GetECS().AddComponent<ModelComponent>(
			entity,
			model  // ģ����Դ
		);

		auto& materialComp = defaultScene.GetECS().AddComponent<MaterialComponent>(
			entity,
			defaultMaterial
			);

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
		float lastFrameTime = 0.0f; // ��һ֡��ʱ��

		while (m_Running)
		{
			float currentTime = (float)glfwGetTime(); // ����ʹ�� GLFW����Ҫ���� GLFW/glfw3.h
			float deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			Input::Update();

			if (s_SceneManager)
			{
				s_SceneManager->OnUpdate(deltaTime);
			}

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(deltaTime); // ���� deltaTime

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