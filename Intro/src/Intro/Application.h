#pragma once

#include "Core.h"
#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "LayerStack.h"
#include "Intro/ECS/SceneManager.h"


namespace Intro {
	class ITR_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
		inline static SceneManager& GetSceneManager()
		{
			ITR_CORE_ASSERT(!s_SceneManager, "SceneManager is null!");
			return *s_SceneManager;
		}
		
		bool IsViewportHovered() const { return m_ViewportHovered; }
		bool IsViewportFocused() const { return m_ViewportFocused; }
		bool IsUsingGizmo() const { return m_IsUsingGizmo; }

		void SetViewportState(bool hovered, bool focused, bool usingGizmo) {
			m_ViewportHovered = hovered;
			m_ViewportFocused = focused;
			m_IsUsingGizmo = usingGizmo;
		}

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

		bool m_ViewportHovered = false;
		bool m_ViewportFocused = false;
		bool m_IsUsingGizmo = false;

	private:
		static Application* s_Instance;

		static SceneManager* s_SceneManager;
	};

	Application* CreateApplication();

}