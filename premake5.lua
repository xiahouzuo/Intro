-- premake5.lua (使用静态 CRT: /MT, /MTd)
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

-- 引入子项目（如果这些 premake.module 存在）
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
        staticruntime "On"        -- 使用静态运行时 (/MT 或 /MTd)
        systemversion "latest"
        buildoptions { "/utf-8" }
        defines { "ITR_PLATFORM_WINDOWS", "ITR_BUILD_DLL", "GLFW_INCLUDE_NONE" }
        postbuildcommands {
            ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
        }

    -- Debug (静态 CRT Debug: /MTd)
    filter { "system:windows", "configurations:Debug" }
        defines { "ITR_DEBUG" }
        symbols "On"
        -- 使用绝对路径以确保链接器能找到库
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Debug"),
            path.getabsolute("Intro/vendor/assimp/zlib/Debug")
        }
        -- 与磁盘上一致的静态 debug 库名（如有不同请改为实际文件名）
        links { "assimp-vc143-mtd.lib", "zlibstaticd.lib" }

    -- Release (静态 CRT Release: /MT)
    filter { "system:windows", "configurations:Release" }
        defines { "ITR_RELEASE" }
        optimize "On"
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    -- Dist (复用 Release 设置)
    filter { "system:windows", "configurations:Dist" }
        defines { "ITR_DIST" }
        optimize "On"
        libdirs {
            path.getabsolute("Intro/vendor/assimp/libs/Release"),
            path.getabsolute("Intro/vendor/assimp/zlib/Release")
        }
        links { "assimp-vc143-mt.lib", "zlibstatic.lib" }

    -- 清空 filter 继承
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
        staticruntime "On"        -- Sandbox 也使用静态 CRT
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
