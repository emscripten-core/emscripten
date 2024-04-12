cmake_minimum_required(VERSION 3.0)

project(threads VERSION 0.42)

find_package(Threads REQUIRED)

if (!CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "pthreads not found")
endif()

add_executable(hello hello.c)

target_link_libraries(hello ${CMAKE_THREAD_LIBS_INIT})
