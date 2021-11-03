cmake_minimum_required(VERSION 3.0)

project(find_package VERSION 0.42)

find_package(Foo REQUIRED)

add_executable(Bar bar.c)

target_link_libraries(Bar Foo)
