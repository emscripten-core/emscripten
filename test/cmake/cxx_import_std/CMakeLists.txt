cmake_minimum_required(VERSION 4.3)

set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD 451f2fe2-a8a2-47c3-bc32-94786d8fc91b)

project(cxx_import_std)

foreach(_STD IN LISTS CMAKE_CXX_COMPILER_IMPORT_STD)
  add_executable(cxx_import_std${_STD} main.cpp)
  target_compile_features(cxx_import_std${_STD} PRIVATE cxx_std_${_STD})
endforeach()
