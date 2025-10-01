-- premake5.lua (ʹ�þ�̬ CRT: /MT, /MTd)
workspace "Intro"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-x86_64"

-- helper
local path = require("path")

IncludeDir = {}
IncludeDir["GLFW"]     = "Intro/vendor/GLFW/include"
IncludeDir["Glad"]     = "Intro/vendor/Glad/include"
IncludeDir["ImGui"]    = "Intro/vendor/imgui"
IncludeDir["glm"]      = "Intro/vendor/glm"
IncludeDir["nlohmann"] = "Intro/vendor/nlohmann"
IncludeDir["entt"]     = "Intro/vendor/entt"
IncludeDir["Assimp"]   = "Intro/vendor/assimp/include"
IncludeDir["stbimage"] = "Intro/vendor/stbimage"
IncludeDir["ImGuizmo"] = "Intro/vendor/imguizmo"

-- ��������Ŀ�������Щ premake.module ���ڣ�
include "Intro/vendor/GLFW"
include "Intro/vendor/Glad"
include "Intro/vendor/imgui"

project "Intro"
    location "Intro"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "itrpch.h"
    pchsource "Intro/src/itrpch.cpp"

    files { 
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/imguizmo/*.h",
        "%{prj.name}/vendor/imguizmo/*.cpp"
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "Intro/vendor/imgui",
        "Intro/vendor/glm",
        "Intro/vendor/nlohmann",
        "Intro/vendor/entt",
        "Intro/vendor/assimp/include",
        "Intro/vendor/stbimage",
        "Intro/vendor/imguizmo"
    }

    links {
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    -- Windows common settings
    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"        -- ʹ�þ�̬����ʱ (/MT �� /MTd)
        systemversion "latest"
        buildoptions { "/utf-8" }
        defines { "ITR_PLATFORM_WINDOWS", "ITR_BUILD_DLL", "GLFW_INCLUDE_NONE" }
        postbuildcommands {
            ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
        }

    -- Debug (��̬ CRT Debug: /MTd)
    filter { "system:windows", "configurations:Debug" }
        defines { "ITR_DEBUG" }
        symbols "On"
        -- ʹ�þ���·����ȷ�����������ҵ���
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Debug"),
            path.getabsolute("Intro/vendor/assimp/zlib/Debug")
        }
        -- �������һ�µľ�̬ debug ���������в�ͬ���Ϊʵ���ļ�����
        links { "assimp-vc143-mtd.lib", "zlibstaticd.lib" }

    -- Release (��̬ CRT Release: /MT)
    filter { "system:windows", "configurations:Release" }
        defines { "ITR_RELEASE" }
        optimize "On"
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    -- Dist (���� Release ����)
    filter { "system:windows", "configurations:Dist" }
        defines { "ITR_DIST" }
        optimize "On"
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    -- ��� filter �̳�
    filter {}

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    dependson "Intro"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    files { "%{prj.name}/src/**.h", "%{prj.name}/src/**.cpp" }

    includedirs {
        "Intro/vendor/spdlog/include",
        "Intro/src",
        "Intro/src/Intro",
        "Intro/vendor/glm",
        "%{IncludeDir.nlohmann}",
        "%{IncludeDir.Glad}",
        "Intro/vendor/assimp/include",
        "Intro/vendor/stbimage",
        "Intro/vendor/entt",
        "Intro/vendor/imgui",
        "Intro/vendor/imguizmo"
    }

    links { "Intro" }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"        -- Sandbox Ҳʹ�þ�̬ CRT
        systemversion "latest"
        buildoptions { "/utf-8" }
        defines { "ITR_PLATFORM_WINDOWS" }

    filter { "system:windows", "configurations:Debug" }
        defines { "ITR_DEBUG" }
        symbols "On"
        libdirs {
            path.getabsolute("bin/" .. outputdir .. "/Intro"),
            path.getabsolute("Intro/vendor/assimp/libs/Debug"),
            path.getabsolute("Intro/vendor/assimp/zlib/Debug")
        }
        links { "assimp-vc143-mtd.lib", "zlibstaticd.lib" }

    filter { "system:windows", "configurations:Release" }
        defines { "ITR_RELEASE" }
        optimize "On"
        libdirs {
            path.getabsolute("bin/" .. outputdir .. "/Intro"),
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    filter { "system:windows", "configurations:Dist" }
        defines { "ITR_DIST" }
        optimize "On"
        libdirs {
            path.getabsolute("bin/" .. outputdir .. "/Intro"),
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    filter {}
