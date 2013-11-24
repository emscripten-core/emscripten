//================================================= -*- mode: c++ -*- ====
//
// poppler-config.h
//
// Copyright 1996-2011 Glyph & Cog, LLC
//
//========================================================================

#ifndef POPPLER_CONFIG_H
#define POPPLER_CONFIG_H

// We duplicate some of the config.h #define's here since they are
// used in some of the header files we install.  The #ifndef/#endif
// around #undef look odd, but it's to silence warnings about
// redefining those symbols.

/* Defines the poppler version */
#ifndef POPPLER_VERSION
#define POPPLER_VERSION "${POPPLER_VERSION}"
#endif

/* Enable multithreading support. */
#ifndef MULTITHREADED
#cmakedefine MULTITHREADED 1
#endif

/* Use fixedpoint. */
#ifndef USE_FIXEDPOINT
#cmakedefine USE_FIXEDPOINT 1
#endif

/* Use single precision arithmetic in the Splash backend */
#ifndef USE_FLOAT
#cmakedefine USE_FLOAT 1
#endif

/* Include support for OPI comments. */
#ifndef OPI_SUPPORT
#cmakedefine OPI_SUPPORT 1
#endif

/* Enable word list support. */
#ifndef TEXTOUT_WORD_LIST
#cmakedefine TEXTOUT_WORD_LIST 1
#endif

/* Support for curl is compiled in. */
#ifndef POPPLER_HAS_CURL_SUPPORT
#cmakedefine POPPLER_HAS_CURL_SUPPORT 1
#endif

/* Use libjpeg instead of builtin jpeg decoder. */
#ifndef ENABLE_LIBJPEG
#cmakedefine ENABLE_LIBJPEG 1
#endif

/* Build against libtiff. */
#ifndef ENABLE_LIBTIFF
#cmakedefine ENABLE_LIBTIFF 1
#endif

/* Build against libpng. */
#ifndef ENABLE_LIBPNG
#cmakedefine ENABLE_LIBPNG 1
#endif

/* Use zlib instead of builtin zlib decoder. */
#ifndef ENABLE_ZLIB
#cmakedefine ENABLE_ZLIB 1
#endif

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_DIRENT_H
#cmakedefine HAVE_DIRENT_H 1
#endif

/* Defines if gettimeofday is available on your system */
#ifndef HAVE_GETTIMEOFDAY
#cmakedefine HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#ifndef HAVE_NDIR_H
#cmakedefine HAVE_NDIR_H 1
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_SYS_DIR_H
#cmakedefine HAVE_SYS_DIR_H 1
#endif

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_SYS_NDIR_H
#cmakedefine HAVE_SYS_NDIR_H 1
#endif

/* Have FreeType2 include files */
#ifndef HAVE_FREETYPE_H
#cmakedefine HAVE_FREETYPE_H 1
#endif

/* Defines if use cms */
#ifndef USE_CMS
#cmakedefine USE_CMS 1
#endif

// Also, there are preprocessor symbols in the header files
// that are used but never defined when building poppler using configure
// or cmake: DISABLE_OUTLINE, DEBUG_MEM, SPLASH_CMYK, HAVE_T1LIB_H,
// ENABLE_PLUGINS, DEBUG_FORMS, HAVE_FREETYPE_FREETYPE_H

//------------------------------------------------------------------------
// version
//------------------------------------------------------------------------

// copyright notice
#define popplerCopyright "Copyright 2005-2013 The Poppler Developers - http://poppler.freedesktop.org"
#define xpdfCopyright "Copyright 1996-2011 Glyph & Cog, LLC"

//------------------------------------------------------------------------
// popen
//------------------------------------------------------------------------

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define popen _popen
#define pclose _pclose
#endif

#if defined(VMS) || defined(VMCMS) || defined(DOS) || defined(OS2) || defined(__EMX__) || defined(_WIN32) || defined(__DJGPP__) || defined(MACOS)
#define POPEN_READ_MODE "rb"
#else
#define POPEN_READ_MODE "r"
#endif

//------------------------------------------------------------------------
// Win32 stuff
//------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_MSC_VER)
#include <windef.h>
#else
#define CDECL
#endif

#if defined(_WIN32)
#ifdef _MSC_VER
#define strtok_r strtok_s
#elif __MINGW32__ && !defined(__WINPTHREADS_VERSION)
char * strtok_r (char *s, const char *delim, char **save_ptr);
#endif
#endif

//------------------------------------------------------------------------
// Compiler
//------------------------------------------------------------------------

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#ifdef __MINGW_PRINTF_FORMAT
#define GCC_PRINTF_FORMAT(fmt_index, va_index) \
       __attribute__((__format__(__MINGW_PRINTF_FORMAT, fmt_index, va_index)))
#else
#define GCC_PRINTF_FORMAT(fmt_index, va_index) \
	__attribute__((__format__(__printf__, fmt_index, va_index)))
#endif
#else
#define GCC_PRINTF_FORMAT(fmt_index, va_index)
#endif

#if defined(_MSC_VER)
#define fmax(a, b) max(a, b)
#define fmin(a, b) min(a, b)
#endif


#endif /* POPPLER_CONFIG_H */
