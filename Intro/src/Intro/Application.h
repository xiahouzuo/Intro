#pragma once

#include"Core.h"

namespace Intro {
	class INTRO_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
	
	};

	Application* CreateApplication();

}