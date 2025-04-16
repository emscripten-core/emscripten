SET(VAR HAVE_VISIBILITY)

IF(NOT DEFINED ${VAR})
  SET(SOURCE
"void __attribute__((visibility(\"default\"))) test() {}
#ifdef __INTEL_COMPILER
#error ICC breaks with binutils and visibility
#endif
int main(){}
")
  FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeTmp/src.c" "${SOURCE}")

  MESSAGE(STATUS "Performing Test ${VAR}")
  TRY_COMPILE(${VAR}
              ${CMAKE_BINARY_DIR}
              ${CMAKE_BINARY_DIR}/CMakeTmp/src.c
              CMAKE_FLAGS
              "-DCOMPILE_DEFINITIONS:STRING=-fvisibility=hidden"
              OUTPUT_VARIABLE OUTPUT)

  WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeOutput.log
             "Performing manual C SOURCE FILE Test ${VAR} with the following output:\n"
             "${OUTPUT}\n"
             "Source file was:\n${SOURCE}\n" APPEND)

  SET(${VAR} ${${VAR}} CACHE INTERNAL "Test Visibility")
  IF(${VAR})
    MESSAGE(STATUS "Performing Test ${VAR} - Success")
  ELSE(${VAR})
    MESSAGE(STATUS "Performing Test ${VAR} - Failed")
  ENDIF(${VAR})
ENDIF(NOT DEFINED ${VAR})

IF(${VAR})
  ADD_DEFINITIONS(-fvisibility=hidden)
  ADD_DEFINITIONS(-DHAVE_GCC_VISIBILITY)
ENDIF(${VAR})
