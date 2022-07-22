project(emscripten_system_processor)

# Test that EMSCRIPTEN_SYSTEM_PROCESSOR can be overridden, and that
# CMAKE_SYSTEM_PROCESSOR is set to its value.

message(STATUS "CMAKE_SYSTEM_PROCESSOR is ${CMAKE_SYSTEM_PROCESSOR}")
