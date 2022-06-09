cmake_minimum_required(VERSION 3.0)

project(find_package VERSION 0.42)

find_package(Foo REQUIRED)

add_executable(Bar bar.cpp)

target_link_libraries(Bar Foo)

install(TARGETS Bar EXPORT BarTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
