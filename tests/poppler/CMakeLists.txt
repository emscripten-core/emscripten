project(poppler)

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(PopplerDefaults)
include(PopplerMacros)
include(MacroOptionalFindPackage)
find_package(PkgConfig)
include(MacroEnsureVersion)
include(MacroBoolTo01)
find_package(Threads)
include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)
include(CheckFileOffsetBits)
CHECK_FILE_OFFSET_BITS()

set(POPPLER_MAJOR_VERSION "0")
set(POPPLER_MINOR_VERSION "24")
set(POPPLER_MICRO_VERSION "3")
set(POPPLER_VERSION "${POPPLER_MAJOR_VERSION}.${POPPLER_MINOR_VERSION}.${POPPLER_MICRO_VERSION}")

# command line switches
option(ENABLE_XPDF_HEADERS "Install unsupported xpdf headers." OFF)
option(BUILD_GTK_TESTS "Whether compile the GTK+ test programs." ON)
option(BUILD_QT4_TESTS "Whether compile the Qt4 test programs." ON)
option(BUILD_QT5_TESTS "Whether compile the Qt5 test programs." ON)
option(BUILD_CPP_TESTS "Whether compile the CPP test programs." ON)
option(ENABLE_SPLASH "Build the Splash graphics backend." ON)
option(ENABLE_UTILS "Compile poppler command line utils." ON)
option(ENABLE_CPP "Compile poppler cpp wrapper." ON)
option(ENABLE_LIBOPENJPEG "Use libopenjpeg for JPX streams." ON)
set(ENABLE_CMS "auto" CACHE STRING "Use color management system. Possible values: auto, lcms1, lcms2. 'auto' prefers lcms2 over lcms1 if both are available. Unset to disable color management system.")
option(ENABLE_LIBCURL "Build libcurl based HTTP support." OFF)
option(ENABLE_ZLIB "Build with zlib (not totally safe)." OFF)
option(USE_FIXEDPOINT "Use fixed point arithmetic in the Splash backend" OFF)
option(USE_FLOAT "Use single precision arithmetic in the Splash backend" OFF)
if(WIN32)
  option(ENABLE_RELOCATABLE "Do not hardcode the poppler library location (on Windows)." ON)
else(WIN32)
  set(ENABLE_RELOCATABLE OFF)
endif(WIN32)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")

set(TESTDATADIR "${CMAKE_SOURCE_DIR}/../test" CACHE STRING "Specify test data dir.")
if(NOT (EXISTS ${TESTDATADIR} AND EXISTS ${TESTDATADIR}/test-poppler.c))
  message(WARNING "
 No test data found in $testdatadir.
 You will not be able to run 'make test' successfully.

 The test data is not included in the source packages
 and is also not part of the main git repository. Instead,
 you can checkout the test data from its own git
 repository with:

   git clone git://git.freedesktop.org/git/poppler/test

 You should checkout the test data as a sibling of your
 poppler source folder or specify the location of your
 checkout with -DTESTDATADIR=/path/to/checkoutdir/test.
  ")
endif()

if(WIN32)
  set(_default_fontconfiguration "win32")
else(WIN32)
  set(_default_fontconfiguration "fontconfig")
endif(WIN32)
set(FONT_CONFIGURATION "${_default_fontconfiguration}" CACHE STRING "The font configuration backend (win32|fontconfig).")
string(TOLOWER "${FONT_CONFIGURATION}" font_configuration)
set(WITH_FONTCONFIGURATION_WIN32 OFF)
set(WITH_FONTCONFIGURATION_FONTCONFIG OFF)
if(font_configuration STREQUAL "win32")
  set(WITH_FONTCONFIGURATION_WIN32 ON)
elseif(font_configuration STREQUAL "fontconfig")
  set(WITH_FONTCONFIGURATION_FONTCONFIG ON)
else(font_configuration STREQUAL "win32")
  message(FATAL_ERROR "Invalid font configuration setting: ${FONT_CONFIGURATION}")
endif(font_configuration STREQUAL "win32")

# Enable these unconditionally.
set(MULTITHREADED ON)
set(OPI_SUPPORT ON)
set(TEXTOUT_WORD_LIST ON)

# setting the minimum required versions for some components
set(QT4_MIN_VERSION "4.4.0")
set(CAIRO_VERSION "1.10.0")
set(GLIB_REQUIRED "2.18")

macro_bool_to_01(ENABLE_SPLASH HAVE_SPLASH)
find_package(Freetype REQUIRED)
if(WITH_FONTCONFIGURATION_FONTCONFIG)
  find_package(Fontconfig REQUIRED)
endif(WITH_FONTCONFIGURATION_FONTCONFIG)
macro_optional_find_package(JPEG)
macro_optional_find_package(PNG)
macro_optional_find_package(TIFF)
if(JPEG_FOUND)
  set(ENABLE_LIBJPEG ${JPEG_FOUND})
endif(JPEG_FOUND)
macro_optional_find_package(Qt4)
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.8.7)
  find_package(Qt5Core)
  find_package(Qt5Gui)
  find_package(Qt5Xml)
  find_package(Qt5Widgets)
  find_package(Qt5Test)
  if (Qt5Core_FOUND AND Qt5Gui_FOUND AND Qt5Xml_FOUND AND Qt5Widgets_FOUND AND Qt5Test_FOUND)
    set(QT5_FOUND true)
  else ()
    message("-- Package Qt5Core or Qt5Gui or Qt5Xml or Qt5Widgets or Qt5Test not found")
    set(QT5_FOUND false)
  endif()
else(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.8.7)
  set(QT5_FOUND false)
  message("-- CMake >= 2.8.8 is needed to enable Qt5")
endif(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.8.7)

macro_optional_find_package(Cairo ${CAIRO_VERSION})
if(CAIRO_FOUND)
  set(HAVE_CAIRO ${CAIRO_FOUND})
  set(CAIRO_FEATURE "#define POPPLER_HAS_CAIRO 1")
  set(CAIRO_REQ "cairo")
  set(POPPLER_GLIB_DISABLE_DEPRECATED "")
  set(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES "")
  macro_optional_find_package(GLIB)
  if(GLIB_FOUND)
    set(ENABLE_GLIB ON)
    # Check for introspection
    macro_optional_find_package(GObjectIntrospection 0.9.12)
    set(HAVE_INTROSPECTION ${INTROSPECTION_FOUND})
    set(POPPLER_GLIB_DISABLE_DEPRECATED "${POPPLER_GLIB_DISABLE_DEPRECATED} -DG_DISABLE_DEPRECATED")
    set(POPPLER_GLIB_DISABLE_SINGLE_INCLUDES "${POPPLER_GLIB_DISABLE_SINGLE_INCLUDES} -DG_DISABLE_SINGLE_INCLUDES")
    macro_optional_find_package(GTK)
  endif(GLIB_FOUND)
else(CAIRO_FOUND)
  set(CAIRO_FEATURE "#undef POPPLER_HAS_CAIRO")
endif(CAIRO_FOUND)
if(ENABLE_CPP)
  macro_optional_find_package(Iconv)
  set(ENABLE_CPP ${ICONV_FOUND})
  set(HAVE_ICONV ${ICONV_FOUND})
endif(ENABLE_CPP)
if(ENABLE_ZLIB)
  find_package(ZLIB)
  if(ZLIB_FOUND)
    set(HAVE_LIBZ ${ZLIB_FOUND})
  endif(ZLIB_FOUND)
  set(ENABLE_ZLIB ${ZLIB_FOUND})
endif(ENABLE_ZLIB)
if(ENABLE_LIBOPENJPEG)
  find_package(LIBOPENJPEG)
  set(ENABLE_LIBOPENJPEG ${LIBOPENJPEG_FOUND})
  set(HAVE_OPENJPEG_H ON)
endif(ENABLE_LIBOPENJPEG)
if(ENABLE_CMS STREQUAL "auto")
  find_package(LCMS2)
  set(USE_CMS ${LCMS2_FOUND})
  if(NOT LCMS2_FOUND)
    find_package(LCMS)
    set(USE_CMS ${LCMS_FOUND})
    set(USE_LCMS1 ${LCMS_FOUND})
  endif(NOT LCMS2_FOUND)
elseif(ENABLE_CMS STREQUAL "lcms1")
  find_package(LCMS)
  set(USE_CMS ${LCMS_FOUND})
  set(USE_LCMS1 ${LCMS_FOUND})
elseif(ENABLE_CMS STREQUAL "lcms2")
  find_package(LCMS2)
  set(USE_CMS ${LCMS2_FOUND})
endif()
if(ENABLE_LIBCURL)
  find_package(CURL)
  include_directories(${CURL_INCLUDE_DIR})
  set(POPPLER_HAS_CURL_SUPPORT ON)
endif(ENABLE_LIBCURL)

add_definitions(-DHAVE_CONFIG_H=1)
if(MINGW)
  # Use mingw's ansi stdio extensions
  add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif(MINGW)
if(FONTCONFIG_FOUND)
  add_definitions(${FONTCONFIG_DEFINITIONS})
  include_directories(${FONTCONFIG_INCLUDE_DIR})
endif(FONTCONFIG_FOUND)
if(WITH_FONTCONFIGURATION_WIN32)
  if(MINGW)
    # Set the minimum required Internet Explorer version to 5.0
    add_definitions(-D_WIN32_IE=0x0500)
  endif(MINGW)
endif(WITH_FONTCONFIGURATION_WIN32)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/fofi
  ${CMAKE_CURRENT_SOURCE_DIR}/goo
  ${CMAKE_CURRENT_SOURCE_DIR}/poppler
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/poppler
  ${FREETYPE_INCLUDE_DIRS}
)
set(HAVE_FREETYPE_H ON)
if(CMAKE_USE_PTHREADS_INIT)
  set(HAVE_PTHREAD ON)
endif(CMAKE_USE_PTHREADS_INIT)

if(ENABLE_ZLIB)
  include_directories(${ZLIB_INCLUDE_DIR})
endif(ENABLE_ZLIB)
if(JPEG_FOUND)
  include_directories(${JPEG_INCLUDE_DIR})
  set(ENABLE_LIBJPEG ON)
endif(JPEG_FOUND)
if(PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIR})
  set(ENABLE_LIBPNG ON)
endif(PNG_FOUND)
if(TIFF_FOUND)
  include_directories(${TIFF_INCLUDE_DIR})
  set(ENABLE_LIBTIFF ON)
endif(TIFF_FOUND)
if(LIBOPENJPEG_FOUND)
  include_directories(${LIBOPENJPEG_INCLUDE_DIR})
endif(LIBOPENJPEG_FOUND)
if(LCMS_FOUND)
  include_directories(${LCMS_INCLUDE_DIR})
endif(LCMS_FOUND)
if(LCMS2_FOUND)
  include_directories(${LCMS2_INCLUDE_DIR})
endif(LCMS2_FOUND)

if(DEFINED COMPILE_WARNINGS)
else(DEFINED COMPILE_WARNINGS)
  set(COMPILE_WARNINGS "yes")
endif(DEFINED COMPILE_WARNINGS)
string(TOLOWER "${COMPILE_WARNINGS}" _comp_warnings)
if(_comp_warnings STREQUAL "no")
  set(CMAKE_CXX_FLAGS "${DEFAULT_COMPILE_WARNINGS_NO} ${CMAKE_CXX_FLAGS}")
endif(_comp_warnings STREQUAL "no")
if(_comp_warnings STREQUAL "yes")
  set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${DEFAULT_COMPILE_WARNINGS_YES} ${CMAKE_CXX_FLAGS}")
endif(_comp_warnings STREQUAL "yes")
if(_comp_warnings STREQUAL "kde")
  set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${DEFAULT_COMPILE_WARNINGS_KDE} ${CMAKE_CXX_FLAGS}")
endif(_comp_warnings STREQUAL "kde")


include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(poppler/poppler-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/poppler/poppler-config.h)


set(poppler_SRCS
  goo/gfile.cc
  goo/gmempp.cc
  goo/GooHash.cc
  goo/GooList.cc
  goo/GooTimer.cc
  goo/GooString.cc
  goo/gmem.cc
  goo/FixedPoint.cc
  goo/NetPBMWriter.cc
  goo/PNGWriter.cc
  goo/TiffWriter.cc
  goo/JpegWriter.cc
  goo/ImgWriter.cc
  goo/gstrtod.cc
  goo/grandom.cc
  fofi/FoFiBase.cc
  fofi/FoFiEncodings.cc
  fofi/FoFiTrueType.cc
  fofi/FoFiType1.cc
  fofi/FoFiType1C.cc
  fofi/FoFiIdentifier.cc
  poppler/Annot.cc
  poppler/Array.cc
  poppler/BuiltinFont.cc
  poppler/BuiltinFontTables.cc
  poppler/CachedFile.cc
  poppler/Catalog.cc
  poppler/CharCodeToUnicode.cc
  poppler/CMap.cc
  poppler/DateInfo.cc
  poppler/Decrypt.cc
  poppler/Dict.cc
  poppler/Error.cc
  poppler/FileSpec.cc
  poppler/FontEncodingTables.cc
  poppler/Form.cc
  poppler/FontInfo.cc
  poppler/Function.cc
  poppler/Gfx.cc
  poppler/GfxFont.cc
  poppler/GfxState.cc
  poppler/GlobalParams.cc
  poppler/Hints.cc
  poppler/JArithmeticDecoder.cc
  poppler/JBIG2Stream.cc
  poppler/Lexer.cc
  poppler/Link.cc
  poppler/Linearization.cc
  poppler/LocalPDFDocBuilder.cc
  poppler/NameToCharCode.cc
  poppler/Object.cc
  poppler/OptionalContent.cc
  poppler/Outline.cc
  poppler/OutputDev.cc
  poppler/Page.cc
  poppler/PageTransition.cc
  poppler/Parser.cc
  poppler/PDFDoc.cc
  poppler/PDFDocEncoding.cc
  poppler/PDFDocFactory.cc
  poppler/PopplerCache.cc
  poppler/ProfileData.cc
  poppler/PreScanOutputDev.cc
  poppler/PSTokenizer.cc
  poppler/Stream.cc
  poppler/StructTreeRoot.cc
  poppler/StructElement.cc
  poppler/strtok_r.cpp
  poppler/UnicodeMap.cc
  poppler/UnicodeTypeTable.cc
  poppler/UTF.cc
  poppler/XRef.cc
  poppler/PSOutputDev.cc
  poppler/TextOutputDev.cc
  poppler/PageLabelInfo.cc
  poppler/SecurityHandler.cc
  poppler/StdinCachedFile.cc
  poppler/StdinPDFDocBuilder.cc
  poppler/Sound.cc
  poppler/ViewerPreferences.cc
  poppler/XpdfPluginAPI.cc
  poppler/Movie.cc
  poppler/Rendition.cc
)
set(poppler_LIBS ${FREETYPE_LIBRARIES})
if(ENABLE_SPLASH)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/SplashOutputDev.cc
    splash/Splash.cc
    splash/SplashBitmap.cc
    splash/SplashClip.cc
    splash/SplashFTFont.cc
    splash/SplashFTFontEngine.cc
    splash/SplashFTFontFile.cc
    splash/SplashFont.cc
    splash/SplashFontEngine.cc
    splash/SplashFontFile.cc
    splash/SplashFontFileID.cc
    splash/SplashPath.cc
    splash/SplashPattern.cc
    splash/SplashScreen.cc
    splash/SplashState.cc
    splash/SplashT1Font.cc
    splash/SplashT1FontEngine.cc
    splash/SplashT1FontFile.cc
    splash/SplashXPath.cc
    splash/SplashXPathScanner.cc
  )
endif(ENABLE_SPLASH)
if(FONTCONFIG_FOUND)
  set(poppler_LIBS ${poppler_LIBS} ${FONTCONFIG_LIBRARIES})
endif(FONTCONFIG_FOUND)
if(JPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/DCTStream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${JPEG_LIBRARIES})
endif(JPEG_FOUND)
if(ENABLE_ZLIB)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/FlateStream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${ZLIB_LIBRARIES})
endif(ENABLE_ZLIB)
if(ENABLE_LIBCURL)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/CurlCachedFile.cc
    poppler/CurlPDFDocBuilder.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${CURL_LIBRARIES})
endif(ENABLE_LIBCURL)
if(LIBOPENJPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/JPEG2000Stream.cc
  )
  set(poppler_LIBS ${poppler_LIBS} ${LIBOPENJPEG_LIBRARIES})
else (LIBOPENJPEG_FOUND)
  set(poppler_SRCS ${poppler_SRCS}
    poppler/JPXStream.cc
  )
endif(LIBOPENJPEG_FOUND)
if(USE_CMS)
  if(LCMS_FOUND)
    set(poppler_LIBS ${poppler_LIBS} ${LCMS_LIBRARIES})
  endif(LCMS_FOUND)
  if(LCMS2_FOUND)
    set(poppler_LIBS ${poppler_LIBS} ${LCMS2_LIBRARIES})
  endif(LCMS2_FOUND)
endif(USE_CMS)
if(WIN32)
  # gdi32 is needed under win32
  set(poppler_LIBS ${poppler_LIBS} gdi32)
endif(WIN32)
if(PNG_FOUND)
  set(poppler_LIBS ${poppler_LIBS} ${PNG_LIBRARIES})
endif(PNG_FOUND)
if(TIFF_FOUND)
  set(poppler_LIBS ${poppler_LIBS} ${TIFF_LIBRARIES})
endif(TIFF_FOUND)
if(HAVE_PTHREAD)
  set(poppler_LIBS ${poppler_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

if(MSVC)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "/Zc:wchar_t- ${CMAKE_CXX_FLAGS}")
add_library(poppler STATIC ${poppler_SRCS})
else(MSVC)
add_library(poppler SHARED ${poppler_SRCS})
endif(MSVC)
set_target_properties(poppler PROPERTIES VERSION 43.0.0 SOVERSION 43)
target_link_libraries(poppler ${poppler_LIBS})
target_link_libraries(poppler LINK_INTERFACE_LIBRARIES "")
install(TARGETS poppler RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

if(ENABLE_XPDF_HEADERS)
  install(FILES
    poppler/Annot.h
    poppler/Array.h
    poppler/BuiltinFont.h
    poppler/BuiltinFontTables.h
    poppler/CachedFile.h
    poppler/Catalog.h
    poppler/CharCodeToUnicode.h
    poppler/CMap.h
    poppler/DateInfo.h
    poppler/Decrypt.h
    poppler/Dict.h
    poppler/Error.h
    poppler/FileSpec.h
    poppler/FontEncodingTables.h
    poppler/FontInfo.h
    poppler/Form.h
    poppler/Function.cc
    poppler/Function.h
    poppler/Gfx.h
    poppler/GfxFont.h
    poppler/GfxState.h
    poppler/GfxState_helpers.h
    poppler/GlobalParams.h
    poppler/Hints.h
    poppler/JArithmeticDecoder.h
    poppler/JBIG2Stream.h
    poppler/Lexer.h
    poppler/Link.h
    poppler/Linearization.h
    poppler/LocalPDFDocBuilder.h
    poppler/Movie.h
    poppler/NameToCharCode.h
    poppler/Object.h
    poppler/OptionalContent.h
    poppler/Outline.h
    poppler/OutputDev.h
    poppler/Page.h
    poppler/PageTransition.h
    poppler/Parser.h
    poppler/PDFDoc.h
    poppler/PDFDocBuilder.h
    poppler/PDFDocEncoding.h
    poppler/PDFDocFactory.h
    poppler/PopplerCache.h
    poppler/ProfileData.h
    poppler/PreScanOutputDev.h
    poppler/PSTokenizer.h
    poppler/Rendition.h
    poppler/Stream-CCITT.h
    poppler/Stream.h
    poppler/StructElement.h
    poppler/StructTreeRoot.h
    poppler/UnicodeMap.h
    poppler/UnicodeMapTables.h
    poppler/UnicodeTypeTable.h
    poppler/UnicodeCClassTables.h
    poppler/UnicodeCompTables.h
    poppler/UnicodeDecompTables.h
    poppler/ViewerPreferences.h
    poppler/XRef.h
    poppler/CharTypes.h
    poppler/CompactFontTables.h
    poppler/ErrorCodes.h
    poppler/NameToUnicodeTable.h
    poppler/PSOutputDev.h
    poppler/TextOutputDev.h
    poppler/SecurityHandler.h
    poppler/StdinCachedFile.h
    poppler/StdinPDFDocBuilder.h
    poppler/UTF.h
    poppler/UTF8.h
    poppler/XpdfPluginAPI.h
    poppler/Sound.h
    ${CMAKE_CURRENT_BINARY_DIR}/poppler/poppler-config.h
    DESTINATION include/poppler)
  install(FILES
    goo/GooHash.h
    goo/GooList.h
    goo/GooTimer.h
    goo/GooMutex.h
    goo/GooString.h
    goo/gtypes.h
    goo/gmem.h
    goo/gfile.h
    goo/FixedPoint.h
    goo/ImgWriter.h
    goo/GooLikely.h
    goo/gstrtod.h
    goo/grandom.h
    DESTINATION include/poppler/goo)
  if(PNG_FOUND)
    install(FILES
      goo/PNGWriter.h
      DESTINATION include/poppler/goo)
  endif(PNG_FOUND)
  if(TIFF_FOUND)
    install(FILES
      goo/TiffWriter.h
      DESTINATION include/poppler/goo)
  endif(TIFF_FOUND)
  install(FILES
    fofi/FoFiBase.h
    fofi/FoFiEncodings.h
    fofi/FoFiTrueType.h
    fofi/FoFiType1.h
    fofi/FoFiType1C.h
    fofi/FoFiIdentifier.h
    DESTINATION include/poppler/fofi)
  if(ENABLE_LIBCURL)
    install(FILES
      poppler/CurlCachedFile.h
      poppler/CurlPDFDocBuilder.h
      DESTINATION include/poppler)
  endif(ENABLE_LIBCURL)
  if(LIBOPENJPEG_FOUND)
    install(FILES
      poppler/JPEG2000Stream.h
      DESTINATION include/poppler)
    install(FILES
      goo/JpegWriter.h
      DESTINATION include/poppler/goo)
  else(LIBOPENJPEG_FOUND)
    install(FILES
      poppler/JPXStream.h
      DESTINATION include/poppler)
  endif(LIBOPENJPEG_FOUND)
  if(ENABLE_SPLASH)
    install(FILES
      poppler/SplashOutputDev.h
      DESTINATION include/poppler)
    install(FILES
      splash/Splash.h
      splash/SplashBitmap.h
      splash/SplashClip.h
      splash/SplashErrorCodes.h
      splash/SplashFTFont.h
      splash/SplashFTFontEngine.h
      splash/SplashFTFontFile.h
      splash/SplashFont.h
      splash/SplashFontEngine.h
      splash/SplashFontFile.h
      splash/SplashFontFileID.h
      splash/SplashGlyphBitmap.h
      splash/SplashMath.h
      splash/SplashPath.h
      splash/SplashPattern.h
      splash/SplashScreen.h
      splash/SplashState.h
      splash/SplashT1Font.h
      splash/SplashT1FontEngine.h
      splash/SplashT1FontFile.h
      splash/SplashTypes.h
      splash/SplashXPath.h
      splash/SplashXPathScanner.h
      DESTINATION include/poppler/splash)
  endif(ENABLE_SPLASH)
endif(ENABLE_XPDF_HEADERS)


if(ENABLE_UTILS)
  add_subdirectory(utils)
endif(ENABLE_UTILS)
if(ENABLE_GLIB)
  add_subdirectory(glib)
endif(ENABLE_GLIB)
add_subdirectory(test)
if(QT4_FOUND)
  add_subdirectory(qt4)
endif(QT4_FOUND)
if(QT5_FOUND)
  add_subdirectory(qt5)
endif(QT5_FOUND)
if(ENABLE_CPP)
  add_subdirectory(cpp)
endif(ENABLE_CPP)

set(PKG_CONFIG_VERSION_0_18 TRUE)
if(PKG_CONFIG_EXECUTABLE)
  exec_program(${PKG_CONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _output_VAR)
  macro_ensure_version("0.18" "${_output_VAR}" PKG_CONFIG_VERSION_0_18)
endif(PKG_CONFIG_EXECUTABLE)
if(PKG_CONFIG_VERSION_0_18)
  set(PC_REQUIRES "")
  set(PC_REQUIRES_PRIVATE "Requires.private: poppler = ${POPPLER_VERSION}")
else(PKG_CONFIG_VERSION_0_18)
  set(PC_REQUIRES "poppler = ${POPPLER_VERSION}")
  set(PC_REQUIRES_PRIVATE "")
endif(PKG_CONFIG_VERSION_0_18)

poppler_create_install_pkgconfig(poppler.pc lib${LIB_SUFFIX}/pkgconfig)
if(ENABLE_SPLASH)
  poppler_create_install_pkgconfig(poppler-splash.pc lib${LIB_SUFFIX}/pkgconfig)
endif(ENABLE_SPLASH)
if(QT4_FOUND)
  poppler_create_install_pkgconfig(poppler-qt4.pc lib${LIB_SUFFIX}/pkgconfig)
endif(QT4_FOUND)
if(QT5_FOUND)
  poppler_create_install_pkgconfig(poppler-qt5.pc lib${LIB_SUFFIX}/pkgconfig)
endif(QT5_FOUND)
if(ENABLE_GLIB)
  poppler_create_install_pkgconfig(poppler-glib.pc lib${LIB_SUFFIX}/pkgconfig)
endif(ENABLE_GLIB)
if(CAIRO_FOUND)
  poppler_create_install_pkgconfig(poppler-cairo.pc lib${LIB_SUFFIX}/pkgconfig)
endif(CAIRO_FOUND)
if(ENABLE_CPP)
  poppler_create_install_pkgconfig(poppler-cpp.pc lib${LIB_SUFFIX}/pkgconfig)
endif(ENABLE_CPP)


message("Building Poppler with support for:")
show_end_message("font configuration" ${font_configuration})
show_end_message_yesno("splash output" ENABLE_SPLASH)
show_end_message_yesno("cairo output" CAIRO_FOUND)
show_end_message_yesno("qt4 wrapper" QT4_FOUND)
show_end_message_yesno("qt5 wrapper" QT5_FOUND)
show_end_message_yesno("glib wrapper" ENABLE_GLIB)
show_end_message_yesno("  introspection" INTROSPECTION_FOUND)
show_end_message_yesno("cpp wrapper" ENABLE_CPP)
show_end_message("use gtk-doc" "not supported with this CMake build system")
show_end_message_yesno("use libjpeg" ENABLE_LIBJPEG)
show_end_message_yesno("use libpng" ENABLE_LIBPNG)
show_end_message_yesno("use libtiff" ENABLE_LIBTIFF)
show_end_message_yesno("use zlib" ENABLE_ZLIB)
show_end_message_yesno("use curl" ENABLE_LIBCURL)
show_end_message_yesno("use libopenjpeg" LIBOPENJPEG_FOUND)
show_end_message_yesno("use cms" USE_CMS)
if(LCMS_FOUND)
  message("      with lcms1")
endif(LCMS_FOUND)
if(LCMS2_FOUND)
  message("      with lcms2")
endif(LCMS2_FOUND)
show_end_message_yesno("command line utils" ENABLE_UTILS)
show_end_message("test data dir" ${TESTDATADIR})

if(USE_FIXEDPOINT AND USE_FLOAT)
  message("Warning: Single precision and fixed point options should not be enabled at the same time")
endif(USE_FIXEDPOINT AND USE_FLOAT)
