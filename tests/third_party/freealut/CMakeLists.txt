# cmake project file by Prakash Punnoor
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/admin/CMakeModules")
PROJECT(Alut C)
SET(PACKAGE "freealut")
SET(PACKAGE_TARNAME "freealut")
SET(PACKAGE_NAME "freealut library")
SET(PACKAGE_MAJOR_VERSION "1")
SET(PACKAGE_MINOR_VERSION "0")
SET(PACKAGE_BUILD_VERSION "1")
SET(PACKAGE_VERSION "${PACKAGE_MAJOR_VERSION}.${PACKAGE_MINOR_VERSION}.${PACKAGE_BUILD_VERSION}")
SET(MAJOR_VERSION "0")
SET(MINOR_VERSION "0")
SET(BUILD_VERSION "0")
SET(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_BUGREPORT "openal-devel@opensource.creative.com")
SET(OPERATING_SYSTEM "${CMAKE_SYSTEM_NAME}")

INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckCSourceCompiles.cmake)

FIND_PACKAGE(ConfigHelper)

SET(ALUT_SOURCES src/alutBufferData.c
                 src/alutCodec.c
                 src/alutError.c
                 src/alutInit.c
                 src/alutInputStream.c
                 src/alutInternal.h
                 src/alutLoader.c
                 src/alutOutputStream.c
                 src/alutUtil.c
                 src/alutVersion.c
                 src/alutWaveform.c)

INCLUDE_DIRECTORIES(${Alut_SOURCE_DIR}/include)

SET(ADD_WFLAGS "")
SET(ADD_CFLAGS "")
SET(ADD_LDFLAGS "")
SET(ADD_LIBS "")

OPTION(BUILD_STATIC    "build static library too"   ON)
OPTION(PROFILE         "enable profile"            OFF)
OPTION(OPTIMIZATION    "enable optimization"        ON)
OPTION(WARNINGS        "enable warnings"            ON)
IF(WARNINGS)
  OPTION(MORE_WARNINGS "enable more warnings"       OFF)
  OPTION(USE_WERROR    "enable fail on all warning" OFF)
ENDIF(WARNINGS)

#want test-suit?
OPTION(BUILD_TESTS "build the test-suite" OFF)


ADD_DEFINE(ALUT_BUILD_LIBRARY)
# We could possibly need struct timespec and random(), which are not ANSI.
# Define to 500 if Single Unix conformance is wanted, 600 for sixth revision.
ADD_DEFINE("_XOPEN_SOURCE 500")
# We might need nanosleep, which is a POSIX IEEE Std 1003.1b-1993 feature.
# Define to the POSIX version that should be used.
ADD_DEFINE("_POSIX_C_SOURCE 199309")
# Without __NO_CTYPE tolower and friends are macros which introduce a GLIBC 2.3
# dependency. By defining this identifier we are currently backwards compatible
# to GLIBC 2.1.3, which is a good thing. In addition, the macros lead to code
# which triggers warnings with -Wunreachable-code.
ADD_DEFINE("__NO_CTYPE 1")

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DNDEBUG)

if (EMSCRIPTEN)
  # Emscripten cannot use FIND_LIBRARY, since that seaches for linkable library files, but Emscripten supports
  # OpenAL in core and there are no library files to link against. Instead, use a custom OpenAL package finder
  # provided by Emscripten.
  FIND_PACKAGE(OpenAL)

  # All the include file checks bloew are no-ops for Emscripten, so for that platform, just define the flags we support.
  SET(HAVE_STDINT_H 1)
  SET(HAVE_STAT 1)
  SET(HAVE_NANOSLEEP 1)
  SET(HAVE_TIME_H 1)
  ADD_DEFINITIONS(-DHAVE_STDINT_H=1 -DHAVE_STAT=1 -DHAVE_NANOSLEEP=1 -DHAVE_TIME_H=1)
else()
  FIND_LIBRARY(OPENAL_LIB NAMES openal openal32 PATHS /usr/lib /usr/local/lib ${OPENAL_LIB_DIR})

  IF(OPENAL_LIB MATCHES "NOTFOUND")
    MESSAGE(FATAL_ERROR "OpenAL not installed, cannot build alut - aborting.")
  ENDIF(OPENAL_LIB MATCHES "NOTFOUND")

  IF(UNIX)
    SET(ADD_LIBS ${ADD_LIBS} m)
  ENDIF(UNIX)
endif()

SET(CMAKE_REQUIRED_INCLUDES ${OPENAL_INCLUDE_DIR})

if (NOT EMSCRIPTEN) # Emscripten is a cross-compiler and cannot verify paths of include files with CHECK_INCLUDE_FILES, since that requires building native executables.
  CHECK_INCLUDE_FILES("AL/alc.h;AL/al.h" AL_HEADERS)
  IF(NOT AL_HEADERS)
    MESSAGE(FATAL_ERROR "OpenAL header files not found - aborting.")
  ENDIF(NOT AL_HEADERS)
endif()

IF(DEFINED OPENAL_INCLUDE_DIR)
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
ENDIF(DEFINED OPENAL_INCLUDE_DIR)

FIND_PACKAGE(SleepFunction)

CHECK_INCLUDE_FILE_DEFINE(stdint.h HAVE_STDINT_H)

IF(WIN32)
  CHECK_INCLUDE_FILE_DEFINE(basetsd.h HAVE_BASETSD_H)
ENDIF(WIN32)

# FindSleepFunction will check for unistd.h if time.h wasn't found
IF(HAVE_TIME_H)
  CHECK_INCLUDE_FILE_DEFINE(unistd.h HAVE_UNISTD_H)
ENDIF(HAVE_TIME_H)

IF(HAVE_UNISTD_H)
  CHECK_FUNCTION_DEFINE("
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  " stat "(\"\", (struct stat*)0)" HAVE_STAT)
ENDIF(HAVE_UNISTD_H)

IF(NOT HAVE_STAT)
  CHECK_FUNCTION_DEFINE("
  #include <sys/types.h>
  #include <sys/stat.h>
  " _stat "(\"\", (struct _stat*)0)" HAVE__STAT)
ENDIF(NOT HAVE_STAT)

# compiler specific settings
FIND_PACKAGE(CompilerAttribute)
FIND_PACKAGE(CompilerVisibility)
FIND_PACKAGE(CompilerFlagsSet)

GENERATE_CONFIG_H()

SET(CMAKE_C_FLAGS "${ADD_WFLAGS} ${ADD_CFLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${ADD_CFLAGS} ${CMAKE_CXX_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${ADD_LDFLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS "${ADD_LDFLAGS} ${CMAKE_MODULE_LINKER_FLAGS}")

IF(BUILD_STATIC)
  # we can't create a static library with the same name
  # as the shared one, so we copy it over after creation
  ADD_LIBRARY(alut_static STATIC ${ALUT_SOURCES})
  TARGET_LINK_LIBRARIES(alut_static ${OPENAL_LIB} ${ADD_LIBS})
  IF(NOT WIN32)
    ADD_CUSTOM_COMMAND(
      TARGET alut_static
      POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy
        ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}alut_static${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}alut${CMAKE_STATIC_LIBRARY_SUFFIX})
    INSTALL_FILES(/lib FILES ${CMAKE_STATIC_LIBRARY_PREFIX}alut${CMAKE_STATIC_LIBRARY_SUFFIX})
  ENDIF(NOT WIN32)
ENDIF(BUILD_STATIC)


ADD_LIBRARY(alut SHARED ${ALUT_SOURCES})
SET_TARGET_PROPERTIES(alut PROPERTIES VERSION ${VERSION} SOVERSION ${MAJOR_VERSION})
TARGET_LINK_LIBRARIES(alut ${OPENAL_LIB} ${ADD_LIBS})

INSTALL_TARGETS(/lib alut)
INSTALL_FILES(/include/AL FILES include/AL/alut.h)

# needed for openal.pc.in and openal-config.in
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix "\${prefix}")
SET(libdir "\${exec_prefix}/lib")
SET(bindir "\${exec_prefix}/bin")
SET(includedir "\${prefix}/include")
SET(requirements "")
# static linking dependecies are broken, so pthread isn't needed currently
SET(PTHREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/admin/pkgconfig/freealut.pc.in
               ${CMAKE_BINARY_DIR}/admin/pkgconfig/freealut.pc @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/admin/pkgconfig/freealut-config.in
               ${CMAKE_BINARY_DIR}/admin/pkgconfig/freealut-config @ONLY)
INSTALL_PROGRAMS(/bin FILES admin/pkgconfig/freealut-config)
INSTALL_FILES(/lib/pkgconfig FILES admin/pkgconfig/freealut.pc)


# test-suite
IF(BUILD_TESTS)
    # examples
    ADD_EXECUTABLE(hello_world examples/hello_world.c)
    if (EMSCRIPTEN)
      TARGET_LINK_LIBRARIES(hello_world ${OPENAL_LIB} ${ADD_LIBS} alut_static)
    else()
      TARGET_LINK_LIBRARIES(hello_world ${OPENAL_LIB} ${ADD_LIBS} alut)
    endif()

    ADD_EXECUTABLE(playfile examples/playfile.c)
    if (EMSCRIPTEN)
      TARGET_LINK_LIBRARIES(playfile ${OPENAL_LIB} ${ADD_LIBS} alut_static)
    else()
      TARGET_LINK_LIBRARIES(playfile ${OPENAL_LIB} ${ADD_LIBS} alut)
    endif()


    SET(TESTS errorstuff
              fileloader
              memoryloader
              version
              waveforms)

    FOREACH(TEST ${TESTS})
      SET(TEST_SRC test_suite/test_${TEST}.c)
      SET_SOURCE_FILES_PROPERTIES(${TEST_SRC} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
      ADD_EXECUTABLE(test_${TEST} ${TEST_SRC})
      TARGET_LINK_LIBRARIES(test_${TEST} ${OPENAL_LIB} ${ADD_LIBS} alut)
    ENDFOREACH(TEST)

    #copy over testdata, so test-suite can be used in binary dir
    SET(TESTDATA file1.wav
                 file2.au
                 file3.raw)

    FOREACH(TESTDATUM ${TESTDATA})
      CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/test_suite/${TESTDATUM} ${CMAKE_BINARY_DIR}/${TESTDATUM} COPYONLY)
    ENDFOREACH(TESTDATUM)
ENDIF(BUILD_TESTS)
