workspace "Intro"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-x86_64"

IncludeDir = {}
IncludeDir["GLFW"] = "Intro/vendor/GLFW/include"

include "Intro/vendor/GLFW"

project "Intro"
	location "Intro"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "itrpch.h"
	pchsource "Intro/src/itrpch.cpp"

	files { "%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp" }

	includedirs {
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		buildoptions { "/utf-8" }

		defines { "ITR_PLATFORM_WINDOWS", "ITR_BUILD_DLL" }

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}

	filter "configurations:Debug"
		defines "ITR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ITR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "ITR_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	dependson "Intro"  -- 添加依赖声明
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files { "%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp" }

	includedirs {
		"Intro/vendor/spdlog/include",
		"Intro/src",           -- 主目录
		"Intro/src/Intro"      -- 子目录（包含Log.h的实际位置）
	}

	libdirs {                  -- 添加库目录
		"bin/" .. outputdir .. "/Intro"
	}

	links { "Intro" }

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		buildoptions { "/utf-8" }

		defines { "ITR_PLATFORM_WINDOWS" }

		-- 移除不必要的后期生成命令
		-- postbuildcommands {
		--	("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		-- }

	filter "configurations:Debug"
		defines "ITR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ITR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "ITR_DIST"
		optimize "On"