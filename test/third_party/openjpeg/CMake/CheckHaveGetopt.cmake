# Check if getopt is present:
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
SET(DONT_HAVE_GETOPT 1)
IF(UNIX) #I am pretty sure only *nix sys have this anyway
  CHECK_INCLUDE_FILE("getopt.h" CMAKE_HAVE_GETOPT_H)
  # Seems like we need the contrary:
  IF(CMAKE_HAVE_GETOPT_H)
    SET(DONT_HAVE_GETOPT 0)
  ENDIF(CMAKE_HAVE_GETOPT_H)
ENDIF(UNIX)

IF(DONT_HAVE_GETOPT)
  ADD_DEFINITIONS(-DDONT_HAVE_GETOPT)
ENDIF(DONT_HAVE_GETOPT)

