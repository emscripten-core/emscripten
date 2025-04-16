/***************************************************************************/
/*                                                                         */
/*  ftoption.h                                                             */
/*                                                                         */
/*    User-selectable configuration macros (specification only).           */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,   */
/*            2010 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTOPTION_H__
#define __FTOPTION_H__


#include <ft2build.h>


FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /*                 USER-SELECTABLE CONFIGURATION MACROS                  */
  /*                                                                       */
  /* This file contains the default configuration macro definitions for    */
  /* a standard build of the FreeType library.  There are three ways to    */
  /* use this file to build project-specific versions of the library:      */
  /*                                                                       */
  /*  - You can modify this file by hand, but this is not recommended in   */
  /*    cases where you would like to build several versions of the        */
  /*    library from a single source directory.                            */
  /*                                                                       */
  /*  - You can put a copy of this file in your build directory, more      */
  /*    precisely in `$BUILD/freetype/config/ftoption.h', where `$BUILD'   */
  /*    is the name of a directory that is included _before_ the FreeType  */
  /*    include path during compilation.                                   */
  /*                                                                       */
  /*    The default FreeType Makefiles and Jamfiles use the build          */
  /*    directory `builds/<system>' by default, but you can easily change  */
  /*    that for your own projects.                                        */
  /*                                                                       */
  /*  - Copy the file <ft2build.h> to `$BUILD/ft2build.h' and modify it    */
  /*    slightly to pre-define the macro FT_CONFIG_OPTIONS_H used to       */
  /*    locate this file during the build.  For example,                   */
  /*                                                                       */
  /*      #define FT_CONFIG_OPTIONS_H  <myftoptions.h>                     */
  /*      #include <freetype/config/ftheader.h>                            */
  /*                                                                       */
  /*    will use `$BUILD/myftoptions.h' instead of this file for macro     */
  /*    definitions.                                                       */
  /*                                                                       */
  /*    Note also that you can similarly pre-define the macro              */
  /*    FT_CONFIG_MODULES_H used to locate the file listing of the modules */
  /*    that are statically linked to the library at compile time.  By     */
  /*    default, this file is <freetype/config/ftmodule.h>.                */
  /*                                                                       */
  /*  We highly recommend using the third method whenever possible.        */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /**** G E N E R A L   F R E E T Y P E   2   C O N F I G U R A T I O N ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Uncomment the line below if you want to activate sub-pixel rendering  */
  /* (a.k.a. LCD rendering, or ClearType) in this build of the library.    */
  /*                                                                       */
  /* Note that this feature is covered by several Microsoft patents        */
  /* and should not be activated in any default build of the library.      */
  /*                                                                       */
  /* This macro has no impact on the FreeType API, only on its             */
  /* _implementation_.  For example, using FT_RENDER_MODE_LCD when calling */
  /* FT_Render_Glyph still generates a bitmap that is 3 times wider than   */
  /* the original size in case this macro isn't defined; however, each     */
  /* triplet of subpixels has R=G=B.                                       */
  /*                                                                       */
  /* This is done to allow FreeType clients to run unmodified, forcing     */
  /* them to display normal gray-level anti-aliased glyphs.                */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING */


  /*************************************************************************/
  /*                                                                       */
  /* Many compilers provide a non-ANSI 64-bit data type that can be used   */
  /* by FreeType to speed up some computations.  However, this will create */
  /* some problems when compiling the library in strict ANSI mode.         */
  /*                                                                       */
  /* For this reason, the use of 64-bit integers is normally disabled when */
  /* the __STDC__ macro is defined.  You can however disable this by       */
  /* defining the macro FT_CONFIG_OPTION_FORCE_INT64 here.                 */
  /*                                                                       */
  /* For most compilers, this will only create compilation warnings when   */
  /* building the library.                                                 */
  /*                                                                       */
  /* ObNote: The compiler-specific 64-bit integers are detected in the     */
  /*         file `ftconfig.h' either statically or through the            */
  /*         `configure' script on supported platforms.                    */
  /*                                                                       */
#undef FT_CONFIG_OPTION_FORCE_INT64


  /*************************************************************************/
  /*                                                                       */
  /* If this macro is defined, do not try to use an assembler version of   */
  /* performance-critical functions (e.g. FT_MulFix).  You should only do  */
  /* that to verify that the assembler function works properly, or to      */
  /* execute benchmark tests of the various implementations.               */
/* #define FT_CONFIG_OPTION_NO_ASSEMBLER */


  /*************************************************************************/
  /*                                                                       */
  /* If this macro is defined, try to use an inlined assembler version of  */
  /* the `FT_MulFix' function, which is a `hotspot' when loading and       */
  /* hinting glyphs, and which should be executed as fast as possible.     */
  /*                                                                       */
  /* Note that if your compiler or CPU is not supported, this will default */
  /* to the standard and portable implementation found in `ftcalc.c'.      */
  /*                                                                       */
#define FT_CONFIG_OPTION_INLINE_MULFIX


  /*************************************************************************/
  /*                                                                       */
  /* LZW-compressed file support.                                          */
  /*                                                                       */
  /*   FreeType now handles font files that have been compressed with the  */
  /*   `compress' program.  This is mostly used to parse many of the PCF   */
  /*   files that come with various X11 distributions.  The implementation */
  /*   uses NetBSD's `zopen' to partially uncompress the file on the fly   */
  /*   (see src/lzw/ftgzip.c).                                             */
  /*                                                                       */
  /*   Define this macro if you want to enable this `feature'.             */
  /*                                                                       */
#define FT_CONFIG_OPTION_USE_LZW


  /*************************************************************************/
  /*                                                                       */
  /* Gzip-compressed file support.                                         */
  /*                                                                       */
  /*   FreeType now handles font files that have been compressed with the  */
  /*   `gzip' program.  This is mostly used to parse many of the PCF files */
  /*   that come with XFree86.  The implementation uses `zlib' to          */
  /*   partially uncompress the file on the fly (see src/gzip/ftgzip.c).   */
  /*                                                                       */
  /*   Define this macro if you want to enable this `feature'.  See also   */
  /*   the macro FT_CONFIG_OPTION_SYSTEM_ZLIB below.                       */
  /*                                                                       */
#define FT_CONFIG_OPTION_USE_ZLIB


  /*************************************************************************/
  /*                                                                       */
  /* ZLib library selection                                                */
  /*                                                                       */
  /*   This macro is only used when FT_CONFIG_OPTION_USE_ZLIB is defined.  */
  /*   It allows FreeType's `ftgzip' component to link to the system's     */
  /*   installation of the ZLib library.  This is useful on systems like   */
  /*   Unix or VMS where it generally is already available.                */
  /*                                                                       */
  /*   If you let it undefined, the component will use its own copy        */
  /*   of the zlib sources instead.  These have been modified to be        */
  /*   included directly within the component and *not* export external    */
  /*   function names.  This allows you to link any program with FreeType  */
  /*   _and_ ZLib without linking conflicts.                               */
  /*                                                                       */
  /*   Do not #undef this macro here since the build system might define   */
  /*   it for certain configurations only.                                 */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_SYSTEM_ZLIB */


  /*************************************************************************/
  /*                                                                       */
  /* DLL export compilation                                                */
  /*                                                                       */
  /*   When compiling FreeType as a DLL, some systems/compilers need a     */
  /*   special keyword in front OR after the return type of function       */
  /*   declarations.                                                       */
  /*                                                                       */
  /*   Two macros are used within the FreeType source code to define       */
  /*   exported library functions: FT_EXPORT and FT_EXPORT_DEF.            */
  /*                                                                       */
  /*     FT_EXPORT( return_type )                                          */
  /*                                                                       */
  /*       is used in a function declaration, as in                        */
  /*                                                                       */
  /*         FT_EXPORT( FT_Error )                                         */
  /*         FT_Init_FreeType( FT_Library*  alibrary );                    */
  /*                                                                       */
  /*                                                                       */
  /*     FT_EXPORT_DEF( return_type )                                      */
  /*                                                                       */
  /*       is used in a function definition, as in                         */
  /*                                                                       */
  /*         FT_EXPORT_DEF( FT_Error )                                     */
  /*         FT_Init_FreeType( FT_Library*  alibrary )                     */
  /*         {                                                             */
  /*           ... some code ...                                           */
  /*           return FT_Err_Ok;                                           */
  /*         }                                                             */
  /*                                                                       */
  /*   You can provide your own implementation of FT_EXPORT and            */
  /*   FT_EXPORT_DEF here if you want.  If you leave them undefined, they  */
  /*   will be later automatically defined as `extern return_type' to      */
  /*   allow normal compilation.                                           */
  /*                                                                       */
  /*   Do not #undef these macros here since the build system might define */
  /*   them for certain configurations only.                               */
  /*                                                                       */
/* #define FT_EXPORT(x)      extern x */
/* #define FT_EXPORT_DEF(x)  x */


  /*************************************************************************/
  /*                                                                       */
  /* Glyph Postscript Names handling                                       */
  /*                                                                       */
  /*   By default, FreeType 2 is compiled with the `psnames' module.  This */
  /*   module is in charge of converting a glyph name string into a        */
  /*   Unicode value, or return a Macintosh standard glyph name for the    */
  /*   use with the TrueType `post' table.                                 */
  /*                                                                       */
  /*   Undefine this macro if you do not want `psnames' compiled in your   */
  /*   build of FreeType.  This has the following effects:                 */
  /*                                                                       */
  /*   - The TrueType driver will provide its own set of glyph names,      */
  /*     if you build it to support postscript names in the TrueType       */
  /*     `post' table.                                                     */
  /*                                                                       */
  /*   - The Type 1 driver will not be able to synthesize a Unicode        */
  /*     charmap out of the glyphs found in the fonts.                     */
  /*                                                                       */
  /*   You would normally undefine this configuration macro when building  */
  /*   a version of FreeType that doesn't contain a Type 1 or CFF driver.  */
  /*                                                                       */
#define FT_CONFIG_OPTION_POSTSCRIPT_NAMES


  /*************************************************************************/
  /*                                                                       */
  /* Postscript Names to Unicode Values support                            */
  /*                                                                       */
  /*   By default, FreeType 2 is built with the `PSNames' module compiled  */
  /*   in.  Among other things, the module is used to convert a glyph name */
  /*   into a Unicode value.  This is especially useful in order to        */
  /*   synthesize on the fly a Unicode charmap from the CFF/Type 1 driver  */
  /*   through a big table named the `Adobe Glyph List' (AGL).             */
  /*                                                                       */
  /*   Undefine this macro if you do not want the Adobe Glyph List         */
  /*   compiled in your `PSNames' module.  The Type 1 driver will not be   */
  /*   able to synthesize a Unicode charmap out of the glyphs found in the */
  /*   fonts.                                                              */
  /*                                                                       */
#define FT_CONFIG_OPTION_ADOBE_GLYPH_LIST


  /*************************************************************************/
  /*                                                                       */
  /* Support for Mac fonts                                                 */
  /*                                                                       */
  /*   Define this macro if you want support for outline fonts in Mac      */
  /*   format (mac dfont, mac resource, macbinary containing a mac         */
  /*   resource) on non-Mac platforms.                                     */
  /*                                                                       */
  /*   Note that the `FOND' resource isn't checked.                        */
  /*                                                                       */
#define FT_CONFIG_OPTION_MAC_FONTS


  /*************************************************************************/
  /*                                                                       */
  /* Guessing methods to access embedded resource forks                    */
  /*                                                                       */
  /*   Enable extra Mac fonts support on non-Mac platforms (e.g.           */
  /*   GNU/Linux).                                                         */
  /*                                                                       */
  /*   Resource forks which include fonts data are stored sometimes in     */
  /*   locations which users or developers don't expected.  In some cases, */
  /*   resource forks start with some offset from the head of a file.  In  */
  /*   other cases, the actual resource fork is stored in file different   */
  /*   from what the user specifies.  If this option is activated,         */
  /*   FreeType tries to guess whether such offsets or different file      */
  /*   names must be used.                                                 */
  /*                                                                       */
  /*   Note that normal, direct access of resource forks is controlled via */
  /*   the FT_CONFIG_OPTION_MAC_FONTS option.                              */
  /*                                                                       */
#ifdef FT_CONFIG_OPTION_MAC_FONTS
#define FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK
#endif


  /*************************************************************************/
  /*                                                                       */
  /* Allow the use of FT_Incremental_Interface to load typefaces that      */
  /* contain no glyph data, but supply it via a callback function.         */
  /* This is required by clients supporting document formats which         */
  /* supply font data incrementally as the document is parsed, such        */
  /* as the Ghostscript interpreter for the PostScript language.           */
  /*                                                                       */
#define FT_CONFIG_OPTION_INCREMENTAL


  /*************************************************************************/
  /*                                                                       */
  /* The size in bytes of the render pool used by the scan-line converter  */
  /* to do all of its work.                                                */
  /*                                                                       */
  /* This must be greater than 4KByte if you use FreeType to rasterize     */
  /* glyphs; otherwise, you may set it to zero to avoid unnecessary        */
  /* allocation of the render pool.                                        */
  /*                                                                       */
#define FT_RENDER_POOL_SIZE  16384L


  /*************************************************************************/
  /*                                                                       */
  /* FT_MAX_MODULES                                                        */
  /*                                                                       */
  /*   The maximum number of modules that can be registered in a single    */
  /*   FreeType library object.  32 is the default.                        */
  /*                                                                       */
#define FT_MAX_MODULES  32


  /*************************************************************************/
  /*                                                                       */
  /* Debug level                                                           */
  /*                                                                       */
  /*   FreeType can be compiled in debug or trace mode.  In debug mode,    */
  /*   errors are reported through the `ftdebug' component.  In trace      */
  /*   mode, additional messages are sent to the standard output during    */
  /*   execution.                                                          */
  /*                                                                       */
  /*   Define FT_DEBUG_LEVEL_ERROR to build the library in debug mode.     */
  /*   Define FT_DEBUG_LEVEL_TRACE to build it in trace mode.              */
  /*                                                                       */
  /*   Don't define any of these macros to compile in `release' mode!      */
  /*                                                                       */
  /*   Do not #undef these macros here since the build system might define */
  /*   them for certain configurations only.                               */
  /*                                                                       */
/* #define FT_DEBUG_LEVEL_ERROR */
/* #define FT_DEBUG_LEVEL_TRACE */


  /*************************************************************************/
  /*                                                                       */
  /* Memory Debugging                                                      */
  /*                                                                       */
  /*   FreeType now comes with an integrated memory debugger that is       */
  /*   capable of detecting simple errors like memory leaks or double      */
  /*   deletes.  To compile it within your build of the library, you       */
  /*   should define FT_DEBUG_MEMORY here.                                 */
  /*                                                                       */
  /*   Note that the memory debugger is only activated at runtime when     */
  /*   when the _environment_ variable `FT2_DEBUG_MEMORY' is defined also! */
  /*                                                                       */
  /*   Do not #undef this macro here since the build system might define   */
  /*   it for certain configurations only.                                 */
  /*                                                                       */
/* #define FT_DEBUG_MEMORY */


  /*************************************************************************/
  /*                                                                       */
  /* Module errors                                                         */
  /*                                                                       */
  /*   If this macro is set (which is _not_ the default), the higher byte  */
  /*   of an error code gives the module in which the error has occurred,  */
  /*   while the lower byte is the real error code.                        */
  /*                                                                       */
  /*   Setting this macro makes sense for debugging purposes only, since   */
  /*   it would break source compatibility of certain programs that use    */
  /*   FreeType 2.                                                         */
  /*                                                                       */
  /*   More details can be found in the files ftmoderr.h and fterrors.h.   */
  /*                                                                       */
#undef FT_CONFIG_OPTION_USE_MODULE_ERRORS


  /*************************************************************************/
  /*                                                                       */
  /* Position Independent Code                                             */
  /*                                                                       */
  /*   If this macro is set (which is _not_ the default), FreeType2 will   */
  /*   avoid creating constants that require address fixups.  Instead the  */
  /*   constants will be moved into a struct and additional intialization  */
  /*   code will be used.                                                  */
  /*                                                                       */
  /*   Setting this macro is needed for systems that prohibit address      */
  /*   fixups, such as BREW.                                               */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_PIC */


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****        S F N T   D R I V E R    C O N F I G U R A T I O N       ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_EMBEDDED_BITMAPS if you want to support       */
  /* embedded bitmaps in all formats using the SFNT module (namely         */
  /* TrueType & OpenType).                                                 */
  /*                                                                       */
#define TT_CONFIG_OPTION_EMBEDDED_BITMAPS


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_POSTSCRIPT_NAMES if you want to be able to    */
  /* load and enumerate the glyph Postscript names in a TrueType or        */
  /* OpenType file.                                                        */
  /*                                                                       */
  /* Note that when you do not compile the `PSNames' module by undefining  */
  /* the above FT_CONFIG_OPTION_POSTSCRIPT_NAMES, the `sfnt' module will   */
  /* contain additional code used to read the PS Names table from a font.  */
  /*                                                                       */
  /* (By default, the module uses `PSNames' to extract glyph names.)       */
  /*                                                                       */
#define TT_CONFIG_OPTION_POSTSCRIPT_NAMES


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_SFNT_NAMES if your applications need to       */
  /* access the internal name table in a SFNT-based format like TrueType   */
  /* or OpenType.  The name table contains various strings used to         */
  /* describe the font, like family name, copyright, version, etc.  It     */
  /* does not contain any glyph name though.                               */
  /*                                                                       */
  /* Accessing SFNT names is done through the functions declared in        */
  /* `freetype/ftsnames.h'.                                                */
  /*                                                                       */
#define TT_CONFIG_OPTION_SFNT_NAMES


  /*************************************************************************/
  /*                                                                       */
  /* TrueType CMap support                                                 */
  /*                                                                       */
  /*   Here you can fine-tune which TrueType CMap table format shall be    */
  /*   supported.                                                          */
#define TT_CONFIG_CMAP_FORMAT_0
#define TT_CONFIG_CMAP_FORMAT_2
#define TT_CONFIG_CMAP_FORMAT_4
#define TT_CONFIG_CMAP_FORMAT_6
#define TT_CONFIG_CMAP_FORMAT_8
#define TT_CONFIG_CMAP_FORMAT_10
#define TT_CONFIG_CMAP_FORMAT_12
#define TT_CONFIG_CMAP_FORMAT_13
#define TT_CONFIG_CMAP_FORMAT_14


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****    T R U E T Y P E   D R I V E R    C O N F I G U R A T I O N   ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_BYTECODE_INTERPRETER if you want to compile   */
  /* a bytecode interpreter in the TrueType driver.                        */
  /*                                                                       */
  /* By undefining this, you will only compile the code necessary to load  */
  /* TrueType glyphs without hinting.                                      */
  /*                                                                       */
  /*   Do not #undef this macro here, since the build system might         */
  /*   define it for certain configurations only.                          */
  /*                                                                       */
#define TT_CONFIG_OPTION_BYTECODE_INTERPRETER


  /*************************************************************************/
  /*                                                                       */
  /* If you define TT_CONFIG_OPTION_UNPATENTED_HINTING, a special version  */
  /* of the TrueType bytecode interpreter is used that doesn't implement   */
  /* any of the patented opcodes and algorithms.  The patents related to   */
  /* TrueType hinting have expired worldwide since May 2010; this option   */
  /* is now deprecated.                                                    */
  /*                                                                       */
  /* Note that the TT_CONFIG_OPTION_UNPATENTED_HINTING macro is *ignored*  */
  /* if you define TT_CONFIG_OPTION_BYTECODE_INTERPRETER; in other words,  */
  /* either define TT_CONFIG_OPTION_BYTECODE_INTERPRETER or                */
  /* TT_CONFIG_OPTION_UNPATENTED_HINTING but not both at the same time.    */
  /*                                                                       */
  /* This macro is only useful for a small number of font files (mostly    */
  /* for Asian scripts) that require bytecode interpretation to properly   */
  /* load glyphs.  For all other fonts, this produces unpleasant results,  */
  /* thus the unpatented interpreter is never used to load glyphs from     */
  /* TrueType fonts unless one of the following two options is used.       */
  /*                                                                       */
  /*   - The unpatented interpreter is explicitly activated by the user    */
  /*     through the FT_PARAM_TAG_UNPATENTED_HINTING parameter tag         */
  /*     when opening the FT_Face.                                         */
  /*                                                                       */
  /*   - FreeType detects that the FT_Face corresponds to one of the       */
  /*     `trick' fonts (e.g., `Mingliu') it knows about.  The font engine  */
  /*     contains a hard-coded list of font names and other matching       */
  /*     parameters (see function `tt_face_init' in file                   */
  /*     `src/truetype/ttobjs.c').                                         */
  /*                                                                       */
  /* Here a sample code snippet for using FT_PARAM_TAG_UNPATENTED_HINTING. */
  /*                                                                       */
  /*   {                                                                   */
  /*     FT_Parameter  parameter;                                          */
  /*     FT_Open_Args  open_args;                                          */
  /*                                                                       */
  /*                                                                       */
  /*     parameter.tag = FT_PARAM_TAG_UNPATENTED_HINTING;                  */
  /*                                                                       */
  /*     open_args.flags      = FT_OPEN_PATHNAME | FT_OPEN_PARAMS;         */
  /*     open_args.pathname   = my_font_pathname;                          */
  /*     open_args.num_params = 1;                                         */
  /*     open_args.params     = &parameter;                                */
  /*                                                                       */
  /*     error = FT_Open_Face( library, &open_args, index, &face );        */
  /*     ...                                                               */
  /*   }                                                                   */
  /*                                                                       */
/* #define TT_CONFIG_OPTION_UNPATENTED_HINTING */


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_INTERPRETER_SWITCH to compile the TrueType    */
  /* bytecode interpreter with a huge switch statement, rather than a call */
  /* table.  This results in smaller and faster code for a number of       */
  /* architectures.                                                        */
  /*                                                                       */
  /* Note however that on some compiler/processor combinations, undefining */
  /* this macro will generate faster, though larger, code.                 */
  /*                                                                       */
#define TT_CONFIG_OPTION_INTERPRETER_SWITCH


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_COMPONENT_OFFSET_SCALED to compile the        */
  /* TrueType glyph loader to use Apple's definition of how to handle      */
  /* component offsets in composite glyphs.                                */
  /*                                                                       */
  /* Apple and MS disagree on the default behavior of component offsets    */
  /* in composites.  Apple says that they should be scaled by the scaling  */
  /* factors in the transformation matrix (roughly, it's more complex)     */
  /* while MS says they should not.  OpenType defines two bits in the      */
  /* composite flags array which can be used to disambiguate, but old      */
  /* fonts will not have them.                                             */
  /*                                                                       */
  /*   http://partners.adobe.com/asn/developer/opentype/glyf.html          */
  /*   http://fonts.apple.com/TTRefMan/RM06/Chap6glyf.html                 */
  /*                                                                       */
#undef TT_CONFIG_OPTION_COMPONENT_OFFSET_SCALED


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_GX_VAR_SUPPORT if you want to include         */
  /* support for Apple's distortable font technology (fvar, gvar, cvar,    */
  /* and avar tables).  This has many similarities to Type 1 Multiple      */
  /* Masters support.                                                      */
  /*                                                                       */
#define TT_CONFIG_OPTION_GX_VAR_SUPPORT


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_BDF if you want to include support for        */
  /* an embedded `BDF ' table within SFNT-based bitmap formats.            */
  /*                                                                       */
#define TT_CONFIG_OPTION_BDF


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****      T Y P E 1   D R I V E R    C O N F I G U R A T I O N       ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_DICT_DEPTH is the maximal depth of nest dictionaries and       */
  /* arrays in the Type 1 stream (see t1load.c).  A minimum of 4 is        */
  /* required.                                                             */
  /*                                                                       */
#define T1_MAX_DICT_DEPTH  5


  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_SUBRS_CALLS details the maximum number of nested sub-routine   */
  /* calls during glyph loading.                                           */
  /*                                                                       */
#define T1_MAX_SUBRS_CALLS  16


  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_CHARSTRING_OPERANDS is the charstring stack's capacity.  A     */
  /* minimum of 16 is required.                                            */
  /*                                                                       */
  /* The Chinese font MingTiEG-Medium (CNS 11643 character set) needs 256. */
  /*                                                                       */
#define T1_MAX_CHARSTRINGS_OPERANDS  256


  /*************************************************************************/
  /*                                                                       */
  /* Define this configuration macro if you want to prevent the            */
  /* compilation of `t1afm', which is in charge of reading Type 1 AFM      */
  /* files into an existing face.  Note that if set, the T1 driver will be */
  /* unable to produce kerning distances.                                  */
  /*                                                                       */
#undef T1_CONFIG_OPTION_NO_AFM


  /*************************************************************************/
  /*                                                                       */
  /* Define this configuration macro if you want to prevent the            */
  /* compilation of the Multiple Masters font support in the Type 1        */
  /* driver.                                                               */
  /*                                                                       */
#undef T1_CONFIG_OPTION_NO_MM_SUPPORT


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****    A U T O F I T   M O D U L E    C O N F I G U R A T I O N     ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Compile autofit module with CJK (Chinese, Japanese, Korean) script    */
  /* support.                                                              */
  /*                                                                       */
#define AF_CONFIG_OPTION_CJK

  /*************************************************************************/
  /*                                                                       */
  /* Compile autofit module with Indic script support.                     */
  /*                                                                       */
#define AF_CONFIG_OPTION_INDIC

  /* */


  /*
   * Define this variable if you want to keep the layout of internal
   * structures that was used prior to FreeType 2.2.  This also compiles in
   * a few obsolete functions to avoid linking problems on typical Unix
   * distributions.
   *
   * For embedded systems or building a new distribution from scratch, it
   * is recommended to disable the macro since it reduces the library's code
   * size and activates a few memory-saving optimizations as well.
   */
#define FT_CONFIG_OPTION_OLD_INTERNALS


  /*
   *  To detect legacy cache-lookup call from a rogue client (<= 2.1.7),
   *  we restrict the number of charmaps in a font.  The current API of
   *  FTC_CMapCache_Lookup() takes cmap_index & charcode, but old API
   *  takes charcode only.  To determine the passed value is for cmap_index
   *  or charcode, the possible cmap_index is restricted not to exceed
   *  the minimum possible charcode by a rogue client.  It is also very
   *  unlikely that a rogue client is interested in Unicode values 0 to 15.
   *
   *  NOTE: The original threshold was 4 deduced from popular number of
   *        cmap subtables in UCS-4 TrueType fonts, but now it is not
   *        irregular for OpenType fonts to have more than 4 subtables,
   *        because variation selector subtables are available for Apple
   *        and Microsoft platforms.
   */

#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
#define FT_MAX_CHARMAP_CACHEABLE 15
#endif


  /*
   * This macro is defined if either unpatented or native TrueType
   * hinting is requested by the definitions above.
   */
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#define  TT_USE_BYTECODE_INTERPRETER
#undef   TT_CONFIG_OPTION_UNPATENTED_HINTING
#elif defined TT_CONFIG_OPTION_UNPATENTED_HINTING
#define  TT_USE_BYTECODE_INTERPRETER
#endif

FT_END_HEADER


#endif /* __FTOPTION_H__ */


/* END */
