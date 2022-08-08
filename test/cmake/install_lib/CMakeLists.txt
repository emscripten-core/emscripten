cmake_minimum_required(VERSION 3.0)

project(foo VERSION 1.42)

add_library(Foo STATIC foo.c)

install(TARGETS Foo EXPORT FooTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(FILES foo.h DESTINATION include)

install(EXPORT FooTargets
  FILE FooTargets.cmake
  DESTINATION lib/cmake/Foo
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("FooConfigVersion.cmake"
  VERSION ${Foo_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "FooConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/FooConfigVersion.cmake"
  DESTINATION lib/cmake/Foo
)
