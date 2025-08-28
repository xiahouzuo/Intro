#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"


namespace Intro {
	class ITR_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}



#define ITR_CORE_TRACE(...)   ::Intro::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ITR_CORE_INFO(...)    ::Intro::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ITR_CORE_WARN(...)    ::Intro::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ITR_CORE_ERROR(...)   ::Intro::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ITR_CORE_FATAL(...)   ::Intro::Log::GetCoreLogger()->fatal(__VA_ARGS__)


#define ITR_TRACE(...)   ::Intro::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ITR_INFO(...)    ::Intro::Log::GetClientLogger()->info(__VA_ARGS__)
#define ITR_WARN(...)    ::Intro::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ITR_ERROR(...)   ::Intro::Log::GetClientLogger()->error(__VA_ARGS__)
#define ITR_FATAL(...)   ::Intro::Log::GetClientLogger()->fatal(__VA_ARGS__)