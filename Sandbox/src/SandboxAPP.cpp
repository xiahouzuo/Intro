
#include<Intro.h>
#include"Intro/Renderer/Model.h"

class ExampleLayer : public Intro::Layer
{
public:
	ExampleLayer()
		:Layer("Example")
	{
		
	}

	void OnUpdate(float deltaTime) override
	{
		ITR_INFO("EXAMPLELAYER::UPDATE");

		if (Intro::Input::IsKeyPressed(ITR_KEY_TAB)) {
			ITR_TRACE("Tab key is pressed!");
			std::cout << "11";
		}
		

	}

	void OnEvent(Intro::Event& event) override
	{
		//ITR_TRACE("{0}", event.ToString());
	}

private:
	
};


class Sandbox :public Intro::Application {
	
public:
	Sandbox() {
		Intro::RendererLayer* rendererlayer = new Intro::RendererLayer(GetWindow());
		PushLayer(rendererlayer);
		Intro::ImGuiLayer* imguilayer = new Intro::ImGuiLayer(&Application::GetSceneManager());
		imguilayer->SetRendererLayer(rendererlayer);
		PushOverlay(imguilayer);
		
	}

	~Sandbox() {

	}
};

Intro::Application* Intro::CreateApplication() {

	return new Sandbox();

}

