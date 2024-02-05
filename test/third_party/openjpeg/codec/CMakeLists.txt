  # Build the demo app, small examples

# First thing define the common source:
SET(common_SRCS
  convert.c
  index.c
  ${OPENJPEG_SOURCE_DIR}/common/color.c
)

if (NOT CMAKE_HAVE_GETOPT_H)
  SET(common_SRCS ${common_SRCS} ${OPENJPEG_SOURCE_DIR}/common/getopt.c)
endif()

# Headers file are located here:
INCLUDE_DIRECTORIES(
  ${OPENJPEG_SOURCE_DIR}/libopenjpeg
  ${LCMS_INCLUDE_DIR}
  ${OPENJPEG_SOURCE_DIR}/common
  )
IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ENDIF(PNG_FOUND)
IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)

IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_EXPORTS)
  ELSE(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DOPJ_STATIC)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

# Loop over all executables:
FOREACH(exe j2k_to_image image_to_j2k j2k_dump)
  ADD_EXECUTABLE(${exe} ${exe}.c ${common_SRCS})
  TARGET_LINK_LIBRARIES(${exe} ${OPENJPEG_LIBRARY_NAME} ${LCMS_LIB})
  IF(PNG_FOUND)
    TARGET_LINK_LIBRARIES(${exe} ${PNG_LIBRARIES})
  ENDIF(PNG_FOUND)
  IF(TIFF_FOUND)
    TARGET_LINK_LIBRARIES(${exe} ${TIFF_LIBRARIES})
  ENDIF(TIFF_FOUND)
  ADD_TEST(${exe} ${EXECUTABLE_OUTPUT_PATH}/${exe})
  # calling those exe without option will make them fail always:
  SET_TESTS_PROPERTIES(${exe} PROPERTIES WILL_FAIL TRUE)
  # On unix you need to link to the math library:
  IF(UNIX)
    TARGET_LINK_LIBRARIES(${exe} m)
  ENDIF(UNIX)
  # Install exe
  INSTALL(TARGETS ${exe}
    EXPORT OpenJPEGTargets
    DESTINATION ${OPENJPEG_INSTALL_BIN_DIR} COMPONENT Applications
  )
ENDFOREACH(exe)

# Install man pages
INSTALL(
  FILES       ../doc/man/man1/image_to_j2k.1
              ../doc/man/man1/j2k_dump.1
              ../doc/man/man1/j2k_to_image.1
  DESTINATION ${OPENJPEG_INSTALL_MAN_DIR}/man1)
#

if(BUILD_TESTING)
# Do testing here, once we know the examples are being built:
FILE(GLOB_RECURSE OPENJPEG_DATA_IMAGES_GLOB
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.j2k"
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.j2c"
  "${JPEG2000_CONFORMANCE_DATA_ROOT}/*.jp2"
  )

foreach(filename ${OPENJPEG_DATA_IMAGES_GLOB})
  get_filename_component(filename_temp ${filename} NAME)
  get_filename_component(filename_ext ${filename} EXT)
  execute_process(COMMAND ${EXECUTABLE_OUTPUT_PATH}/j2k_dump -i ${filename}
    OUTPUT_VARIABLE dump_success
    OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${filename_temp}.dump
    ERROR_QUIET
  )
  if(dump_success)
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/${filename_temp}.dump numcomp_file)
  string(REGEX REPLACE ".*numcomps=([0-9]+).*" "\\1"
    numcomps "${numcomp_file}")
  #message( "found:${output_variable} for ${filename_temp}" )
  endif()
  ADD_TEST(dump-${filename_temp} ${EXECUTABLE_OUTPUT_PATH}/j2k_dump -i ${filename})
  foreach(codec_type ppm pgx bmp tif raw tga png)
    ADD_TEST(j2i-${filename_temp}-${codec_type} ${EXECUTABLE_OUTPUT_PATH}/j2k_to_image -i ${filename} -o ${filename_temp}.${codec_type})
    ADD_TEST(i2j-${filename_temp}-${codec_type} ${EXECUTABLE_OUTPUT_PATH}/image_to_j2k -i ${filename_temp}.${codec_type} -o ${filename_temp}.${codec_type}${filename_ext})
    #if(UNIX)
    #  ADD_TEST(cmp-${filename_temp}-${codec_type} cmp ${filename} ${filename_temp}.${codec_type}${filename_ext})
    #endif(UNIX)
  endforeach(codec_type)
endforeach(filename)
endif(BUILD_TESTING)
