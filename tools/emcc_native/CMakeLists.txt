cmake_minimum_required(VERSION 3.20)
project(emcc_native CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "Default install prefix to Emscripten root" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /WX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Werror)
endif()

enable_testing()

add_library(native_launcher_lib OBJECT
    config.cpp
    driver.cpp
    exec.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Build `emcc` native launcher
add_executable(emcc main.cpp)
target_link_libraries(emcc PRIVATE native_launcher_lib)

# Create `em++` executable (symlink on Unix, copy on Windows)
if(WIN32)
  set(CREATE_EMXX_COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:emcc> $<TARGET_FILE_DIR:emcc>/em++$<TARGET_FILE_SUFFIX:emcc>)
else()
  set(CREATE_EMXX_COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:emcc> $<TARGET_FILE_DIR:emcc>/em++$<TARGET_FILE_SUFFIX:emcc>)
endif()

add_custom_command(
    TARGET emcc POST_BUILD
    COMMAND ${CREATE_EMXX_COMMAND}
    COMMENT "Creating em++ launcher executable"
)

# Build unit tests
add_executable(native_tests tests/test_native.cpp)
target_link_libraries(native_tests PRIVATE native_launcher_lib)

add_test(NAME native_tests COMMAND native_tests)

install(TARGETS emcc DESTINATION bin)
install(PROGRAMS $<TARGET_FILE_DIR:emcc>/em++$<TARGET_FILE_SUFFIX:emcc> DESTINATION bin)
