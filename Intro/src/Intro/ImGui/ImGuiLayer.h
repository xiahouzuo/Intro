#pragma once
#include "Intro/Layer.h"

#include "Intro/Events/ApplicationEvent.h"
#include "Intro/Events/KeyEvent.h"
#include "Intro/Events/MouseEvent.h"
#include "Intro/Renderer/RendererLayer.h"

namespace Intro {

	class ITR_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate(float deltaTime) override;
		void OnEvent(Event& event);

		void SetRendererLayer(RendererLayer* layer) { m_RendererLayer = layer; }
	private:
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseScrooledEvent(MouseScrolledEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnWindowResizedEvent(WindowResizeEvent& e);

	private:
		float m_Time = 0.0f;

		RendererLayer* m_RendererLayer;
	};

}
