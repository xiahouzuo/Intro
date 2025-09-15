#include "itrpch.h"
#include "Application.h"
#include "Log.h"
#include "Intro/Input.h"
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>

namespace Intro {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		ITR_CORE_ASSERT(!s_Instance, "Application is already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application() {

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