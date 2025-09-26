#include "itrpch.h"
#include "Application.h"
#include "Log.h"
#include "Intro/Input.h"
#include "Intro/ECS/Components.h"
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

		s_SceneManager = new SceneManager;
		//��ʱģ��
		std::shared_ptr<Model> model;
		model = std::make_shared<Model>("E:/MyEngine/Intro/Intro/src/Intro/Assert/models/backpack.obj");
		Scene& defaultScene = s_SceneManager->CreateScene<Scene>("defaultScene");
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