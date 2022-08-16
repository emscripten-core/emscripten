MAJOR = 1
MINOR = 4
BUILD = 0

JP3D_MAJOR = 1
JP3D_MINOR = 3
JP3D_BUILD = 0

prefix=/usr/local
CC = gcc
AR = ar
#
#Set this to no if you do no want to compile/install shared libs.
ENABLE_SHARED = yes
#
#Set to yes if you BOTH have the library AND the header
#Set to no if a file is missing or you hate it.
#Either lcms or lcms2 : not both
#==== HAVE YOU CREATED opj_config.h FROM opj_config.h.in.user ? ====
#==== SHOULD BE IN SYNC WITH opj_config.h ====
WITH_LCMS1 = no
WITH_LCMS2 = no
WITH_PNG = no
WITH_TIFF = no
#
# Set to yes if you want compile/install 
#    jpwl libraries/binaries
#    jp3d libraries/binaries
WITH_JPWL = no
WITH_JP3D = no
#
#Set to yes if you have doxygen installed
#Set to no if doxygen is missing.
HAS_DOXYGEN = no

#Check whether these paths are correct; change them appropiatly.
LCMS1_INCLUDE = -I/usr/include
LCMS2_INCLUDE = -I/usr/include
PNG_INCLUDE = -I/usr/include
TIFF_INCLUDE = -I/usr/include

LCMS1_LIB = -L/usr/lib -llcms
LCMS2_LIB = -L/usr/lib -llcms2
PNG_LIB = -L/usr/lib -lpng -lz
#tiff with jpeg/jbig support?
JBIG_LIB = -ljbig
JBIG85_LIB = -ljbig85
JPEG_LIB = -ljpeg
TIFF_LIB = -L/usr/lib -ltiff $(JPEG_LIB) $(JBIG_LIB) $(JBIG85_LIB)
