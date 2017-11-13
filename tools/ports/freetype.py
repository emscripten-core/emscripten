import os, shutil, logging
from subprocess import Popen

TAG = 'version_1'

def get(ports, settings, shared):
  if settings.USE_FREETYPE == 1:
    ports.fetch_project('freetype', 'https://github.com/emscripten-ports/FreeType/archive/' + TAG + '.zip', 'FreeType-' + TAG)
    def create():     
      ports.clear_project_build('freetype')

      source_path = os.path.join(ports.get_dir(), 'freetype', 'FreeType-' + TAG)
      dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'freetype')
      shared.try_delete(dest_path)
      os.makedirs(dest_path)
      shutil.rmtree(dest_path, ignore_errors=True)
      shutil.copytree(source_path, dest_path)
      open(os.path.join(dest_path, 'include/ftconfig.h'), 'w').write(ftconf_h)

      # build
      srcs = ['src/autofit/autofit.c',
              'src/base/ftadvanc.c',
              'src/base/ftbbox.c',
              'src/base/ftbdf.c',
              'src/base/ftbitmap.c',
              'src/base/ftcalc.c',
              'src/base/ftcid.c',
              'src/base/ftdbgmem.c',
              'src/base/ftdebug.c',
              'src/base/ftfntfmt.c',
              'src/base/ftfstype.c',
              'src/base/ftgasp.c',
              'src/base/ftgloadr.c',
              'src/base/ftglyph.c',
              'src/base/ftgxval.c',
              'src/base/ftinit.c',
              'src/base/ftlcdfil.c',
              'src/base/ftmm.c',
              'src/base/ftobjs.c',
              'src/base/ftotval.c',
              'src/base/ftoutln.c',
              'src/base/ftpatent.c',
              'src/base/ftpfr.c',
              'src/base/ftrfork.c',
              'src/base/ftsnames.c',
              'src/base/ftstream.c',
              'src/base/ftstroke.c',
              'src/base/ftsynth.c',
              'src/base/ftsystem.c',
              'src/base/fttrigon.c',
              'src/base/fttype1.c',
              'src/base/ftutil.c',
              'src/base/ftwinfnt.c',
              'src/bdf/bdf.c',
              'src/bzip2/ftbzip2.c',
              'src/cache/ftcache.c',
              'src/cff/cff.c',
              'src/cid/type1cid.c',
              'src/gzip/ftgzip.c',
              'src/lzw/ftlzw.c',
              'src/pcf/pcf.c',
              'src/pfr/pfr.c',
              'src/psaux/psaux.c',
              'src/pshinter/pshinter.c',
              'src/psnames/psmodule.c',
              'src/raster/raster.c',
              'src/sfnt/sfnt.c',
              'src/smooth/smooth.c',
              'src/truetype/truetype.c',
              'src/type1/type1.c',
              'src/type42/type42.c',
              'src/winfonts/winfnt.c']
      commands = []
      o_s = []
      for src in srcs:
        o = os.path.join(ports.get_build_dir(), 'freetype', src + '.o')
        shared.safe_ensure_dirs(os.path.dirname(o))
        commands.append([shared.PYTHON, shared.EMCC, os.path.join(dest_path, src), '-DFT2_BUILD_LIBRARY', '-O2', '-o', o, '-I' + dest_path + '/include',
                         '-I' + dest_path + '/truetype', '-I' + dest_path + '/sfnt', '-I' + dest_path + '/autofit', '-I' + dest_path + '/smooth', 
                         '-I' + dest_path + '/raster', '-I' + dest_path + '/psaux', '-I' + dest_path + '/psnames', '-I' + dest_path + '/truetype', 
                         '-w',])
        o_s.append(o)

      ports.run_commands(commands)
      final = os.path.join(ports.get_build_dir(), 'freetype', 'libfreetype.a')
      shared.try_delete(final)
      Popen([shared.LLVM_AR, 'rc', final] + o_s).communicate()
      assert os.path.exists(final)
      return final
    return [shared.Cache.get('freetype', create, what='port')]
  else:
    return []

def process_args(ports, args, settings, shared):
  if settings.USE_FREETYPE == 1:
    get(ports, settings, shared)
    args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'freetype/include')]
  return args

def show():
  return 'freetype (USE_FREETYPE=1; freetype license)'



ftconf_h = r'''/***************************************************************************/
/*                                                                         */
/*  ftconfig.in                                                            */
/*                                                                         */
/*    UNIX-specific configuration file (specification only).               */
/*                                                                         */
/*  Copyright 1996-2015 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This header file contains a number of macro definitions that are used */
  /* by the rest of the engine.  Most of the macros here are automatically */
  /* determined at compile time, and you should not need to change it to   */
  /* port FreeType, except to compile the library with a non-ANSI          */
  /* compiler.                                                             */
  /*                                                                       */
  /* Note however that if some specific modifications are needed, we       */
  /* advise you to place a modified copy in your build directory.          */
  /*                                                                       */
  /* The build directory is usually `builds/<system>', and contains        */
  /* system-specific files that are always included first when building    */
  /* the library.                                                          */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTCONFIG_H__
#define __FTCONFIG_H__

#include <ft2build.h>
#include <ftoption.h>
#include <ftstdlib.h>


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /*               PLATFORM-SPECIFIC CONFIGURATION MACROS                  */
  /*                                                                       */
  /* These macros can be toggled to suit a specific system.  The current   */
  /* ones are defaults used to compile FreeType in an ANSI C environment   */
  /* (16bit compilers are also supported).  Copy this file to your own     */
  /* `builds/<system>' directory, and edit it to port the engine.          */
  /*                                                                       */
  /*************************************************************************/


#undef HAVE_UNISTD_H
#undef HAVE_FCNTL_H
#undef HAVE_STDINT_H


  /* There are systems (like the Texas Instruments 'C54x) where a `char' */
  /* has 16 bits.  ANSI C says that sizeof(char) is always 1.  Since an  */
  /* `int' has 16 bits also for this system, sizeof(int) gives 1 which   */
  /* is probably unexpected.                                             */
  /*                                                                     */
  /* `CHAR_BIT' (defined in limits.h) gives the number of bits in a      */
  /* `char' type.                                                        */

#ifndef FT_CHAR_BIT
#define FT_CHAR_BIT  CHAR_BIT
#endif


#undef FT_USE_AUTOCONF_SIZEOF_TYPES
#ifdef FT_USE_AUTOCONF_SIZEOF_TYPES

#undef SIZEOF_INT
#undef SIZEOF_LONG
#define FT_SIZEOF_INT  SIZEOF_INT
#define FT_SIZEOF_LONG SIZEOF_LONG

#else /* !FT_USE_AUTOCONF_SIZEOF_TYPES */

  /* Following cpp computation of the bit length of int and long */
  /* is copied from default include/config/ftconfig.h.           */
  /* If any improvement is required for this file, it should be  */
  /* applied to the original header file for the builders that   */
  /* do not use configure script.                                */

  /* The size of an `int' type.  */
#if                                 FT_UINT_MAX == 0xFFFFUL
#define FT_SIZEOF_INT  (16 / FT_CHAR_BIT)
#elif                               FT_UINT_MAX == 0xFFFFFFFFUL
#define FT_SIZEOF_INT  (32 / FT_CHAR_BIT)
#elif FT_UINT_MAX > 0xFFFFFFFFUL && FT_UINT_MAX == 0xFFFFFFFFFFFFFFFFUL
#define FT_SIZEOF_INT  (64 / FT_CHAR_BIT)
#else
#error "Unsupported size of `int' type!"
#endif

  /* The size of a `long' type.  A five-byte `long' (as used e.g. on the */
  /* DM642) is recognized but avoided.                                   */
#if                                  FT_ULONG_MAX == 0xFFFFFFFFUL
#define FT_SIZEOF_LONG  (32 / FT_CHAR_BIT)
#elif FT_ULONG_MAX > 0xFFFFFFFFUL && FT_ULONG_MAX == 0xFFFFFFFFFFUL
#define FT_SIZEOF_LONG  (32 / FT_CHAR_BIT)
#elif FT_ULONG_MAX > 0xFFFFFFFFUL && FT_ULONG_MAX == 0xFFFFFFFFFFFFFFFFUL
#define FT_SIZEOF_LONG  (64 / FT_CHAR_BIT)
#else
#error "Unsupported size of `long' type!"
#endif

#endif /* !FT_USE_AUTOCONF_SIZEOF_TYPES */


  /* FT_UNUSED is a macro used to indicate that a given parameter is not  */
  /* used -- this is only used to get rid of unpleasant compiler warnings */
#ifndef FT_UNUSED
#define FT_UNUSED( arg )  ( (arg) = (arg) )
#endif


  /*************************************************************************/
  /*                                                                       */
  /*                     AUTOMATIC CONFIGURATION MACROS                    */
  /*                                                                       */
  /* These macros are computed from the ones defined above.  Don't touch   */
  /* their definition, unless you know precisely what you are doing.  No   */
  /* porter should need to mess with them.                                 */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Mac support                                                           */
  /*                                                                       */
  /*   This is the only necessary change, so it is defined here instead    */
  /*   providing a new configuration file.                                 */
  /*                                                                       */
#if defined( __APPLE__ ) || ( defined( __MWERKS__ ) && defined( macintosh ) )
  /* no Carbon frameworks for 64bit 10.4.x */
  /* AvailabilityMacros.h is available since Mac OS X 10.2,        */
  /* so guess the system version by maximum errno before inclusion */
#include <errno.h>
#ifdef ECANCELED /* defined since 10.2 */
#include "AvailabilityMacros.h"
#endif
#if defined( __LP64__ ) && \
    ( MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4 )
#undef FT_MACINTOSH
#endif

#elif defined( __SC__ ) || defined( __MRC__ )
  /* Classic MacOS compilers */
#include "ConditionalMacros.h"
#if TARGET_OS_MAC
#define FT_MACINTOSH 1
#endif

#endif


  /* Fix compiler warning with sgi compiler */
#if defined( __sgi ) && !defined( __GNUC__ )
#if defined( _COMPILER_VERSION ) && ( _COMPILER_VERSION >= 730 )
#pragma set woff 3505
#endif
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int16                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 16bit signed integer type.                         */
  /*                                                                       */
  typedef signed short  FT_Int16;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt16                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 16bit unsigned integer type.                       */
  /*                                                                       */
  typedef unsigned short  FT_UInt16;

  /* */


  /* this #if 0 ... #endif clause is for documentation purposes */
#if 0

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int32                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 32bit signed integer type.  The size depends on    */
  /*    the configuration.                                                 */
  /*                                                                       */
  typedef signed XXX  FT_Int32;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt32                                                          */
  /*                                                                       */
  /*    A typedef for a 32bit unsigned integer type.  The size depends on  */
  /*    the configuration.                                                 */
  /*                                                                       */
  typedef unsigned XXX  FT_UInt32;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int64                                                           */
  /*                                                                       */
  /*    A typedef for a 64bit signed integer type.  The size depends on    */
  /*    the configuration.  Only defined if there is real 64bit support;   */
  /*    otherwise, it gets emulated with a structure (if necessary).       */
  /*                                                                       */
  typedef signed XXX  FT_Int64;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt64                                                          */
  /*                                                                       */
  /*    A typedef for a 64bit unsigned integer type.  The size depends on  */
  /*    the configuration.  Only defined if there is real 64bit support;   */
  /*    otherwise, it gets emulated with a structure (if necessary).       */
  /*                                                                       */
  typedef unsigned XXX  FT_UInt64;

  /* */

#endif

#if FT_SIZEOF_INT == 4

  typedef signed int      FT_Int32;
  typedef unsigned int    FT_UInt32;

#elif FT_SIZEOF_LONG == 4

  typedef signed long     FT_Int32;
  typedef unsigned long   FT_UInt32;

#else
#error "no 32bit type found -- please check your configuration files"
#endif


  /* look up an integer type that is at least 32 bits */
#if FT_SIZEOF_INT >= 4

  typedef int            FT_Fast;
  typedef unsigned int   FT_UFast;

#elif FT_SIZEOF_LONG >= 4

  typedef long           FT_Fast;
  typedef unsigned long  FT_UFast;

#endif


  /* determine whether we have a 64-bit int type for platforms without */
  /* Autoconf                                                          */
#if FT_SIZEOF_LONG == 8

  /* FT_LONG64 must be defined if a 64-bit type is available */
#define FT_LONG64
#define FT_INT64   long
#define FT_UINT64  unsigned long

  /*************************************************************************/
  /*                                                                       */
  /* A 64-bit data type may create compilation problems if you compile     */
  /* in strict ANSI mode.  To avoid them, we disable other 64-bit data     */
  /* types if __STDC__ is defined.  You can however ignore this rule       */
  /* by defining the FT_CONFIG_OPTION_FORCE_INT64 configuration macro.     */
  /*                                                                       */
#elif !defined( __STDC__ ) || defined( FT_CONFIG_OPTION_FORCE_INT64 )

#if defined( _MSC_VER ) && _MSC_VER >= 900  /* Visual C++ (and Intel C++) */

  /* this compiler provides the __int64 type */
#define FT_LONG64
#define FT_INT64   __int64
#define FT_UINT64  unsigned __int64

#elif defined( __BORLANDC__ )  /* Borland C++ */

  /* XXXX: We should probably check the value of __BORLANDC__ in order */
  /*       to test the compiler version.                               */

  /* this compiler provides the __int64 type */
#define FT_LONG64
#define FT_INT64   __int64
#define FT_UINT64  unsigned __int64

#elif defined( __WATCOMC__ )   /* Watcom C++ */

  /* Watcom doesn't provide 64-bit data types */

#elif defined( __MWERKS__ )    /* Metrowerks CodeWarrior */

#define FT_LONG64
#define FT_INT64   long long int
#define FT_UINT64  unsigned long long int

#elif defined( __GNUC__ )

  /* GCC provides the `long long' type */
#define FT_LONG64
#define FT_INT64   long long int
#define FT_UINT64  unsigned long long int

#endif /* _MSC_VER */

#endif /* FT_SIZEOF_LONG == 8 */

#ifdef FT_LONG64
  typedef FT_INT64   FT_Int64;
  typedef FT_UINT64  FT_UInt64;
#endif


  /*************************************************************************/
  /*                                                                       */
  /* miscellaneous                                                         */
  /*                                                                       */
  /*************************************************************************/


#define FT_BEGIN_STMNT  do {
#define FT_END_STMNT    } while ( 0 )
#define FT_DUMMY_STMNT  FT_BEGIN_STMNT FT_END_STMNT


  /* typeof condition taken from gnulib's `intprops.h' header file */
#if ( __GNUC__ >= 2                         || \
      defined( __IBM__TYPEOF__ )            || \
      ( __SUNPRO_C >= 0x5110 && !__STDC__ ) )
#define TYPEOF( type )  (__typeof__ (type))
#else
#define TYPEOF( type )  /* empty */
#endif


#ifdef FT_MAKE_OPTION_SINGLE_OBJECT

#define FT_LOCAL( x )      static  x
#define FT_LOCAL_DEF( x )  static  x

#else

#ifdef __cplusplus
#define FT_LOCAL( x )      extern "C"  x
#define FT_LOCAL_DEF( x )  extern "C"  x
#else
#define FT_LOCAL( x )      extern  x
#define FT_LOCAL_DEF( x )  x
#endif

#endif /* FT_MAKE_OPTION_SINGLE_OBJECT */

#define FT_LOCAL_ARRAY( x )      extern const  x
#define FT_LOCAL_ARRAY_DEF( x )  const  x


#ifndef FT_BASE

#ifdef __cplusplus
#define FT_BASE( x )  extern "C"  x
#else
#define FT_BASE( x )  extern  x
#endif

#endif /* !FT_BASE */


#ifndef FT_BASE_DEF

#ifdef __cplusplus
#define FT_BASE_DEF( x )  x
#else
#define FT_BASE_DEF( x )  x
#endif

#endif /* !FT_BASE_DEF */


#ifndef FT_EXPORT

#ifdef __cplusplus
#define FT_EXPORT( x )  extern "C"  x
#else
#define FT_EXPORT( x )  extern  x
#endif

#endif /* !FT_EXPORT */


#ifndef FT_EXPORT_DEF

#ifdef __cplusplus
#define FT_EXPORT_DEF( x )  extern "C"  x
#else
#define FT_EXPORT_DEF( x )  extern  x
#endif

#endif /* !FT_EXPORT_DEF */


#ifndef FT_EXPORT_VAR

#ifdef __cplusplus
#define FT_EXPORT_VAR( x )  extern "C"  x
#else
#define FT_EXPORT_VAR( x )  extern  x
#endif

#endif /* !FT_EXPORT_VAR */

  /* The following macros are needed to compile the library with a   */
  /* C++ compiler and with 16bit compilers.                          */
  /*                                                                 */

  /* This is special.  Within C++, you must specify `extern "C"' for */
  /* functions which are used via function pointers, and you also    */
  /* must do that for structures which contain function pointers to  */
  /* assure C linkage -- it's not possible to have (local) anonymous */
  /* functions which are accessed by (global) function pointers.     */
  /*                                                                 */
  /*                                                                 */
  /* FT_CALLBACK_DEF is used to _define_ a callback function.        */
  /*                                                                 */
  /* FT_CALLBACK_TABLE is used to _declare_ a constant variable that */
  /* contains pointers to callback functions.                        */
  /*                                                                 */
  /* FT_CALLBACK_TABLE_DEF is used to _define_ a constant variable   */
  /* that contains pointers to callback functions.                   */
  /*                                                                 */
  /*                                                                 */
  /* Some 16bit compilers have to redefine these macros to insert    */
  /* the infamous `_cdecl' or `__fastcall' declarations.             */
  /*                                                                 */
#ifndef FT_CALLBACK_DEF
#ifdef __cplusplus
#define FT_CALLBACK_DEF( x )  extern "C"  x
#else
#define FT_CALLBACK_DEF( x )  static  x
#endif
#endif /* FT_CALLBACK_DEF */

#ifndef FT_CALLBACK_TABLE
#ifdef __cplusplus
#define FT_CALLBACK_TABLE      extern "C"
#define FT_CALLBACK_TABLE_DEF  extern "C"
#else
#define FT_CALLBACK_TABLE      extern
#define FT_CALLBACK_TABLE_DEF  /* nothing */
#endif
#endif /* FT_CALLBACK_TABLE */


FT_END_HEADER


#endif /* __FTCONFIG_H__ */


/* END */
'''
