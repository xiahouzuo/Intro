#pragma once

#ifdef INTRO_PLATFORM_WINDOWS

extern Intro::Application* Intro::CreateApplication();

int main(int argc,char** argv) {
	auto app = Intro::CreateApplication();
	app->Run();
	delete app;
}

#endif