cmake_minimum_required(VERSION 3.20)
project(emcc_native CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "Default install prefix to Emscripten root" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /WX /UNDEBUG)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Werror -UNDEBUG)
endif()

add_library(native_launcher_lib OBJECT
    config.cpp
    diagnostics.cpp
    driver.cpp
    exec.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Build `emcc` native launcher
add_executable(emcc main.cpp)
target_link_libraries(emcc PRIVATE native_launcher_lib)

set(TOP_LEVEL_ENTRY_POINTS
  em++
  emar
  embuilder
  emcmake
  em-config
  emconfigure
  emmake
  emranlib
  emrun
  emscons
  emsize
  emprofile
  emdwp
  emnm
  emstrip
  emsymbolizer
  emscan-deps
  empath-split
)

set(TOOLS_ENTRY_POINTS
  file_packager
  webidl_binder
)

set(TEST_ENTRY_POINTS
  runner
)

if(WIN32)
  foreach(entry ${TOP_LEVEL_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:emcc> $<TARGET_FILE_DIR:emcc>/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating ${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION .)
  endforeach()

  foreach(entry ${TOOLS_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:emcc>/tools
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:emcc> $<TARGET_FILE_DIR:emcc>/tools/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating tools/${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/tools/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION tools)
  endforeach()

  foreach(entry ${TEST_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:emcc>/test
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:emcc> $<TARGET_FILE_DIR:emcc>/test/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating test/${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/test/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION test)
  endforeach()
else()
  foreach(entry ${TOP_LEVEL_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:emcc> $<TARGET_FILE_DIR:emcc>/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating ${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION .)
  endforeach()

  foreach(entry ${TOOLS_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:emcc>/tools
      COMMAND ${CMAKE_COMMAND} -E create_symlink ../$<TARGET_FILE_NAME:emcc> $<TARGET_FILE_DIR:emcc>/tools/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating tools/${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/tools/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION tools)
  endforeach()

  foreach(entry ${TEST_ENTRY_POINTS})
    add_custom_command(
      TARGET emcc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:emcc>/test
      COMMAND ${CMAKE_COMMAND} -E create_symlink ../$<TARGET_FILE_NAME:emcc> $<TARGET_FILE_DIR:emcc>/test/${entry}$<TARGET_FILE_SUFFIX:emcc>
      COMMENT "Creating test/${entry} launcher executable"
    )
    install(PROGRAMS $<TARGET_FILE_DIR:emcc>/test/${entry}$<TARGET_FILE_SUFFIX:emcc> DESTINATION test)
  endforeach()
endif()

install(TARGETS emcc DESTINATION .)
