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

#ifdef ITR_ENABLE_ASSERTS
	#define ITR_ASSERT(x, ...) { if(!(x)) {ITR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define ITR_CORE_ASSERT(x, ...) { if(!(x)) {ITR_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define ITR_ASSERT(x, ...)
	#define ITR_CORE_ASSERT(x, ...)
#endif // ITR_ENABLE_ASSERTS


#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define BIT(x) (1 << x)