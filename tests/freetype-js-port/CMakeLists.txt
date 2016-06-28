# CMakeLists.txt
#
# Copyright 2013-2016 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# Written originally by John Cary <cary@txcorp.com>
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
#
#
# As a preliminary, create a compilation directory and change into it, for
# example
#
#   mkdir ~/freetype2.compiled
#   cd ~/freetype2.compiled
#
# Now you can say
#
#   cmake <path-to-freetype2-src-dir>
#
# to create a Makefile that builds a static version of the library.
#
# For a dynamic library, use
#
#   cmake <path-to-freetype2-src-dir> -D BUILD_SHARED_LIBS:BOOL=true
#
# For a framework on OS X, use
#
#   cmake <path-to-freetype2-src-dir> -D BUILD_FRAMEWORK:BOOL=true -G Xcode
#
# instead.
#
# For an iOS static library, use
#
#   cmake <path-to-freetype2-src-dir> -D IOS_PLATFORM=OS -G Xcode
#
# or
#
#   cmake <path-to-freetype2-src-dir> -D IOS_PLATFORM=SIMULATOR -G Xcode
#
# Please refer to the cmake manual for further options, in particular, how
# to modify compilation and linking parameters.
#
# Some notes.
#
# . `cmake' creates configuration files in
#
#     <build-directory>/include/freetype/config
#
#   which should be further modified if necessary.
#
# . You can use `cmake' directly on a freshly cloned FreeType git
#   repository.
#
# . `CMakeLists.txt' is provided as-is since it is normally not used by the
#   developer team.
#
# . If you want to disable the automatic generation of the distribution
#   targets, add the `-D FREETYPE_NO_DIST=true' command line argument.
#
# . Set the `WITH_ZLIB', `WITH_BZip2', `WITH_PNG', and `WITH_HarfBuzz'
#   CMake variables to `ON' or `OFF' to force or skip using a dependency.
#   Leave a variable undefined (which is the default) to use the dependency
#   only if it is available.  Example:
#
#     cmake ... -DWITH_ZLIB=ON -DWITH_HarfBuzz=OFF ...


cmake_minimum_required(VERSION 2.6)


include(CheckIncludeFile)


# CMAKE_TOOLCHAIN_FILE must be set before `project' is called, which
# configures the base build environment and references the toolchain file
if (APPLE)
  if (DEFINED IOS_PLATFORM)
    if (NOT "${IOS_PLATFORM}" STREQUAL "OS"
        AND NOT "${IOS_PLATFORM}" STREQUAL "SIMULATOR")
      message(FATAL_ERROR
        "IOS_PLATFORM must be set to either OS or SIMULATOR")
    endif ()
    if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
      message(AUTHOR_WARNING
        "You should use Xcode generator with IOS_PLATFORM enabled to get Universal builds.")
    endif ()
    if (BUILD_SHARED_LIBS)
      message(FATAL_ERROR
        "BUILD_SHARED_LIBS can not be on with IOS_PLATFORM enabled")
    endif ()
    if (BUILD_FRAMEWORK)
      message(FATAL_ERROR
        "BUILD_FRAMEWORK can not be on with IOS_PLATFORM enabled")
    endif ()

    # iOS only uses static libraries
    set(BUILD_SHARED_LIBS OFF)

    set(CMAKE_TOOLCHAIN_FILE
      ${PROJECT_SOURCE_DIR}/builds/cmake/iOS.cmake)
  endif ()
else ()
  if (DEFINED IOS_PLATFORM)
    message(FATAL_ERROR "IOS_PLATFORM is not supported on this platform")
  endif ()
endif ()


project(freetype)


if (WIN32 AND NOT MINGW AND BUILD_SHARED_LIBS)
  message(FATAL_ERROR "Building shared libraries on Windows needs MinGW")
endif ()

# Disallow in-source builds
if ("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
  message(FATAL_ERROR
    "
In-source builds are not permitted!  Make a separate folder for"
    " building, e.g.,"
    "
  mkdir build; cd build; cmake .."
    "
Before that, remove the files created by this failed run with"
    "
  rm -rf CMakeCache.txt CMakeFiles")
endif ()


# Add local cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/builds/cmake)


if (BUILD_FRAMEWORK)
  if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
    message(FATAL_ERROR
      "You should use Xcode generator with BUILD_FRAMEWORK enabled")
  endif ()
  set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_32_64_BIT)")
  set(BUILD_SHARED_LIBS ON)
endif ()


set(VERSION_MAJOR "2")
set(VERSION_MINOR "6")
set(VERSION_PATCH "3")

set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(SHARED_LIBRARY_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})


# Compiler definitions for building the library
add_definitions(-DFT2_BUILD_LIBRARY)


# Find dependencies
foreach (d ZLIB BZip2 PNG HarfBuzz)
  string(TOUPPER "${d}" D)

  if (DEFINED WITH_${d} OR DEFINED WITH_${D})
    if (WITH_${d} OR WITH_${D})
      find_package(${d} QUIET REQUIRED)
    endif ()
  else ()
    find_package(${d} QUIET)
  endif ()

  if (${d}_FOUND OR ${D}_FOUND)
    message(STATUS "Building with ${d}")
  endif()
endforeach ()


message(STATUS
  "Creating directory ${PROJECT_BINARY_DIR}/include/freetype/config")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/freetype/config")


# Create the configuration file
message(STATUS
  "Creating file ${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h")

if (UNIX)
  check_include_file("unistd.h" HAVE_UNISTD_H)
  check_include_file("fcntl.h" HAVE_FCNTL_H)
  check_include_file("stdint.h" HAVE_STDINT_H)

  file(READ "${PROJECT_SOURCE_DIR}/builds/unix/ftconfig.in"
    FTCONFIG_H)
  if (HAVE_UNISTD_H)
    string(REGEX REPLACE
      "#undef +(HAVE_UNISTD_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  if (HAVE_FCNTL_H)
    string(REGEX REPLACE
      "#undef +(HAVE_FCNTL_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  if (HAVE_STDINT_H)
    string(REGEX REPLACE
      "#undef +(HAVE_STDINT_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  string(REPLACE "/undef " "#undef "
    FTCONFIG_H "${FTCONFIG_H}")
  file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h"
    "${FTCONFIG_H}")
else ()
  file(READ "${PROJECT_SOURCE_DIR}/include/freetype/config/ftconfig.h"
    FTCONFIG_H)
  file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h"
    "${FTCONFIG_H}")
endif ()


# Create the options file
message(STATUS
  "Creating file ${PROJECT_BINARY_DIR}/include/freetype/config/ftoption.h")

file(READ "${PROJECT_SOURCE_DIR}/include/freetype/config/ftoption.h"
  FTOPTION_H)
if (ZLIB_FOUND)
  string(REGEX REPLACE
    "/\\* +(#define +FT_CONFIG_OPTION_SYSTEM_ZLIB) +\\*/" "\\1"
    FTOPTION_H "${FTOPTION_H}")
endif ()
if (BZIP2_FOUND)
  string(REGEX REPLACE
    "/\\* +(#define +FT_CONFIG_OPTION_USE_BZIP2) +\\*/" "\\1"
    FTOPTION_H "${FTOPTION_H}")
endif ()
if (PNG_FOUND)
  string(REGEX REPLACE
    "/\\* +(#define +FT_CONFIG_OPTION_USE_PNG) +\\*/" "\\1"
    FTOPTION_H "${FTOPTION_H}")
endif ()
if (HARFBUZZ_FOUND)
  string(REGEX REPLACE
    "/\\* +(#define +FT_CONFIG_OPTION_USE_HARFBUZZ) +\\*/" "\\1"
    FTOPTION_H "${FTOPTION_H}")
endif ()
file(WRITE "${PROJECT_BINARY_DIR}/include/freetype/config/ftoption.h"
  "${FTOPTION_H}")


# Specify library include directories
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(BEFORE "${PROJECT_BINARY_DIR}/include")


file(GLOB PUBLIC_HEADERS "include/ft2build.h" "include/freetype/*.h")
file(GLOB PUBLIC_CONFIG_HEADERS "include/freetype/config/*.h")
file(GLOB PRIVATE_HEADERS "include/freetype/internal/*.h")


set(BASE_SRCS
  src/autofit/autofit.c
  src/base/ftbase.c
  src/base/ftbbox.c
  src/base/ftbdf.c
  src/base/ftbitmap.c
  src/base/ftcid.c
  src/base/ftfntfmt.c
  src/base/ftfstype.c
  src/base/ftgasp.c
  src/base/ftglyph.c
  src/base/ftgxval.c
  src/base/ftinit.c
  src/base/ftlcdfil.c
  src/base/ftmm.c
  src/base/ftotval.c
  src/base/ftpatent.c
  src/base/ftpfr.c
  src/base/ftstroke.c
  src/base/ftsynth.c
  src/base/ftsystem.c
  src/base/fttype1.c
  src/base/ftwinfnt.c
  src/bdf/bdf.c
  src/bzip2/ftbzip2.c
  src/cache/ftcache.c
  src/cff/cff.c
  src/cid/type1cid.c
  src/gzip/ftgzip.c
  src/lzw/ftlzw.c
  src/pcf/pcf.c
  src/pfr/pfr.c
  src/psaux/psaux.c
  src/pshinter/pshinter.c
  src/psnames/psnames.c
  src/raster/raster.c
  src/sfnt/sfnt.c
  src/smooth/smooth.c
  src/truetype/truetype.c
  src/type1/type1.c
  src/type42/type42.c
  src/winfonts/winfnt.c
)

if (WIN32)
  set(BASE_SRCS ${BASE_SRCS} builds/windows/ftdebug.c)
elseif (WINCE)
  set(BASE_SRCS ${BASE_SRCS} builds/wince/ftdebug.c)
else ()
  set(BASE_SRCS ${BASE_SRCS} src/base/ftdebug.c)
endif ()


if (BUILD_FRAMEWORK)
  set(BASE_SRCS
    ${BASE_SRCS}
    builds/mac/freetype-Info.plist
  )
endif ()

set(CMAKE_DEBUG_POSTFIX d)

add_library(freetype
  ${PUBLIC_HEADERS}
  ${PUBLIC_CONFIG_HEADERS}
  ${PRIVATE_HEADERS}
  ${BASE_SRCS}
)


if (BUILD_SHARED_LIBS)
  set_target_properties(freetype PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${SHARED_LIBRARY_VERSION}
    COMPILE_DEFINITIONS freetype_EXPORTS
  )
endif ()


if (BUILD_FRAMEWORK)
  set_property(SOURCE ${PUBLIC_CONFIG_HEADERS}
    PROPERTY MACOSX_PACKAGE_LOCATION Headers/config
  )
  set_target_properties(freetype PROPERTIES
    FRAMEWORK TRUE
    MACOSX_FRAMEWORK_INFO_PLIST builds/mac/freetype-Info.plist
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
  )
endif ()

if (NOT CMAKE_VERSION VERSION_LESS 2.8.11)
  target_include_directories(freetype
    PUBLIC $<INSTALL_INTERFACE:include/freetype2>)
endif ()

if (CMAKE_VERSION VERSION_LESS 2.8.12)
  set(MAYBE_PRIVATE "")
else ()
  set(MAYBE_PRIVATE "PRIVATE")
endif ()

if (ZLIB_FOUND)
  target_link_libraries(freetype ${MAYBE_PRIVATE} ${ZLIB_LIBRARIES})
  include_directories(${ZLIB_INCLUDE_DIRS})
endif ()
if (BZIP2_FOUND)
  target_link_libraries(freetype ${MAYBE_PRIVATE} ${BZIP2_LIBRARIES})
  include_directories(${BZIP2_INCLUDE_DIR}) # not BZIP2_INCLUDE_DIRS
endif ()
if (PNG_FOUND)
  add_definitions(${PNG_DEFINITIONS})
  target_link_libraries(freetype ${MAYBE_PRIVATE} ${PNG_LIBRARIES})
  include_directories(${PNG_INCLUDE_DIRS})
endif ()
if (HARFBUZZ_FOUND)
  target_link_libraries(freetype ${MAYBE_PRIVATE} ${HARFBUZZ_LIBRARIES})
  include_directories(${HARFBUZZ_INCLUDE_DIRS})
endif ()


# Installations
# Note the trailing slash in the argument to the `DIRECTORY' directive
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include/freetype2
  PATTERN "internal" EXCLUDE
  PATTERN "ftconfig.h" EXCLUDE
  PATTERN "ftoption.h" EXCLUDE
)
install(FILES
  ${PROJECT_BINARY_DIR}/include/freetype/config/ftconfig.h
  ${PROJECT_BINARY_DIR}/include/freetype/config/ftoption.h
  DESTINATION include/freetype2/freetype/config
)
install(TARGETS freetype
  EXPORT freetype-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Library/Frameworks
)
install(EXPORT freetype-targets
  DESTINATION lib/cmake/freetype
  FILE freetype-config.cmake
)


# Packaging
# CPack version numbers for release tarball name.
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH}})
if (NOT DEFINED CPACK_PACKAGE_DESCRIPTION_SUMMARY)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CMAKE_PROJECT_NAME}")
endif ()
if (NOT DEFINED CPACK_SOURCE_PACKAGE_FILE_NAME)
  set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}-r${PROJECT_REV}"
    CACHE INTERNAL "tarball basename"
  )
endif ()
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES
  "/CVS/;/.svn/;.swp$;.#;/#;/build/;/serial/;/ser/;/parallel/;/par/;~;/preconfig.out;/autom4te.cache/;/.config")
set(CPACK_GENERATOR TGZ)
include(CPack)


# Add `make dist' target if FREETYPE_DIST is set (which is the default)
if (NOT DEFINED FREETYPE_NO_DIST)
  add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
endif ()

# eof
