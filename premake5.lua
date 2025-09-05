workspace "Intro"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-x86_64"

IncludeDir = {}
IncludeDir["GLFW"] = "Intro/vendor/GLFW/include"
IncludeDir["Glad"] = "Intro/vendor/Glad/include"
IncludeDir["ImGui"] = "Intro/vendor/imgui"

include "Intro/vendor/GLFW"
include "Intro/vendor/Glad"
include "Intro/vendor/imgui"

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
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		buildoptions { "/utf-8" }

		defines { "ITR_PLATFORM_WINDOWS", "ITR_BUILD_DLL", "GLFW_INCLUDE_NONE"}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}

	filter "configurations:Debug"
		defines "ITR_DEBUG"
		buildoptions "/MTd"
		symbols "On"

	filter "configurations:Release"
		defines "ITR_RELEASE"
		buildoptions "/MT"
		optimize "On"

	filter "configurations:Dist"
		defines "ITR_DIST"
		buildoptions "/MT"
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
		buildoptions "/MTd"
		symbols "On"

	filter "configurations:Release"
		defines "ITR_RELEASE"
		buildoptions "/MT"
		optimize "On"

	filter "configurations:Dist"
		defines "ITR_DIST"
		buildoptions "/MT"
		optimize "On"