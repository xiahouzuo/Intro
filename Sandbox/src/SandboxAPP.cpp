
#include<Intro.h>

class ExampleLayer : public Intro::Layer
{
public:
	ExampleLayer()
		:Layer("Example")
	{

	}

	void OnUpdate() override
	{
		ITR_INFO("EXAMPLELAYER::UPDATE");

		if (Intro::Input::IsKeyPressed(ITR_KEY_TAB)) {
			ITR_TRACE("Tab key is pressed!");
			std::cout << "11";
		}
	}

	void OnEvent(Intro::Event& event) override
	{
		ITR_TRACE("{0}", event.ToString());
	}
};


class Sandbox :public Intro::Application {
	
public:
	Sandbox() {
		PushLayer(new ExampleLayer);
		PushOverlay(new Intro::ImGuiLayer);
	}

	~Sandbox() {

	}
};

Intro::Application* Intro::CreateApplication() {

	return new Sandbox();

}

