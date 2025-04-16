# Check if getopt is present:
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
SET(DONT_HAVE_GETOPT 1)

## Force the use of the openjpeg version of getopt_long since the system
## version has a differnet signature.  This was fixed in the more recent
## upstream version of openjpeg:
## https://github.com/uclouvain/openjpeg/commit/aba0e602
##
##
## IF(UNIX) #I am pretty sure only *nix sys have this anyway
##   CHECK_INCLUDE_FILE("getopt.h" CMAKE_HAVE_GETOPT_H)
##   # Seems like we need the contrary:
##   IF(CMAKE_HAVE_GETOPT_H)
##     SET(DONT_HAVE_GETOPT 0)
##   ENDIF(CMAKE_HAVE_GETOPT_H)
## ENDIF(UNIX)
##

IF(DONT_HAVE_GETOPT)
  ADD_DEFINITIONS(-DDONT_HAVE_GETOPT)
ENDIF(DONT_HAVE_GETOPT)

