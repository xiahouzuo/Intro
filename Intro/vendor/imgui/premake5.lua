project "ImGui"
	kind "StaticLib"
	language "C++"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",

        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3.cpp",
        "backends/imgui_impl_opengl3_loader.h",

        "misc/cpp/imgui_stdlib.h",    -- ͷ�ļ�
        "misc/cpp/imgui_stdlib.cpp"
	}

    IncludeDir = {}
    IncludeDir["GLFW"] = "Intro/vendor/GLFW/include"
    IncludeDir["Glad"] = "Intro/vendor/Glad/include"

    includedirs
    {
        ".",                 -- �ؼ����� backends �ڵ��ļ��ҵ��ϼ��� imgui.h
        "backends",          -- ���ͷ�ļ�Ŀ¼
        "%{IncludeDir.GLFW}",-- GLFW ͷ�ļ�Ŀ¼�����������
        "%{IncludeDir.Glad}" -- Glad ͷ�ļ�Ŀ¼��OpenGL ����������
    }

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
        buildoptions "/MTd"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
        buildoptions "/MT"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        buildoptions "/MT"
        symbols "off"


    links
      {
          "GLFW",
          "Glad"
      }
