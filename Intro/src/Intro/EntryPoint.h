#pragma once

#ifdef ITR_PLATFORM_WINDOWS

extern Intro::Application* Intro::CreateApplication();

int main(int argc,char** argv) {
	Intro::Log::Init();

	auto app = Intro::CreateApplication();
	app->Run();
	delete app;
}

#endif