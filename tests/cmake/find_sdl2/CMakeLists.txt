cmake_minimum_required(VERSION 3.0)

project(find_sdl2)

find_package(SDL2 REQUIRED)

add_executable(sdl2 sdl2.c)

target_link_libraries(sdl2 SDL2::SDL2)
