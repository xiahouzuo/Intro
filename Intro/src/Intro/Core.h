#pragma once

#ifdef INTRO_PLATFORM_WINDOWS
	#ifdef INTRO_BUILD_DLL
		#define INTRO_API __declspec(dllexport)
	#else
		#define INTRO_API __declspec(dllimport)
	#endif // INTRO_BUILD_DLL
#else
	#error Intro only support Windows!
#endif // Intro_PLATFORM_WINDOWS
