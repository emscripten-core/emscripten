-- Box2D premake script.
-- http://industriousone.com/premake

local action = _ACTION or ""

solution "Box2D"
	location ( "Build/" .. action )
	configurations { "Debug", "Release" }
	
	configuration "vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }	
		
	configuration "Debug"
		targetdir ( "Build/" .. action .. "/bin/Debug" )
		flags { "Symbols" }

   configuration "Release"
		targetdir ( "Build/" .. action .. "/bin/Release" )
		defines { "NDEBUG" }
		flags { "Optimize" }

	project "Box2D"
		kind "StaticLib"
		language "C++"
		files { "Box2D/**.h", "Box2D/**.cpp" }
		vpaths { [""] = "Box2D" }
		includedirs { "." }
		
	if os.get == "windows" then
		project "FreeGLUT"
			kind "StaticLib"
			language "C"
			files { "freeglut/*.h", "freeglut/*.c" }
			vpaths { ["Headers"] = "**.h",  ["Sources"] = "**.c" }
	end
	
	project "GLUI"
		kind "StaticLib"
		language "C++"
		files { "glui/*.h", "glui/*.cpp" }
		vpaths { ["Headers"] = "**.h",  ["Sources"] = "**.cpp" }
		includedirs { "." }
		configuration { "windows" }
			 buildoptions { "/W1" }
			 
	project "HelloWorld"
		kind "ConsoleApp"
		language "C++"
		files { "HelloWorld/Helloworld.cpp" }
		vpaths { [""] = "HelloWorld" }
		includedirs { "." }
		links { "Box2D" }

	project "Testbed"
		kind "ConsoleApp"
		language "C++"
		files { "Testbed/**.h", "Testbed/**.cpp" }
		vpaths { [""] = "Testbed" }
		includedirs { "." }
		links { "Box2D", "GLUI" }
		configuration { "windows" }
			links { "FreeGLUT", "glu32", "opengl32", "winmm" }
		configuration { "macosx" }
			linkoptions { "-framework OpenGL -framework GLUT" }
		configuration { "not windows", "not macosx" }
			links { "X11", "GL", "GLU", "GLUT" }

