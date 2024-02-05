cmake_minimum_required(VERSION 3.0)

project(test_cmake)

if (CMAKE_LIBRARY_ARCHITECTURE MATCHES "wasm64")
  set(PTR_SIZE 8)
else()
  set(PTR_SIZE 4)
endif()

if (NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "${PTR_SIZE}")
  message(FATAL_ERROR "CMAKE_SIZEOF_VOID_P is not ${PTR_SIZE}! (${CMAKE_SIZEOF_VOID_P})")
endif()
message(STATUS "CMAKE_SIZEOF_VOID_P -> ${CMAKE_SIZEOF_VOID_P}")

include(CheckTypeSize)

check_type_size("int" int_size)
if (NOT "${int_size}" EQUAL "4")
  message(FATAL_ERROR "CHECK_TYPE_SIZE with int did not return 4! (${int_size})")
endif()
message(STATUS "CHECK_TYPE_SIZE int -> ${int_size}")

check_type_size("int[256+1]" big_size)
if (NOT "${big_size}" EQUAL "1028")
  message(FATAL_ERROR "CHECK_TYPE_SIZE with int[256+1] did not return 1028! (${big_size})")
endif()
message(STATUS "CHECK_TYPE_SIZE int -> ${big_size}")

check_type_size("void*" ptr_size)
if (NOT "${ptr_size}" EQUAL "${PTR_SIZE}")
  message(FATAL_ERROR "CHECK_TYPE_SIZE with void* did not return ${PTR_SIZE}! (${ptr_size})")
endif()
message(STATUS "CHECK_TYPE_SIZE void* -> ${ptr_size}")
