#pragma once

#ifdef ITR_PLATFORM_WINDOWS
	#ifdef ITR_BUILD_DLL
		#define ITR_API __declspec(dllexport)
	#else
		#define ITR_API __declspec(dllimport)
	#endif // INTRO_BUILD_DLL
#else
	#error Intro only support Windows!
#endif // Intro_PLATFORM_WINDOWS

#define BIT(x) (1 << x)