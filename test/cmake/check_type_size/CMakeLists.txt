cmake_minimum_required(VERSION 3.0)

project(test_cmake)

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
