cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_MODULE_STD ON)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.3.0)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD 451f2fe2-a8a2-47c3-bc32-94786d8fc91b)
else()
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD d0edc3af-4c50-42ea-a356-e2862fe7a444)
endif()

project(cxx_import_std)

foreach(STD_VERSION IN LISTS CMAKE_CXX_COMPILER_IMPORT_STD)
  add_executable(cxx_import_std${STD_VERSION} main.cpp)
  target_compile_features(cxx_import_std${STD_VERSION} PRIVATE cxx_std_${STD_VERSION})
endforeach()
