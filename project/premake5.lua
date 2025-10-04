workspace "TestProject4"
	architecture "x64"
	startproject "TestProject4"
	
	configurations
	{
		"Debug",
		"Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "TestProject4"
	location "TestProject4"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp",
		
		"%{prj.name}/gen/**.h",
		"%{prj.name}/gen/**.hpp",
		"%{prj.name}/gen/**.c",
		"%{prj.name}/gen/**.cpp",
		
		"C:/dev/Omni/Omni/vendor/MurmurHash2/**.h",
		"C:/dev/Omni/Omni/vendor/MurmurHash2/**.cpp",
	}
	
	includedirs
	{
		"%{prj.name}",
		"%{prj.name}/src",
		"%{prj.name}/gen",
		
		"C:/dev/Omni/Omni/include",
		"C:/dev/Omni/Omni/src/Omni",
		"C:/dev/Omni/Omni/src",
		"C:/dev/Omni/Omni/vendor",
		"C:/dev/Omni/Omni/vendor/spdlog/include",
		"C:/dev/Omni/Omni/vendor/glm",
		"C:/dev/Omni/Omni/vendor/glfw/include",
		"C:/dev/Omni/Omni/vendor/glad/include",
		"C:/dev/Omni/Omni/vendor/imgui",
		"C:/dev/Omni/Omni/vendor/box2d/include",
		"C:/dev/Omni/Omni/vendor/entt",
		"C:/dev/Omni/Omni/vendor/yaml-cpp/include",
		"C:/dev/Omni/Omni/vendor/ImGuizmo"
	}
	
	defines
	{
		"BUILD_USER_DLL",
		"OM_DYNAMIC_LINK",
		"IMGUI_API=__declspec(dllimport)",
		"OM_PLATFORM_WINDOWS",
		"OM_DEBUG",
		"YAML_CPP_STATIC_DEFINE",
		"GLFW_INCLUDE_NONE",
		"GLFW_DLL",
		"STB_IMAGE_STATIC",
		"STB_IMAGE_IMPLEMENTATION",
		"STB_TRUETYPE_IMPLEMENTATION",
		"STB_RECT_PACK_IMPLEMENTATION",
	}
	
	prebuildcommands
	{
		"python \"$(SolutionDir)\\scripts\\gen.py\" %{prj.name}\\src\\ %{prj.name}\\gen\\"
	}
	
	filter "configurations:Debug"
		libdirs
		{
			"%{prj.name}/lib",
			"C:/dev/Omni/bin/Debug-windows-x86_64/Omni",
			"C:/dev/Omni/Omni/vendor/imgui/bin/Debug-windows-x86_64/ImGui",
			"C:/dev/Omni/Omni/vendor/glfw/bin/Debug-windows-x86_64/GLFW",
			"C:/dev/Omni/Omni/vendor/glad/bin/Debug-windows-x86_64/Glad",
			"C:/dev/Omni/Omni/vendor/box2d/bin/Debug-windows-x86_64/Box2D",
			"C:/dev/Omni/Omni/vendor/yaml-cpp/bin/Debug-windows-x86_64/yaml-cpp",
		}
	
		links
		{
			"Omni",
			"ImGui",
			"GLFW",
			"Glad",
			"Box2D",
			"yaml-cpp",
		}
	
	filter "configurations:Release"
		libdirs
		{
			"%{prj.name}/lib",
			"C:/dev/Omni/bin/Release-windows-x86_64/Omni",
			"C:/dev/Omni/Omni/vendor/imgui/bin/Release-windows-x86_64/ImGui",
			"C:/dev/Omni/Omni/vendor/glfw/bin/Release-windows-x86_64/GLFW",
			"C:/dev/Omni/Omni/vendor/glad/bin/Release-windows-x86_64/Glad",
			"C:/dev/Omni/Omni/vendor/box2d/bin/Release-windows-x86_64/Box2D",
			"C:/dev/Omni/Omni/vendor/yaml-cpp/bin/Release-windows-x86_64/yaml-cpp",
		}
	
		links
		{
			"Omni",
			"ImGui",
			"GLFW",
			"Glad",
			"Box2D",
			"yaml-cpp",
		}
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
