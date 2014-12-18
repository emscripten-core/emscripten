cmake_minimum_required(VERSION 2.8)

project(asmjs_optimizer)

file(GLOB sourceFiles *.cpp)
file(GLOB headerFiles *.h)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES ".*(gcc|clang|emcc).*" OR CMAKE_C_COMPILER_ID MATCHES ".*(GCC|Clang|emcc).*")
	set(IS_GCC_LIKE TRUE)
else()
	set(IS_GCC_LIKE FALSE)
endif()

if (IS_GCC_LIKE)
	set(cFlags "-std=c++11 -fno-exceptions -fno-rtti")
endif()

if (MSVC)
	set(cFlags "${cFlags} -D_CRT_SECURE_NO_WARNINGS=1")
endif()

set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} ${cFlags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cFlags}")

add_executable(optimizer ${sourceFiles} ${headerFiles})
