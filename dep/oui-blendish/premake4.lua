
local action = _ACTION or ""

solution "blendish"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}

	project "example"
		kind "ConsoleApp"
		language "C++"
		files { "example.cpp", "nanovg/src/nanovg.c" }
		includedirs { "nanovg/src" }
		targetdir("build")

		configuration { "linux" }
			 linkoptions { "`pkg-config --libs glfw3 --static`" }
			 links { "GL", "GLU", "m", "GLEW" }
			 defines { "NANOVG_GLEW" }

		configuration { "windows" }
			 links { "glfw3", "gdi32", "winmm", "user32", "GLEW", "glu32","opengl32" }
			 defines { "NANOVG_GLEW" }

		configuration { "macosx" }
			links { "glfw3" }
			linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}

