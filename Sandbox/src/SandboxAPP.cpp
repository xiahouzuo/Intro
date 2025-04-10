#include<Intro.h>

class Sandbox :public Intro::Application {
	
public:
	Sandbox() {

	}

	~Sandbox() {

	}
};

Intro::Application* Intro::CreateApplication() {

	return new Sandbox();

}

