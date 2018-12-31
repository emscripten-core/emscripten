cmake_minimum_required(VERSION 2.8.10)

project(asmjs_optimizer)

file(GLOB sourceFiles *.cpp)
file(GLOB headerFiles *.h)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES ".*(gcc|clang|emcc).*" OR CMAKE_C_COMPILER_ID MATCHES ".*(GCC|Clang|emcc).*")
	set(IS_GCC_LIKE TRUE)
else()
	set(IS_GCC_LIKE FALSE)
endif()

# -DCMAKE_CXX_FLAGS=-DPROFILING will print crude timing information to stderr
# for initial identification of areas to profile in more depth with
# CALLGRIND_{START,STOP}_INSTRUMENTATION or similar
# Don't forget to also pass -DCMAKE_BUILD_TYPE=Release to cmake or your build
# won't be optimized by the compiler!

if (IS_GCC_LIKE)
	set(cFlags "-fno-exceptions -fno-rtti")

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
		set(cFlags "${cFlags} -std=c++0x")
	else()
		set(cFlags "${cFlags} -std=c++11")
	endif()
endif()

if (MSVC)
	set(cFlags "${cFlags} -D_CRT_SECURE_NO_WARNINGS=1")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10485760")
endif()

set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} ${cFlags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cFlags}")

add_executable(optimizer ${sourceFiles} ${headerFiles})
