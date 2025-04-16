/* config.h.  Generated from config.h.cmake by cmake.  */

/* Build against libcurl. */
#cmakedefine ENABLE_LIBCURL 1

/* Use libjpeg instead of builtin jpeg decoder. */
#cmakedefine ENABLE_LIBJPEG 1

/* Use libopenjpeg instead of builtin jpeg2000 decoder. */
#cmakedefine ENABLE_LIBOPENJPEG 1

/* Build against libpng. */
#cmakedefine ENABLE_LIBPNG 1

/* Do not hardcode the library location */
#cmakedefine ENABLE_RELOCATABLE 1

/* Use zlib instead of builtin zlib decoder. */
#cmakedefine ENABLE_ZLIB 1

/* Use cairo for rendering. */
#cmakedefine HAVE_CAIRO 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Have FreeType2 include files */
#cmakedefine HAVE_FREETYPE_H 1

/* Define to 1 if you have the `fseek64' function. */
#cmakedefine HAVE_FSEEK64 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#cmakedefine HAVE_FSEEKO 1

/* Define to 1 if you have the `ftell64' function. */
#cmakedefine HAVE_FTELL64 1

/* Defines if gettimeofday is available on your system */
#cmakedefine HAVE_GETTIMEOFDAY 1

/* Defines if gmtime_r is available on your system */
#cmakedefine HAVE_GMTIME_R 1

/* Define if you have the iconv() function and it works. */
#cmakedefine HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the `openjpeg' library (-lopenjpeg). */
#cmakedefine HAVE_LIBOPENJPEG 1

/* Define to 1 if you have the `z' library (-lz). */
#cmakedefine HAVE_LIBZ 1

/* Defines if localtime_r is available on your system */
#cmakedefine HAVE_LOCALTIME_R 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP 1

/* Define to 1 if you have the `mkstemps' function. */
#cmakedefine HAVE_MKSTEMPS 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H 1

/* Define to 1 if you have the <openjpeg.h> header file. */
#cmakedefine HAVE_OPENJPEG_H 1

/* Define to 1 if you have the `popen' function. */
#cmakedefine HAVE_POPEN 1

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine HAVE_PTHREAD 1

/* Use splash for rendering. */
#cmakedefine HAVE_SPLASH 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#cmakedefine HAVE_SYS_MMAN_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the <zlib.h> header file. */
#cmakedefine HAVE_ZLIB_H 1

/* Define to 1 if you have a big endian machine */
#cmakedefine WORDS_BIGENDIAN 1

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST ${ICONV_CONST}

/* Enable multithreading support. */
#cmakedefine MULTITHREADED 1

/* Generate OPI comments in PS output. */
#cmakedefine OPI_SUPPORT 1

/* Name of package */
#define PACKAGE "poppler"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=poppler"

/* Define to the full name of this package. */
#define PACKAGE_NAME "poppler"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "poppler ${POPPLER_VERSION}"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "poppler"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "${POPPLER_VERSION}"

/* Poppler data dir */
#define POPPLER_DATADIR "${CMAKE_INSTALL_PREFIX}/share/poppler"

/* Support for curl based doc builder is compiled in. */
#cmakedefine POPPLER_HAS_CURL_SUPPORT 1

/* Have GDK */
#cmakedefine POPPLER_WITH_GDK 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable word list support. */
#cmakedefine TEXTOUT_WORD_LIST 1

/* Defines if use cms */
#cmakedefine USE_CMS 1

/* Use fixed point arithmetic in the Splash backend */
#cmakedefine USE_FIXEDPOINT 1

/* Use single precision arithmetic in the Splash backend */
#cmakedefine USE_FLOAT 1

/* Version number of package */
#define VERSION "${POPPLER_VERSION}"

/* Use fontconfig font configuration backend */
#cmakedefine WITH_FONTCONFIGURATION_FONTCONFIG 1

/* Use win32 font configuration backend */
#cmakedefine WITH_FONTCONFIGURATION_WIN32 1

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif

/* MS has defined snprintf as deprecated */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
