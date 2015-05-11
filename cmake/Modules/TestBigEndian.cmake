#.rst:
# TestBigEndian
# -------------
#
# The TestBigEndian.cmake module that ships with CMake, which
# checks if the system is big endian or little endian, assumes
# that a binary is produced that will have bytes that correspond to the
# endianness on the target system. Since emscripten produces Javascript, we
# override the default behavior and always return little endian.
#
# ::
#
#   TEST_BIG_ENDIAN(VARIABLE)
#   VARIABLE - variable to store the result to
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

function(TEST_BIG_ENDIAN VARIABLE)
  set(${VARIABLE} 0 CACHE INTERNAL "Result of TEST_BIG_ENDIAN" FORCE)
endfunction()
