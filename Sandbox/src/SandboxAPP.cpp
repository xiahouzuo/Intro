
#include<Intro.h>
#include"Intro/Assert/Model.h"

class ExampleLayer : public Intro::Layer
{
public:
	ExampleLayer()
		:Layer("Example")
	{
		m_Model = std::make_unique<Intro::Model>("E:/MyEngine/Intro/Intro/src/Intro/Assert/models/backpack.obj");
	}

	void OnUpdate(float deltaTime) override
	{
		ITR_INFO("EXAMPLELAYER::UPDATE");

		if (Intro::Input::IsKeyPressed(ITR_KEY_TAB)) {
			ITR_TRACE("Tab key is pressed!");
			std::cout << "11";
		}
		m_Model->Draw();

	}

	void OnEvent(Intro::Event& event) override
	{
		//ITR_TRACE("{0}", event.ToString());
	}

private:
	std::unique_ptr<Intro::Model> m_Model;
};


class Sandbox :public Intro::Application {
	
public:
	Sandbox() {
		Intro::RendererLayer* rendererlayer = new Intro::RendererLayer(GetWindow());
		PushLayer(rendererlayer);
		PushLayer(new ExampleLayer);
		Intro::ImGuiLayer* imguilayer = new Intro::ImGuiLayer();
		imguilayer->SetRendererLayer(rendererlayer);
		PushOverlay(imguilayer);
	}

	~Sandbox() {

	}
};

Intro::Application* Intro::CreateApplication() {

	return new Sandbox();

}

