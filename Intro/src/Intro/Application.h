#pragma once

#include"Core.h"

namespace Intro {
	class ITR_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
	
	};

	Application* CreateApplication();

}