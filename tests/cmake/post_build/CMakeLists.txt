cmake_minimum_required(VERSION 2.8)

project(post_build)

add_executable(hello ../../hello_world.c)

add_custom_command(TARGET hello POST_BUILD
  COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:hello>
)

enable_testing()

add_test(NAME hello
  COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:hello>
)
