cmake_minimum_required(VERSION 2.8)

project(cmake_check_c_source_runs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXECUTABLE_SUFFIX ".html")

include(CheckCSourceRuns)
check_c_source_runs("
        static int notsupported = 1;
        void test_ctor (void) __attribute__((constructor));
        void test_ctor (void) { notsupported = 0; }
        int main(void) { return (notsupported); }
        "  HAS_CONSTRUCTOR)

message("HAS_CONSTRUCTOR: ${HAS_CONSTRUCTOR}")


add_executable(cmake_check_c_source_runs main.cpp)
