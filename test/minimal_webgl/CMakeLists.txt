cmake_minimum_required(VERSION 2.8)

# Default to release build if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(minimal_webgl)

macro(append_linker_flags FLAGS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# Link in the JS library file for support code
	append_linker_flags("--js-library ${CMAKE_CURRENT_LIST_DIR}/library_js.js")

	# Link in to WebGL/GLES system library
	append_linker_flags("-lGL")

	# Enable Closure compiler for aggressive JS size minification
	append_linker_flags("--closure 1")

	# When marshalling C UTF-8 strings across the JS<->Wasm language boundary, favor smallest generated code size
	# rather than performance
	append_linker_flags("-sTEXTDECODER=2")

	# Enable aggressive MINIMAL_RUNTIME mode.
	append_linker_flags("-sMINIMAL_RUNTIME=2")

	# Require WebGL 2 support in target browser, for smallest generated code size. (pass -sMIN_WEBGL_VERSION=1 to dual-target WebGL 1 and WebGL 2)
	append_linker_flags("-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")

	# Tell the example code in webgl.c that we are only targeting WebGL 2.
	add_definitions(-DMAX_WEBGL_VERSION=2)

	# The generated build output is only to be expected to be run in a web browser, never in a native CLI shell, or in a web worker.
	append_linker_flags("-sENVIRONMENT=web")

	# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less
	# emulation code may be present for old browser quirks.
	append_linker_flags("-sMIN_FIREFOX_VERSION=70")
	append_linker_flags("-sMIN_SAFARI_VERSION=130000")
	append_linker_flags("-sMIN_CHROME_VERSION=80")

	# Fine tuning for code size: do not generate code to abort program execution on malloc() failures, that will
	# not be interesting here.
	append_linker_flags("-sABORTING_MALLOC=0")

	# Reduce WebGL code size: We do not need GLES2 emulation for automatic GL extension enabling
	append_linker_flags("-sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=0")

	# Reduce WebGL code size: We do not need GLES2 emulation for GL extension names
	append_linker_flags("-sGL_EXTENSIONS_IN_PREFIXED_FORMAT=0")

	# Reduce WebGL code size: No need to specify the GL_VENDOR/GL_RENDERER etc. fields in format required by GLES2 spec.
	append_linker_flags("-sGL_EMULATE_GLES_VERSION_STRING_FORMAT=0")

	# Reduce WebGL code size at the expense of performance (this only has an effect in WebGL 1, practically a no-op here)
	append_linker_flags("-sGL_POOL_TEMP_BUFFERS=0")

	# Reduce WebGL code size: WebGL bindings layer should not keep track of certain WebGL
	# errors that are only meaningful for C/C++ applications. (good to enable for release when glGetError() is not used, but disable in debug)
	append_linker_flags("-sGL_TRACK_ERRORS=0")

	# Reduce WebGL code size: do not emit code for extensions that we might not need.
	append_linker_flags("-sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0")

	# Optimization flag to optimize aggressively for size. (other options -Os, -O3, -O2, -O1, -O0)
	append_linker_flags("-Oz")

	# Reduce code size: We do not need native POSIX filesystem emulation support (Emscripten FS/MEMFS)
	append_linker_flags("-sFILESYSTEM=0")
endif()

file(GLOB_RECURSE sources *.cpp *.c *.h)
add_executable(minimal_webgl ${sources})

file(GLOB_RECURSE assets *.png)
foreach(asset ${assets})
	file(COPY "${asset}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
endforeach()
