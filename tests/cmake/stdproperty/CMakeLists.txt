cmake_minimum_required(VERSION 3.4)
project(helloworld)

add_executable(helloworld main.cpp)

set_property(TARGET helloworld PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET helloworld PROPERTY CXX_STANDARD 11)
