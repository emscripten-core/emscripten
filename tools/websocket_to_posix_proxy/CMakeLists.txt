cmake_minimum_required(VERSION 2.8)

project(websocket_to_posix_proxy)

file(GLOB sourceFiles src/*.cpp src/*.h)

add_executable(websocket_to_posix_proxy ${sourceFiles})

find_package(Threads)
target_link_libraries(websocket_to_posix_proxy ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(/wd4200) # "nonstandard extension used: zero-sized array in struct/union"
	target_link_libraries(websocket_to_posix_proxy Ws2_32.lib)
endif()
