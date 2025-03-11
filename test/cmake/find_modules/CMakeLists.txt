cmake_minimum_required(VERSION 3.16)
project(find_modules)

add_executable(test_prog test.c)

find_package(OpenGL REQUIRED)
target_include_directories(test_prog PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(test_prog ${OPENGL_LIBRARIES})
message(STATUS " test: OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

find_package(OpenAL REQUIRED)
target_include_directories(test_prog PRIVATE ${OPENAL_INCLUDE_DIR})
target_link_libraries(test_prog ${OPENAL_LIBRARY})
message(STATUS " test: OPENAL_LIBRARIES: ${OPENAL_LIBRARIES}")

find_package(SDL2 REQUIRED)
target_include_directories(test_prog PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(test_prog SDL2::SDL2)
message(STATUS " test: SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(STATUS " test: SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")

get_target_property(TEST_OPENGL_GL_LIBNAME OpenGL::GL IMPORTED_LIBNAME)
message(STATUS " test: OpenGL::GL IMPORTED_LIBNAME: ${TEST_OPENGL_GL_LIBNAME}")

get_target_property(TEST_OPENGL_GL_INCLUDES OpenGL::GL INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS " test: OpenGL::GL INTERFACE_INCLUDE_DIRECTORIES: ${TEST_OPENGL_GL_INCLUDES}")

find_package(SDL3 REQUIRED)
add_executable(test_prog_sdl3 test_sdl3.c)
target_link_libraries(test_prog_sdl3 SDL3::SDL3)
