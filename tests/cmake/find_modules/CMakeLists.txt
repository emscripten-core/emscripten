cmake_minimum_required(VERSION 3.0)
project(find_modules)

add_executable(test_prog test.c)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(test_prog ${OPENGL_LIBRARIES})
message(" test: OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})
target_link_libraries(test_prog ${OPENAL_LIBRARY})
message(" test: OPENGL_LIBRARIES: ${OPENAL_LIBRARIES}")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(test_prog SDL2::SDL2)
message(" test: SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(" test: SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
