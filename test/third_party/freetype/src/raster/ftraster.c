/***************************************************************************/
/*                                                                         */
/*  ftraster.c                                                             */
/*                                                                         */
/*    The FreeType glyph rasterizer (body).                                */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2005, 2007, 2008, 2009, 2010 by       */
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
  /* This file can be compiled without the rest of the FreeType engine, by */
  /* defining the _STANDALONE_ macro when compiling it.  You also need to  */
  /* put the files `ftimage.h' and `ftmisc.h' into the $(incdir)           */
  /* directory.  Typically, you should do something like                   */
  /*                                                                       */
  /* - copy `src/raster/ftraster.c' (this file) to your current directory  */
  /*                                                                       */
  /* - copy `include/freetype/ftimage.h' and `src/raster/ftmisc.h'         */
  /*   to your current directory                                           */
  /*                                                                       */
  /* - compile `ftraster' with the _STANDALONE_ macro defined, as in       */
  /*                                                                       */
  /*     cc -c -D_STANDALONE_ ftraster.c                                   */
  /*                                                                       */
  /* The renderer can be initialized with a call to                        */
  /* `ft_standard_raster.raster_new'; a bitmap can be generated            */
  /* with a call to `ft_standard_raster.raster_render'.                    */
  /*                                                                       */
  /* See the comments and documentation in the file `ftimage.h' for more   */
  /* details on how the raster works.                                      */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This is a rewrite of the FreeType 1.x scan-line converter             */
  /*                                                                       */
  /*************************************************************************/

#ifdef _STANDALONE_

#define FT_CONFIG_STANDARD_LIBRARY_H  <stdlib.h>

#include <string.h>           /* for memset */

#include "ftmisc.h"
#include "ftimage.h"

#else /* !_STANDALONE_ */

#include <ft2build.h>
#include "ftraster.h"
#include FT_INTERNAL_CALC_H   /* for FT_MulDiv only */

#include "rastpic.h"

#endif /* !_STANDALONE_ */


  /*************************************************************************/
  /*                                                                       */
  /* A simple technical note on how the raster works                       */
  /* -----------------------------------------------                       */
  /*                                                                       */
  /*   Converting an outline into a bitmap is achieved in several steps:   */
  /*                                                                       */
  /*   1 - Decomposing the outline into successive `profiles'.  Each       */
  /*       profile is simply an array of scanline intersections on a given */
  /*       dimension.  A profile's main attributes are                     */
  /*                                                                       */
  /*       o its scanline position boundaries, i.e. `Ymin' and `Ymax'      */
  /*                                                                       */
  /*       o an array of intersection coordinates for each scanline        */
  /*         between `Ymin' and `Ymax'                                     */
  /*                                                                       */
  /*       o a direction, indicating whether it was built going `up' or    */
  /*         `down', as this is very important for filling rules           */
  /*                                                                       */
  /*       o its drop-out mode                                             */
  /*                                                                       */
  /*   2 - Sweeping the target map's scanlines in order to compute segment */
  /*       `spans' which are then filled.  Additionally, this pass         */
  /*       performs drop-out control.                                      */
  /*                                                                       */
  /*   The outline data is parsed during step 1 only.  The profiles are    */
  /*   built from the bottom of the render pool, used as a stack.  The     */
  /*   following graphics shows the profile list under construction:       */
  /*                                                                       */
  /*     __________________________________________________________ _ _    */
  /*    |         |                 |         |                 |          */
  /*    | profile | coordinates for | profile | coordinates for |-->       */
  /*    |    1    |  profile 1      |    2    |  profile 2      |-->       */
  /*    |_________|_________________|_________|_________________|__ _ _    */
  /*                                                                       */
  /*    ^                                                       ^          */
  /*    |                                                       |          */
  /* start of render pool                                      top         */
  /*                                                                       */
  /*   The top of the profile stack is kept in the `top' variable.         */
  /*                                                                       */
  /*   As you can see, a profile record is pushed on top of the render     */
  /*   pool, which is then followed by its coordinates/intersections.  If  */
  /*   a change of direction is detected in the outline, a new profile is  */
  /*   generated until the end of the outline.                             */
  /*                                                                       */
  /*   Note that when all profiles have been generated, the function       */
  /*   Finalize_Profile_Table() is used to record, for each profile, its   */
  /*   bottom-most scanline as well as the scanline above its upmost       */
  /*   boundary.  These positions are called `y-turns' because they (sort  */
  /*   of) correspond to local extrema.  They are stored in a sorted list  */
  /*   built from the top of the render pool as a downwards stack:         */
  /*                                                                       */
  /*      _ _ _______________________________________                      */
  /*                            |                    |                     */
  /*                         <--| sorted list of     |                     */
  /*                         <--|  extrema scanlines |                     */
  /*      _ _ __________________|____________________|                     */
  /*                                                                       */
  /*                            ^                    ^                     */
  /*                            |                    |                     */
  /*                         maxBuff           sizeBuff = end of pool      */
  /*                                                                       */
  /*   This list is later used during the sweep phase in order to          */
  /*   optimize performance (see technical note on the sweep below).       */
  /*                                                                       */
  /*   Of course, the raster detects whether the two stacks collide and    */
  /*   handles the situation properly.                                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  CONFIGURATION MACROS                                               **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/

  /* define DEBUG_RASTER if you want to compile a debugging version */
/* #define DEBUG_RASTER */

  /* define FT_RASTER_OPTION_ANTI_ALIASING if you want to support */
  /* 5-levels anti-aliasing                                       */
/* #define FT_RASTER_OPTION_ANTI_ALIASING */

  /* The size of the two-lines intermediate bitmap used */
  /* for anti-aliasing, in bytes.                       */
#define RASTER_GRAY_LINES  2048


  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  OTHER MACROS (do not change)                                       **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_raster


#ifdef _STANDALONE_


  /* This macro is used to indicate that a function parameter is unused. */
  /* Its purpose is simply to reduce compiler warnings.  Note also that  */
  /* simply defining it as `(void)x' doesn't avoid warnings with certain */
  /* ANSI compilers (e.g. LCC).                                          */
#define FT_UNUSED( x )  (x) = (x)

  /* Disable the tracing mechanism for simplicity -- developers can      */
  /* activate it easily by redefining these two macros.                  */
#ifndef FT_ERROR
#define FT_ERROR( x )  do { } while ( 0 )     /* nothing */
#endif

#ifndef FT_TRACE
#define FT_TRACE( x )   do { } while ( 0 )    /* nothing */
#define FT_TRACE1( x )  do { } while ( 0 )    /* nothing */
#define FT_TRACE6( x )  do { } while ( 0 )    /* nothing */
#endif

#define Raster_Err_None          0
#define Raster_Err_Not_Ini      -1
#define Raster_Err_Overflow     -2
#define Raster_Err_Neg_Height   -3
#define Raster_Err_Invalid      -4
#define Raster_Err_Unsupported  -5

#define ft_memset  memset

#define FT_DEFINE_RASTER_FUNCS( class_, glyph_format_, raster_new_, \
                                raster_reset_, raster_set_mode_,    \
                                raster_render_, raster_done_ )      \
          const FT_Raster_Funcs class_ =                            \
          {                                                         \
            glyph_format_,                                          \
            raster_new_,                                            \
            raster_reset_,                                          \
            raster_set_mode_,                                       \
            raster_render_,                                         \
            raster_done_                                            \
         };

#else /* !_STANDALONE_ */


#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H        /* for FT_TRACE() and FT_ERROR() */

#include "rasterrs.h"

#define Raster_Err_None         Raster_Err_Ok
#define Raster_Err_Not_Ini      Raster_Err_Raster_Uninitialized
#define Raster_Err_Overflow     Raster_Err_Raster_Overflow
#define Raster_Err_Neg_Height   Raster_Err_Raster_Negative_Height
#define Raster_Err_Invalid      Raster_Err_Invalid_Outline
#define Raster_Err_Unsupported  Raster_Err_Cannot_Render_Glyph


#endif /* !_STANDALONE_ */


#ifndef FT_MEM_SET
#define FT_MEM_SET( d, s, c )  ft_memset( d, s, c )
#endif

#ifndef FT_MEM_ZERO
#define FT_MEM_ZERO( dest, count )  FT_MEM_SET( dest, 0, count )
#endif

  /* FMulDiv means `Fast MulDiv'; it is used in case where `b' is       */
  /* typically a small value and the result of a*b is known to fit into */
  /* 32 bits.                                                           */
#define FMulDiv( a, b, c )  ( (a) * (b) / (c) )

  /* On the other hand, SMulDiv means `Slow MulDiv', and is used typically */
  /* for clipping computations.  It simply uses the FT_MulDiv() function   */
  /* defined in `ftcalc.h'.                                                */
#define SMulDiv  FT_MulDiv

  /* The rasterizer is a very general purpose component; please leave */
  /* the following redefinitions there (you never know your target    */
  /* environment).                                                    */

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  (void*)0
#endif

#ifndef SUCCESS
#define SUCCESS  0
#endif

#ifndef FAILURE
#define FAILURE  1
#endif


#define MaxBezier  32   /* The maximum number of stacked Bezier curves. */
                        /* Setting this constant to more than 32 is a   */
                        /* pure waste of space.                         */

#define Pixel_Bits  6   /* fractional bits of *input* coordinates */


  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  SIMPLE TYPE DECLARATIONS                                           **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/

  typedef int             Int;
  typedef unsigned int    UInt;
  typedef short           Short;
  typedef unsigned short  UShort, *PUShort;
  typedef long            Long, *PLong;

  typedef unsigned char   Byte, *PByte;
  typedef char            Bool;


  typedef union  Alignment_
  {
    long    l;
    void*   p;
    void  (*f)(void);

  } Alignment, *PAlignment;


  typedef struct  TPoint_
  {
    Long  x;
    Long  y;

  } TPoint;


  /* values for the `flags' bit field */
#define Flow_Up           0x8
#define Overshoot_Top     0x10
#define Overshoot_Bottom  0x20


  /* States of each line, arc, and profile */
  typedef enum  TStates_
  {
    Unknown_State,
    Ascending_State,
    Descending_State,
    Flat_State

  } TStates;


  typedef struct TProfile_  TProfile;
  typedef TProfile*         PProfile;

  struct  TProfile_
  {
    FT_F26Dot6  X;           /* current coordinate during sweep          */
    PProfile    link;        /* link to next profile (various purposes)  */
    PLong       offset;      /* start of profile's data in render pool   */
    unsigned    flags;       /* Bit 0-2: drop-out mode                   */
                             /* Bit 3: profile orientation (up/down)     */
                             /* Bit 4: is top profile?                   */
                             /* Bit 5: is bottom profile?                */
    long        height;      /* profile's height in scanlines            */
    long        start;       /* profile's starting scanline              */

    unsigned    countL;      /* number of lines to step before this      */
                             /* profile becomes drawable                 */

    PProfile    next;        /* next profile in same contour, used       */
                             /* during drop-out control                  */
  };

  typedef PProfile   TProfileList;
  typedef PProfile*  PProfileList;


  /* Simple record used to implement a stack of bands, required */
  /* by the sub-banding mechanism                               */
  typedef struct  TBand_
  {
    Short  y_min;   /* band's minimum */
    Short  y_max;   /* band's maximum */

  } TBand;


#define AlignProfileSize \
  ( ( sizeof ( TProfile ) + sizeof ( Alignment ) - 1 ) / sizeof ( long ) )


#ifdef FT_STATIC_RASTER


#define RAS_ARGS       /* void */
#define RAS_ARG        /* void */

#define RAS_VARS       /* void */
#define RAS_VAR        /* void */

#define FT_UNUSED_RASTER  do { } while ( 0 )


#else /* !FT_STATIC_RASTER */


#define RAS_ARGS       PWorker    worker,
#define RAS_ARG        PWorker    worker

#define RAS_VARS       worker,
#define RAS_VAR        worker

#define FT_UNUSED_RASTER  FT_UNUSED( worker )


#endif /* !FT_STATIC_RASTER */


  typedef struct TWorker_  TWorker, *PWorker;


  /* prototypes used for sweep function dispatch */
  typedef void
  Function_Sweep_Init( RAS_ARGS Short*  min,
                                Short*  max );

  typedef void
  Function_Sweep_Span( RAS_ARGS Short       y,
                                FT_F26Dot6  x1,
                                FT_F26Dot6  x2,
                                PProfile    left,
                                PProfile    right );

  typedef void
  Function_Sweep_Step( RAS_ARG );


  /* NOTE: These operations are only valid on 2's complement processors */

#define FLOOR( x )    ( (x) & -ras.precision )
#define CEILING( x )  ( ( (x) + ras.precision - 1 ) & -ras.precision )
#define TRUNC( x )    ( (signed long)(x) >> ras.precision_bits )
#define FRAC( x )     ( (x) & ( ras.precision - 1 ) )
#define SCALED( x )   ( ( (x) << ras.scale_shift ) - ras.precision_half )

#define IS_BOTTOM_OVERSHOOT( x )  ( CEILING( x ) - x >= ras.precision_half )
#define IS_TOP_OVERSHOOT( x )     ( x - FLOOR( x ) >= ras.precision_half )

  /* The most used variables are positioned at the top of the structure. */
  /* Thus, their offset can be coded with less opcodes, resulting in a   */
  /* smaller executable.                                                 */

  struct  TWorker_
  {
    Int         precision_bits;     /* precision related variables         */
    Int         precision;
    Int         precision_half;
    Int         precision_shift;
    Int         precision_step;
    Int         precision_jitter;

    Int         scale_shift;        /* == precision_shift   for bitmaps    */
                                    /* == precision_shift+1 for pixmaps    */

    PLong       buff;               /* The profiles buffer                 */
    PLong       sizeBuff;           /* Render pool size                    */
    PLong       maxBuff;            /* Profiles buffer size                */
    PLong       top;                /* Current cursor in buffer            */

    FT_Error    error;

    Int         numTurns;           /* number of Y-turns in outline        */

    TPoint*     arc;                /* current Bezier arc pointer          */

    UShort      bWidth;             /* target bitmap width                 */
    PByte       bTarget;            /* target bitmap buffer                */
    PByte       gTarget;            /* target pixmap buffer                */

    Long        lastX, lastY;
    Long        minY, maxY;

    UShort      num_Profs;          /* current number of profiles          */

    Bool        fresh;              /* signals a fresh new profile which   */
                                    /* `start' field must be completed     */
    Bool        joint;              /* signals that the last arc ended     */
                                    /* exactly on a scanline.  Allows      */
                                    /* removal of doublets                 */
    PProfile    cProfile;           /* current profile                     */
    PProfile    fProfile;           /* head of linked list of profiles     */
    PProfile    gProfile;           /* contour's first profile in case     */
                                    /* of impact                           */

    TStates     state;              /* rendering state                     */

    FT_Bitmap   target;             /* description of target bit/pixmap    */
    FT_Outline  outline;

    Long        traceOfs;           /* current offset in target bitmap     */
    Long        traceG;             /* current offset in target pixmap     */

    Short       traceIncr;          /* sweep's increment in target bitmap  */

    Short       gray_min_x;         /* current min x during gray rendering */
    Short       gray_max_x;         /* current max x during gray rendering */

    /* dispatch variables */

    Function_Sweep_Init*  Proc_Sweep_Init;
    Function_Sweep_Span*  Proc_Sweep_Span;
    Function_Sweep_Span*  Proc_Sweep_Drop;
    Function_Sweep_Step*  Proc_Sweep_Step;

    Byte        dropOutControl;     /* current drop_out control method     */

    Bool        second_pass;        /* indicates whether a horizontal pass */
                                    /* should be performed to control      */
                                    /* drop-out accurately when calling    */
                                    /* Render_Glyph.  Note that there is   */
                                    /* no horizontal pass during gray      */
                                    /* rendering.                          */

    TPoint      arcs[3 * MaxBezier + 1]; /* The Bezier stack               */

    TBand       band_stack[16];     /* band stack used for sub-banding     */
    Int         band_top;           /* band stack top                      */

#ifdef FT_RASTER_OPTION_ANTI_ALIASING

    Byte*       grays;

    Byte        gray_lines[RASTER_GRAY_LINES];
                                /* Intermediate table used to render the   */
                                /* graylevels pixmaps.                     */
                                /* gray_lines is a buffer holding two      */
                                /* monochrome scanlines                    */

    Short       gray_width;     /* width in bytes of one monochrome        */
                                /* intermediate scanline of gray_lines.    */
                                /* Each gray pixel takes 2 bits long there */

                       /* The gray_lines must hold 2 lines, thus with size */
                       /* in bytes of at least `gray_width*2'.             */

#endif /* FT_RASTER_ANTI_ALIASING */

  };


  typedef struct  TRaster_
  {
    char*    buffer;
    long     buffer_size;
    void*    memory;
    PWorker  worker;
    Byte     grays[5];
    Short    gray_width;

  } TRaster, *PRaster;

#ifdef FT_STATIC_RASTER

  static TWorker  cur_ras;
#define ras  cur_ras

#else /* !FT_STATIC_RASTER */

#define ras  (*worker)

#endif /* !FT_STATIC_RASTER */


#ifdef FT_RASTER_OPTION_ANTI_ALIASING

  /* A lookup table used to quickly count set bits in four gray 2x2 */
  /* cells.  The values of the table have been produced with the    */
  /* following code:                                                */
  /*                                                                */
  /*   for ( i = 0; i < 256; i++ )                                  */
  /*   {                                                            */
  /*     l = 0;                                                     */
  /*     j = i;                                                     */
  /*                                                                */
  /*     for ( c = 0; c < 4; c++ )                                  */
  /*     {                                                          */
  /*       l <<= 4;                                                 */
  /*                                                                */
  /*       if ( j & 0x80 ) l++;                                     */
  /*       if ( j & 0x40 ) l++;                                     */
  /*                                                                */
  /*       j = ( j << 2 ) & 0xFF;                                   */
  /*     }                                                          */
  /*     printf( "0x%04X", l );                                     */
  /*   }                                                            */
  /*                                                                */

  static const short  count_table[256] =
  {
    0x0000, 0x0001, 0x0001, 0x0002, 0x0010, 0x0011, 0x0011, 0x0012,
    0x0010, 0x0011, 0x0011, 0x0012, 0x0020, 0x0021, 0x0021, 0x0022,
    0x0100, 0x0101, 0x0101, 0x0102, 0x0110, 0x0111, 0x0111, 0x0112,
    0x0110, 0x0111, 0x0111, 0x0112, 0x0120, 0x0121, 0x0121, 0x0122,
    0x0100, 0x0101, 0x0101, 0x0102, 0x0110, 0x0111, 0x0111, 0x0112,
    0x0110, 0x0111, 0x0111, 0x0112, 0x0120, 0x0121, 0x0121, 0x0122,
    0x0200, 0x0201, 0x0201, 0x0202, 0x0210, 0x0211, 0x0211, 0x0212,
    0x0210, 0x0211, 0x0211, 0x0212, 0x0220, 0x0221, 0x0221, 0x0222,
    0x1000, 0x1001, 0x1001, 0x1002, 0x1010, 0x1011, 0x1011, 0x1012,
    0x1010, 0x1011, 0x1011, 0x1012, 0x1020, 0x1021, 0x1021, 0x1022,
    0x1100, 0x1101, 0x1101, 0x1102, 0x1110, 0x1111, 0x1111, 0x1112,
    0x1110, 0x1111, 0x1111, 0x1112, 0x1120, 0x1121, 0x1121, 0x1122,
    0x1100, 0x1101, 0x1101, 0x1102, 0x1110, 0x1111, 0x1111, 0x1112,
    0x1110, 0x1111, 0x1111, 0x1112, 0x1120, 0x1121, 0x1121, 0x1122,
    0x1200, 0x1201, 0x1201, 0x1202, 0x1210, 0x1211, 0x1211, 0x1212,
    0x1210, 0x1211, 0x1211, 0x1212, 0x1220, 0x1221, 0x1221, 0x1222,
    0x1000, 0x1001, 0x1001, 0x1002, 0x1010, 0x1011, 0x1011, 0x1012,
    0x1010, 0x1011, 0x1011, 0x1012, 0x1020, 0x1021, 0x1021, 0x1022,
    0x1100, 0x1101, 0x1101, 0x1102, 0x1110, 0x1111, 0x1111, 0x1112,
    0x1110, 0x1111, 0x1111, 0x1112, 0x1120, 0x1121, 0x1121, 0x1122,
    0x1100, 0x1101, 0x1101, 0x1102, 0x1110, 0x1111, 0x1111, 0x1112,
    0x1110, 0x1111, 0x1111, 0x1112, 0x1120, 0x1121, 0x1121, 0x1122,
    0x1200, 0x1201, 0x1201, 0x1202, 0x1210, 0x1211, 0x1211, 0x1212,
    0x1210, 0x1211, 0x1211, 0x1212, 0x1220, 0x1221, 0x1221, 0x1222,
    0x2000, 0x2001, 0x2001, 0x2002, 0x2010, 0x2011, 0x2011, 0x2012,
    0x2010, 0x2011, 0x2011, 0x2012, 0x2020, 0x2021, 0x2021, 0x2022,
    0x2100, 0x2101, 0x2101, 0x2102, 0x2110, 0x2111, 0x2111, 0x2112,
    0x2110, 0x2111, 0x2111, 0x2112, 0x2120, 0x2121, 0x2121, 0x2122,
    0x2100, 0x2101, 0x2101, 0x2102, 0x2110, 0x2111, 0x2111, 0x2112,
    0x2110, 0x2111, 0x2111, 0x2112, 0x2120, 0x2121, 0x2121, 0x2122,
    0x2200, 0x2201, 0x2201, 0x2202, 0x2210, 0x2211, 0x2211, 0x2212,
    0x2210, 0x2211, 0x2211, 0x2212, 0x2220, 0x2221, 0x2221, 0x2222
  };

#endif /* FT_RASTER_OPTION_ANTI_ALIASING */



  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  PROFILES COMPUTATION                                               **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Set_High_Precision                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Set precision variables according to param flag.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    High :: Set to True for high precision (typically for ppem < 18),  */
  /*            false otherwise.                                           */
  /*                                                                       */
  static void
  Set_High_Precision( RAS_ARGS Int  High )
  {
    if ( High )
    {
      ras.precision_bits   = 12;
      ras.precision_step   = 256;
      ras.precision_jitter = 50;
    }
    else
    {
      ras.precision_bits   = 6;
      ras.precision_step   = 32;
      ras.precision_jitter = 2;
    }

    FT_TRACE6(( "Set_High_Precision(%s)\n", High ? "true" : "false" ));

    ras.precision       = 1 << ras.precision_bits;
    ras.precision_half  = ras.precision / 2;
    ras.precision_shift = ras.precision_bits - Pixel_Bits;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    New_Profile                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a new profile in the render pool.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    aState    :: The state/orientation of the new profile.             */
  /*                                                                       */
  /*    overshoot :: Whether the profile's unrounded start position        */
  /*                 differs by at least a half pixel.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*   SUCCESS on success.  FAILURE in case of overflow or of incoherent   */
  /*   profile.                                                            */
  /*                                                                       */
  static Bool
  New_Profile( RAS_ARGS TStates  aState,
                        Bool     overshoot )
  {
    if ( !ras.fProfile )
    {
      ras.cProfile  = (PProfile)ras.top;
      ras.fProfile  = ras.cProfile;
      ras.top      += AlignProfileSize;
    }

    if ( ras.top >= ras.maxBuff )
    {
      ras.error = Raster_Err_Overflow;
      return FAILURE;
    }

    ras.cProfile->flags  = 0;
    ras.cProfile->start  = 0;
    ras.cProfile->height = 0;
    ras.cProfile->offset = ras.top;
    ras.cProfile->link   = (PProfile)0;
    ras.cProfile->next   = (PProfile)0;
    ras.cProfile->flags  = ras.dropOutControl;

    switch ( aState )
    {
    case Ascending_State:
      ras.cProfile->flags |= Flow_Up;
      if ( overshoot )
        ras.cProfile->flags |= Overshoot_Bottom;

      FT_TRACE6(( "New ascending profile = %p\n", ras.cProfile ));
      break;

    case Descending_State:
      if ( overshoot )
        ras.cProfile->flags |= Overshoot_Top;
      FT_TRACE6(( "New descending profile = %p\n", ras.cProfile ));
      break;

    default:
      FT_ERROR(( "New_Profile: invalid profile direction\n" ));
      ras.error = Raster_Err_Invalid;
      return FAILURE;
    }

    if ( !ras.gProfile )
      ras.gProfile = ras.cProfile;

    ras.state = aState;
    ras.fresh = TRUE;
    ras.joint = FALSE;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    End_Profile                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalize the current profile.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    overshoot :: Whether the profile's unrounded end position differs  */
  /*                 by at least a half pixel.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success.  FAILURE in case of overflow or incoherency.   */
  /*                                                                       */
  static Bool
  End_Profile( RAS_ARGS Bool  overshoot )
  {
    Long      h;
    PProfile  oldProfile;


    h = (Long)( ras.top - ras.cProfile->offset );

    if ( h < 0 )
    {
      FT_ERROR(( "End_Profile: negative height encountered\n" ));
      ras.error = Raster_Err_Neg_Height;
      return FAILURE;
    }

    if ( h > 0 )
    {
      FT_TRACE6(( "Ending profile %p, start = %ld, height = %ld\n",
                  ras.cProfile, ras.cProfile->start, h ));

      ras.cProfile->height = h;
      if ( overshoot )
      {
        if ( ras.cProfile->flags & Flow_Up )
          ras.cProfile->flags |= Overshoot_Top;
        else
          ras.cProfile->flags |= Overshoot_Bottom;
      }

      oldProfile   = ras.cProfile;
      ras.cProfile = (PProfile)ras.top;

      ras.top += AlignProfileSize;

      ras.cProfile->height = 0;
      ras.cProfile->offset = ras.top;

      oldProfile->next = ras.cProfile;
      ras.num_Profs++;
    }

    if ( ras.top >= ras.maxBuff )
    {
      FT_TRACE1(( "overflow in End_Profile\n" ));
      ras.error = Raster_Err_Overflow;
      return FAILURE;
    }

    ras.joint = FALSE;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Insert_Y_Turn                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Insert a salient into the sorted list placed on top of the render  */
  /*    pool.                                                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    New y scanline position.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success.  FAILURE in case of overflow.                  */
  /*                                                                       */
  static Bool
  Insert_Y_Turn( RAS_ARGS Int  y )
  {
    PLong  y_turns;
    Int    y2, n;


    n       = ras.numTurns - 1;
    y_turns = ras.sizeBuff - ras.numTurns;

    /* look for first y value that is <= */
    while ( n >= 0 && y < y_turns[n] )
      n--;

    /* if it is <, simply insert it, ignore if == */
    if ( n >= 0 && y > y_turns[n] )
      while ( n >= 0 )
      {
        y2 = (Int)y_turns[n];
        y_turns[n] = y;
        y = y2;
        n--;
      }

    if ( n < 0 )
    {
      ras.maxBuff--;
      if ( ras.maxBuff <= ras.top )
      {
        ras.error = Raster_Err_Overflow;
        return FAILURE;
      }
      ras.numTurns++;
      ras.sizeBuff[-ras.numTurns] = y;
    }

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Finalize_Profile_Table                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adjust all links in the profiles list.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success.  FAILURE in case of overflow.                  */
  /*                                                                       */
  static Bool
  Finalize_Profile_Table( RAS_ARG )
  {
    Int       bottom, top;
    UShort    n;
    PProfile  p;


    n = ras.num_Profs;
    p = ras.fProfile;

    if ( n > 1 && p )
    {
      while ( n > 0 )
      {
        if ( n > 1 )
          p->link = (PProfile)( p->offset + p->height );
        else
          p->link = NULL;

        if ( p->flags & Flow_Up )
        {
          bottom = (Int)p->start;
          top    = (Int)( p->start + p->height - 1 );
        }
        else
        {
          bottom     = (Int)( p->start - p->height + 1 );
          top        = (Int)p->start;
          p->start   = bottom;
          p->offset += p->height - 1;
        }

        if ( Insert_Y_Turn( RAS_VARS bottom )  ||
             Insert_Y_Turn( RAS_VARS top + 1 ) )
          return FAILURE;

        p = p->link;
        n--;
      }
    }
    else
      ras.fProfile = NULL;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Split_Conic                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Subdivide one conic Bezier into two joint sub-arcs in the Bezier   */
  /*    stack.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    None (subdivided Bezier is taken from the top of the stack).       */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This routine is the `beef' of this component.  It is  _the_ inner  */
  /*    loop that should be optimized to hell to get the best performance. */
  /*                                                                       */
  static void
  Split_Conic( TPoint*  base )
  {
    Long  a, b;


    base[4].x = base[2].x;
    b = base[1].x;
    a = base[3].x = ( base[2].x + b ) / 2;
    b = base[1].x = ( base[0].x + b ) / 2;
    base[2].x = ( a + b ) / 2;

    base[4].y = base[2].y;
    b = base[1].y;
    a = base[3].y = ( base[2].y + b ) / 2;
    b = base[1].y = ( base[0].y + b ) / 2;
    base[2].y = ( a + b ) / 2;

    /* hand optimized.  gcc doesn't seem to be too good at common      */
    /* expression substitution and instruction scheduling ;-)          */
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Split_Cubic                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Subdivide a third-order Bezier arc into two joint sub-arcs in the  */
  /*    Bezier stack.                                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This routine is the `beef' of the component.  It is one of _the_   */
  /*    inner loops that should be optimized like hell to get the best     */
  /*    performance.                                                       */
  /*                                                                       */
  static void
  Split_Cubic( TPoint*  base )
  {
    Long  a, b, c, d;


    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = ( base[0].x + c + 1 ) >> 1;
    base[5].x = b = ( base[3].x + d + 1 ) >> 1;
    c = ( c + d + 1 ) >> 1;
    base[2].x = a = ( a + c + 1 ) >> 1;
    base[4].x = b = ( b + c + 1 ) >> 1;
    base[3].x = ( a + b + 1 ) >> 1;

    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = ( base[0].y + c + 1 ) >> 1;
    base[5].y = b = ( base[3].y + d + 1 ) >> 1;
    c = ( c + d + 1 ) >> 1;
    base[2].y = a = ( a + c + 1 ) >> 1;
    base[4].y = b = ( b + c + 1 ) >> 1;
    base[3].y = ( a + b + 1 ) >> 1;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_Up                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Compute the x-coordinates of an ascending line segment and store   */
  /*    them in the render pool.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x1   :: The x-coordinate of the segment's start point.             */
  /*                                                                       */
  /*    y1   :: The y-coordinate of the segment's start point.             */
  /*                                                                       */
  /*    x2   :: The x-coordinate of the segment's end point.               */
  /*                                                                       */
  /*    y2   :: The y-coordinate of the segment's end point.               */
  /*                                                                       */
  /*    miny :: A lower vertical clipping bound value.                     */
  /*                                                                       */
  /*    maxy :: An upper vertical clipping bound value.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE on render pool overflow.               */
  /*                                                                       */
  static Bool
  Line_Up( RAS_ARGS Long  x1,
                    Long  y1,
                    Long  x2,
                    Long  y2,
                    Long  miny,
                    Long  maxy )
  {
    Long   Dx, Dy;
    Int    e1, e2, f1, f2, size;     /* XXX: is `Short' sufficient? */
    Long   Ix, Rx, Ax;

    PLong  top;


    Dx = x2 - x1;
    Dy = y2 - y1;

    if ( Dy <= 0 || y2 < miny || y1 > maxy )
      return SUCCESS;

    if ( y1 < miny )
    {
      /* Take care: miny-y1 can be a very large value; we use     */
      /*            a slow MulDiv function to avoid clipping bugs */
      x1 += SMulDiv( Dx, miny - y1, Dy );
      e1  = (Int)TRUNC( miny );
      f1  = 0;
    }
    else
    {
      e1 = (Int)TRUNC( y1 );
      f1 = (Int)FRAC( y1 );
    }

    if ( y2 > maxy )
    {
      /* x2 += FMulDiv( Dx, maxy - y2, Dy );  UNNECESSARY */
      e2  = (Int)TRUNC( maxy );
      f2  = 0;
    }
    else
    {
      e2 = (Int)TRUNC( y2 );
      f2 = (Int)FRAC( y2 );
    }

    if ( f1 > 0 )
    {
      if ( e1 == e2 )
        return SUCCESS;
      else
      {
        x1 += SMulDiv( Dx, ras.precision - f1, Dy );
        e1 += 1;
      }
    }
    else
      if ( ras.joint )
      {
        ras.top--;
        ras.joint = FALSE;
      }

    ras.joint = (char)( f2 == 0 );

    if ( ras.fresh )
    {
      ras.cProfile->start = e1;
      ras.fresh           = FALSE;
    }

    size = e2 - e1 + 1;
    if ( ras.top + size >= ras.maxBuff )
    {
      ras.error = Raster_Err_Overflow;
      return FAILURE;
    }

    if ( Dx > 0 )
    {
      Ix = SMulDiv( ras.precision, Dx, Dy);
      Rx = ( ras.precision * Dx ) % Dy;
      Dx = 1;
    }
    else
    {
      Ix = SMulDiv( ras.precision, -Dx, Dy) * -1;
      Rx =    ( ras.precision * -Dx ) % Dy;
      Dx = -1;
    }

    Ax  = -Dy;
    top = ras.top;

    while ( size > 0 )
    {
      *top++ = x1;

      x1 += Ix;
      Ax += Rx;
      if ( Ax >= 0 )
      {
        Ax -= Dy;
        x1 += Dx;
      }
      size--;
    }

    ras.top = top;
    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_Down                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Compute the x-coordinates of an descending line segment and store  */
  /*    them in the render pool.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x1   :: The x-coordinate of the segment's start point.             */
  /*                                                                       */
  /*    y1   :: The y-coordinate of the segment's start point.             */
  /*                                                                       */
  /*    x2   :: The x-coordinate of the segment's end point.               */
  /*                                                                       */
  /*    y2   :: The y-coordinate of the segment's end point.               */
  /*                                                                       */
  /*    miny :: A lower vertical clipping bound value.                     */
  /*                                                                       */
  /*    maxy :: An upper vertical clipping bound value.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE on render pool overflow.               */
  /*                                                                       */
  static Bool
  Line_Down( RAS_ARGS Long  x1,
                      Long  y1,
                      Long  x2,
                      Long  y2,
                      Long  miny,
                      Long  maxy )
  {
    Bool  result, fresh;


    fresh  = ras.fresh;

    result = Line_Up( RAS_VARS x1, -y1, x2, -y2, -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cProfile->start = -ras.cProfile->start;

    return result;
  }


  /* A function type describing the functions used to split Bezier arcs */
  typedef void  (*TSplitter)( TPoint*  base );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Bezier_Up                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Compute the x-coordinates of an ascending Bezier arc and store     */
  /*    them in the render pool.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    degree   :: The degree of the Bezier arc (either 2 or 3).          */
  /*                                                                       */
  /*    splitter :: The function to split Bezier arcs.                     */
  /*                                                                       */
  /*    miny     :: A lower vertical clipping bound value.                 */
  /*                                                                       */
  /*    maxy     :: An upper vertical clipping bound value.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE on render pool overflow.               */
  /*                                                                       */
  static Bool
  Bezier_Up( RAS_ARGS Int        degree,
                      TSplitter  splitter,
                      Long       miny,
                      Long       maxy )
  {
    Long   y1, y2, e, e2, e0;
    Short  f1;

    TPoint*  arc;
    TPoint*  start_arc;

    PLong top;


    arc = ras.arc;
    y1  = arc[degree].y;
    y2  = arc[0].y;
    top = ras.top;

    if ( y2 < miny || y1 > maxy )
      goto Fin;

    e2 = FLOOR( y2 );

    if ( e2 > maxy )
      e2 = maxy;

    e0 = miny;

    if ( y1 < miny )
      e = miny;
    else
    {
      e  = CEILING( y1 );
      f1 = (Short)( FRAC( y1 ) );
      e0 = e;

      if ( f1 == 0 )
      {
        if ( ras.joint )
        {
          top--;
          ras.joint = FALSE;
        }

        *top++ = arc[degree].x;

        e += ras.precision;
      }
    }

    if ( ras.fresh )
    {
      ras.cProfile->start = TRUNC( e0 );
      ras.fresh = FALSE;
    }

    if ( e2 < e )
      goto Fin;

    if ( ( top + TRUNC( e2 - e ) + 1 ) >= ras.maxBuff )
    {
      ras.top   = top;
      ras.error = Raster_Err_Overflow;
      return FAILURE;
    }

    start_arc = arc;

    while ( arc >= start_arc && e <= e2 )
    {
      ras.joint = FALSE;

      y2 = arc[0].y;

      if ( y2 > e )
      {
        y1 = arc[degree].y;
        if ( y2 - y1 >= ras.precision_step )
        {
          splitter( arc );
          arc += degree;
        }
        else
        {
          *top++ = arc[degree].x + FMulDiv( arc[0].x - arc[degree].x,
                                            e - y1, y2 - y1 );
          arc -= degree;
          e   += ras.precision;
        }
      }
      else
      {
        if ( y2 == e )
        {
          ras.joint  = TRUE;
          *top++     = arc[0].x;

          e += ras.precision;
        }
        arc -= degree;
      }
    }

  Fin:
    ras.top  = top;
    ras.arc -= degree;
    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Bezier_Down                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Compute the x-coordinates of an descending Bezier arc and store    */
  /*    them in the render pool.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    degree   :: The degree of the Bezier arc (either 2 or 3).          */
  /*                                                                       */
  /*    splitter :: The function to split Bezier arcs.                     */
  /*                                                                       */
  /*    miny     :: A lower vertical clipping bound value.                 */
  /*                                                                       */
  /*    maxy     :: An upper vertical clipping bound value.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE on render pool overflow.               */
  /*                                                                       */
  static Bool
  Bezier_Down( RAS_ARGS Int        degree,
                        TSplitter  splitter,
                        Long       miny,
                        Long       maxy )
  {
    TPoint*  arc = ras.arc;
    Bool     result, fresh;


    arc[0].y = -arc[0].y;
    arc[1].y = -arc[1].y;
    arc[2].y = -arc[2].y;
    if ( degree > 2 )
      arc[3].y = -arc[3].y;

    fresh = ras.fresh;

    result = Bezier_Up( RAS_VARS degree, splitter, -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cProfile->start = -ras.cProfile->start;

    arc[0].y = -arc[0].y;
    return result;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_To                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Inject a new line segment and adjust the Profiles list.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*   x :: The x-coordinate of the segment's end point (its start point   */
  /*        is stored in `lastX').                                         */
  /*                                                                       */
  /*   y :: The y-coordinate of the segment's end point (its start point   */
  /*        is stored in `lastY').                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*   SUCCESS on success, FAILURE on render pool overflow or incorrect    */
  /*   profile.                                                            */
  /*                                                                       */
  static Bool
  Line_To( RAS_ARGS Long  x,
                    Long  y )
  {
    /* First, detect a change of direction */

    switch ( ras.state )
    {
    case Unknown_State:
      if ( y > ras.lastY )
      {
        if ( New_Profile( RAS_VARS Ascending_State,
                                   IS_BOTTOM_OVERSHOOT( ras.lastY ) ) )
          return FAILURE;
      }
      else
      {
        if ( y < ras.lastY )
          if ( New_Profile( RAS_VARS Descending_State,
                                     IS_TOP_OVERSHOOT( ras.lastY ) ) )
            return FAILURE;
      }
      break;

    case Ascending_State:
      if ( y < ras.lastY )
      {
        if ( End_Profile( RAS_VARS IS_TOP_OVERSHOOT( ras.lastY ) ) ||
             New_Profile( RAS_VARS Descending_State,
                                   IS_TOP_OVERSHOOT( ras.lastY ) ) )
          return FAILURE;
      }
      break;

    case Descending_State:
      if ( y > ras.lastY )
      {
        if ( End_Profile( RAS_VARS IS_BOTTOM_OVERSHOOT( ras.lastY ) ) ||
             New_Profile( RAS_VARS Ascending_State,
                                   IS_BOTTOM_OVERSHOOT( ras.lastY ) ) )
          return FAILURE;
      }
      break;

    default:
      ;
    }

    /* Then compute the lines */

    switch ( ras.state )
    {
    case Ascending_State:
      if ( Line_Up( RAS_VARS ras.lastX, ras.lastY,
                             x, y, ras.minY, ras.maxY ) )
        return FAILURE;
      break;

    case Descending_State:
      if ( Line_Down( RAS_VARS ras.lastX, ras.lastY,
                               x, y, ras.minY, ras.maxY ) )
        return FAILURE;
      break;

    default:
      ;
    }

    ras.lastX = x;
    ras.lastY = y;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Conic_To                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Inject a new conic arc and adjust the profile list.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*   cx :: The x-coordinate of the arc's new control point.              */
  /*                                                                       */
  /*   cy :: The y-coordinate of the arc's new control point.              */
  /*                                                                       */
  /*   x  :: The x-coordinate of the arc's end point (its start point is   */
  /*         stored in `lastX').                                           */
  /*                                                                       */
  /*   y  :: The y-coordinate of the arc's end point (its start point is   */
  /*         stored in `lastY').                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*   SUCCESS on success, FAILURE on render pool overflow or incorrect    */
  /*   profile.                                                            */
  /*                                                                       */
  static Bool
  Conic_To( RAS_ARGS Long  cx,
                     Long  cy,
                     Long  x,
                     Long  y )
  {
    Long     y1, y2, y3, x3, ymin, ymax;
    TStates  state_bez;


    ras.arc      = ras.arcs;
    ras.arc[2].x = ras.lastX;
    ras.arc[2].y = ras.lastY;
    ras.arc[1].x = cx;
    ras.arc[1].y = cy;
    ras.arc[0].x = x;
    ras.arc[0].y = y;

    do
    {
      y1 = ras.arc[2].y;
      y2 = ras.arc[1].y;
      y3 = ras.arc[0].y;
      x3 = ras.arc[0].x;

      /* first, categorize the Bezier arc */

      if ( y1 <= y3 )
      {
        ymin = y1;
        ymax = y3;
      }
      else
      {
        ymin = y3;
        ymax = y1;
      }

      if ( y2 < ymin || y2 > ymax )
      {
        /* this arc has no given direction, split it! */
        Split_Conic( ras.arc );
        ras.arc += 2;
      }
      else if ( y1 == y3 )
      {
        /* this arc is flat, ignore it and pop it from the Bezier stack */
        ras.arc -= 2;
      }
      else
      {
        /* the arc is y-monotonous, either ascending or descending */
        /* detect a change of direction                            */
        state_bez = y1 < y3 ? Ascending_State : Descending_State;
        if ( ras.state != state_bez )
        {
          Bool  o = state_bez == Ascending_State ? IS_BOTTOM_OVERSHOOT( y1 )
                                                 : IS_TOP_OVERSHOOT( y1 );


          /* finalize current profile if any */
          if ( ras.state != Unknown_State &&
               End_Profile( RAS_VARS o )  )
            goto Fail;

          /* create a new profile */
          if ( New_Profile( RAS_VARS state_bez, o ) )
            goto Fail;
        }

        /* now call the appropriate routine */
        if ( state_bez == Ascending_State )
        {
          if ( Bezier_Up( RAS_VARS 2, Split_Conic, ras.minY, ras.maxY ) )
            goto Fail;
        }
        else
          if ( Bezier_Down( RAS_VARS 2, Split_Conic, ras.minY, ras.maxY ) )
            goto Fail;
      }

    } while ( ras.arc >= ras.arcs );

    ras.lastX = x3;
    ras.lastY = y3;

    return SUCCESS;

  Fail:
    return FAILURE;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Cubic_To                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Inject a new cubic arc and adjust the profile list.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*   cx1 :: The x-coordinate of the arc's first new control point.       */
  /*                                                                       */
  /*   cy1 :: The y-coordinate of the arc's first new control point.       */
  /*                                                                       */
  /*   cx2 :: The x-coordinate of the arc's second new control point.      */
  /*                                                                       */
  /*   cy2 :: The y-coordinate of the arc's second new control point.      */
  /*                                                                       */
  /*   x   :: The x-coordinate of the arc's end point (its start point is  */
  /*          stored in `lastX').                                          */
  /*                                                                       */
  /*   y   :: The y-coordinate of the arc's end point (its start point is  */
  /*          stored in `lastY').                                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*   SUCCESS on success, FAILURE on render pool overflow or incorrect    */
  /*   profile.                                                            */
  /*                                                                       */
  static Bool
  Cubic_To( RAS_ARGS Long  cx1,
                     Long  cy1,
                     Long  cx2,
                     Long  cy2,
                     Long  x,
                     Long  y )
  {
    Long     y1, y2, y3, y4, x4, ymin1, ymax1, ymin2, ymax2;
    TStates  state_bez;


    ras.arc      = ras.arcs;
    ras.arc[3].x = ras.lastX;
    ras.arc[3].y = ras.lastY;
    ras.arc[2].x = cx1;
    ras.arc[2].y = cy1;
    ras.arc[1].x = cx2;
    ras.arc[1].y = cy2;
    ras.arc[0].x = x;
    ras.arc[0].y = y;

    do
    {
      y1 = ras.arc[3].y;
      y2 = ras.arc[2].y;
      y3 = ras.arc[1].y;
      y4 = ras.arc[0].y;
      x4 = ras.arc[0].x;

      /* first, categorize the Bezier arc */

      if ( y1 <= y4 )
      {
        ymin1 = y1;
        ymax1 = y4;
      }
      else
      {
        ymin1 = y4;
        ymax1 = y1;
      }

      if ( y2 <= y3 )
      {
        ymin2 = y2;
        ymax2 = y3;
      }
      else
      {
        ymin2 = y3;
        ymax2 = y2;
      }

      if ( ymin2 < ymin1 || ymax2 > ymax1 )
      {
        /* this arc has no given direction, split it! */
        Split_Cubic( ras.arc );
        ras.arc += 3;
      }
      else if ( y1 == y4 )
      {
        /* this arc is flat, ignore it and pop it from the Bezier stack */
        ras.arc -= 3;
      }
      else
      {
        state_bez = ( y1 <= y4 ) ? Ascending_State : Descending_State;

        /* detect a change of direction */
        if ( ras.state != state_bez )
        {
          Bool  o = state_bez == Ascending_State ? IS_BOTTOM_OVERSHOOT( y1 )
                                                 : IS_TOP_OVERSHOOT( y1 );


          /* finalize current profile if any */
          if ( ras.state != Unknown_State &&
               End_Profile( RAS_VARS o )  )
            goto Fail;

          if ( New_Profile( RAS_VARS state_bez, o ) )
            goto Fail;
        }

        /* compute intersections */
        if ( state_bez == Ascending_State )
        {
          if ( Bezier_Up( RAS_VARS 3, Split_Cubic, ras.minY, ras.maxY ) )
            goto Fail;
        }
        else
          if ( Bezier_Down( RAS_VARS 3, Split_Cubic, ras.minY, ras.maxY ) )
            goto Fail;
      }

    } while ( ras.arc >= ras.arcs );

    ras.lastX = x4;
    ras.lastY = y4;

    return SUCCESS;

  Fail:
    return FAILURE;
  }


#undef  SWAP_
#define SWAP_( x, y )  do                \
                       {                 \
                         Long  swap = x; \
                                         \
                                         \
                         x = y;          \
                         y = swap;       \
                       } while ( 0 )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Decompose_Curve                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Scan the outline arrays in order to emit individual segments and   */
  /*    Beziers by calling Line_To() and Bezier_To().  It handles all      */
  /*    weird cases, like when the first point is off the curve, or when   */
  /*    there are simply no `on' points in the contour!                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    first   :: The index of the first point in the contour.            */
  /*                                                                       */
  /*    last    :: The index of the last point in the contour.             */
  /*                                                                       */
  /*    flipped :: If set, flip the direction of the curve.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE on error.                              */
  /*                                                                       */
  static Bool
  Decompose_Curve( RAS_ARGS UShort  first,
                            UShort  last,
                            int     flipped )
  {
    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  points;
    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    unsigned    tag;       /* current point's state           */


    points = ras.outline.points;
    limit  = points + last;

    v_start.x = SCALED( points[first].x );
    v_start.y = SCALED( points[first].y );
    v_last.x  = SCALED( points[last].x );
    v_last.y  = SCALED( points[last].y );

    if ( flipped )
    {
      SWAP_( v_start.x, v_start.y );
      SWAP_( v_last.x, v_last.y );
    }

    v_control = v_start;

    point = points + first;
    tags  = ras.outline.tags + first;

    /* set scan mode if necessary */
    if ( tags[0] & FT_CURVE_TAG_HAS_SCANMODE )
      ras.dropOutControl = (Byte)tags[0] >> 5;

    tag = FT_CURVE_TAG( tags[0] );

    /* A contour cannot start with a cubic control point! */
    if ( tag == FT_CURVE_TAG_CUBIC )
      goto Invalid_Outline;

    /* check first point to determine origin */
    if ( tag == FT_CURVE_TAG_CONIC )
    {
      /* first point is conic control.  Yes, this happens. */
      if ( FT_CURVE_TAG( ras.outline.tags[last] ) == FT_CURVE_TAG_ON )
      {
        /* start at last point if it is on the curve */
        v_start = v_last;
        limit--;
      }
      else
      {
        /* if both first and last points are conic,         */
        /* start at their middle and record its position    */
        /* for closure                                      */
        v_start.x = ( v_start.x + v_last.x ) / 2;
        v_start.y = ( v_start.y + v_last.y ) / 2;

        v_last = v_start;
      }
      point--;
      tags--;
    }

    ras.lastX = v_start.x;
    ras.lastY = v_start.y;

    while ( point < limit )
    {
      point++;
      tags++;

      tag = FT_CURVE_TAG( tags[0] );

      switch ( tag )
      {
      case FT_CURVE_TAG_ON:  /* emit a single line_to */
        {
          Long  x, y;


          x = SCALED( point->x );
          y = SCALED( point->y );
          if ( flipped )
            SWAP_( x, y );

          if ( Line_To( RAS_VARS x, y ) )
            goto Fail;
          continue;
        }

      case FT_CURVE_TAG_CONIC:  /* consume conic arcs */
        v_control.x = SCALED( point[0].x );
        v_control.y = SCALED( point[0].y );

        if ( flipped )
          SWAP_( v_control.x, v_control.y );

      Do_Conic:
        if ( point < limit )
        {
          FT_Vector  v_middle;
          Long       x, y;


          point++;
          tags++;
          tag = FT_CURVE_TAG( tags[0] );

          x = SCALED( point[0].x );
          y = SCALED( point[0].y );

          if ( flipped )
            SWAP_( x, y );

          if ( tag == FT_CURVE_TAG_ON )
          {
            if ( Conic_To( RAS_VARS v_control.x, v_control.y, x, y ) )
              goto Fail;
            continue;
          }

          if ( tag != FT_CURVE_TAG_CONIC )
            goto Invalid_Outline;

          v_middle.x = ( v_control.x + x ) / 2;
          v_middle.y = ( v_control.y + y ) / 2;

          if ( Conic_To( RAS_VARS v_control.x, v_control.y,
                                  v_middle.x,  v_middle.y ) )
            goto Fail;

          v_control.x = x;
          v_control.y = y;

          goto Do_Conic;
        }

        if ( Conic_To( RAS_VARS v_control.x, v_control.y,
                                v_start.x,   v_start.y ) )
          goto Fail;

        goto Close;

      default:  /* FT_CURVE_TAG_CUBIC */
        {
          Long  x1, y1, x2, y2, x3, y3;


          if ( point + 1 > limit                             ||
               FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
            goto Invalid_Outline;

          point += 2;
          tags  += 2;

          x1 = SCALED( point[-2].x );
          y1 = SCALED( point[-2].y );
          x2 = SCALED( point[-1].x );
          y2 = SCALED( point[-1].y );

          if ( flipped )
          {
            SWAP_( x1, y1 );
            SWAP_( x2, y2 );
          }

          if ( point <= limit )
          {
            x3 = SCALED( point[0].x );
            y3 = SCALED( point[0].y );

            if ( flipped )
              SWAP_( x3, y3 );

            if ( Cubic_To( RAS_VARS x1, y1, x2, y2, x3, y3 ) )
              goto Fail;
            continue;
          }

          if ( Cubic_To( RAS_VARS x1, y1, x2, y2, v_start.x, v_start.y ) )
            goto Fail;
          goto Close;
        }
      }
    }

    /* close the contour with a line segment */
    if ( Line_To( RAS_VARS v_start.x, v_start.y ) )
      goto Fail;

  Close:
    return SUCCESS;

  Invalid_Outline:
    ras.error = Raster_Err_Invalid;

  Fail:
    return FAILURE;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Convert_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Convert a glyph into a series of segments and arcs and make a      */
  /*    profiles list with them.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    flipped :: If set, flip the direction of curve.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS on success, FAILURE if any error was encountered during    */
  /*    rendering.                                                         */
  /*                                                                       */
  static Bool
  Convert_Glyph( RAS_ARGS int  flipped )
  {
    int       i;
    unsigned  start;

    PProfile  lastProfile;


    ras.fProfile = NULL;
    ras.joint    = FALSE;
    ras.fresh    = FALSE;

    ras.maxBuff  = ras.sizeBuff - AlignProfileSize;

    ras.numTurns = 0;

    ras.cProfile         = (PProfile)ras.top;
    ras.cProfile->offset = ras.top;
    ras.num_Profs        = 0;

    start = 0;

    for ( i = 0; i < ras.outline.n_contours; i++ )
    {
      Bool  o;


      ras.state    = Unknown_State;
      ras.gProfile = NULL;

      if ( Decompose_Curve( RAS_VARS (unsigned short)start,
                                     ras.outline.contours[i],
                                     flipped ) )
        return FAILURE;

      start = ras.outline.contours[i] + 1;

      /* we must now check whether the extreme arcs join or not */
      if ( FRAC( ras.lastY ) == 0 &&
           ras.lastY >= ras.minY  &&
           ras.lastY <= ras.maxY  )
        if ( ras.gProfile                        &&
             ( ras.gProfile->flags & Flow_Up ) ==
               ( ras.cProfile->flags & Flow_Up ) )
          ras.top--;
        /* Note that ras.gProfile can be nil if the contour was too small */
        /* to be drawn.                                                   */

      lastProfile = ras.cProfile;
      if ( ras.cProfile->flags & Flow_Up )
        o = IS_TOP_OVERSHOOT( ras.lastY );
      else
        o = IS_BOTTOM_OVERSHOOT( ras.lastY );
      if ( End_Profile( RAS_VARS o ) )
        return FAILURE;

      /* close the `next profile in contour' linked list */
      if ( ras.gProfile )
        lastProfile->next = ras.gProfile;
    }

    if ( Finalize_Profile_Table( RAS_VAR ) )
      return FAILURE;

    return (Bool)( ras.top < ras.maxBuff ? SUCCESS : FAILURE );
  }


  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  SCAN-LINE SWEEPS AND DRAWING                                       **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*  Init_Linked                                                          */
  /*                                                                       */
  /*    Initializes an empty linked list.                                  */
  /*                                                                       */
  static void
  Init_Linked( TProfileList*  l )
  {
    *l = NULL;
  }


  /*************************************************************************/
  /*                                                                       */
  /*  InsNew                                                               */
  /*                                                                       */
  /*    Inserts a new profile in a linked list.                            */
  /*                                                                       */
  static void
  InsNew( PProfileList  list,
          PProfile      profile )
  {
    PProfile  *old, current;
    Long       x;


    old     = list;
    current = *old;
    x       = profile->X;

    while ( current )
    {
      if ( x < current->X )
        break;
      old     = &current->link;
      current = *old;
    }

    profile->link = current;
    *old          = profile;
  }


  /*************************************************************************/
  /*                                                                       */
  /*  DelOld                                                               */
  /*                                                                       */
  /*    Removes an old profile from a linked list.                         */
  /*                                                                       */
  static void
  DelOld( PProfileList  list,
          PProfile      profile )
  {
    PProfile  *old, current;


    old     = list;
    current = *old;

    while ( current )
    {
      if ( current == profile )
      {
        *old = current->link;
        return;
      }

      old     = &current->link;
      current = *old;
    }

    /* we should never get there, unless the profile was not part of */
    /* the list.                                                     */
  }


  /*************************************************************************/
  /*                                                                       */
  /*  Sort                                                                 */
  /*                                                                       */
  /*    Sorts a trace list.  In 95%, the list is already sorted.  We need  */
  /*    an algorithm which is fast in this case.  Bubble sort is enough    */
  /*    and simple.                                                        */
  /*                                                                       */
  static void
  Sort( PProfileList  list )
  {
    PProfile  *old, current, next;


    /* First, set the new X coordinate of each profile */
    current = *list;
    while ( current )
    {
      current->X       = *current->offset;
      current->offset += current->flags & Flow_Up ? 1 : -1;
      current->height--;
      current = current->link;
    }

    /* Then sort them */
    old     = list;
    current = *old;

    if ( !current )
      return;

    next = current->link;

    while ( next )
    {
      if ( current->X <= next->X )
      {
        old     = &current->link;
        current = *old;

        if ( !current )
          return;
      }
      else
      {
        *old          = next;
        current->link = next->link;
        next->link    = current;

        old     = list;
        current = *old;
      }

      next = current->link;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /*  Vertical Sweep Procedure Set                                         */
  /*                                                                       */
  /*  These four routines are used during the vertical black/white sweep   */
  /*  phase by the generic Draw_Sweep() function.                          */
  /*                                                                       */
  /*************************************************************************/

  static void
  Vertical_Sweep_Init( RAS_ARGS Short*  min,
                                Short*  max )
  {
    Long  pitch = ras.target.pitch;

    FT_UNUSED( max );


    ras.traceIncr = (Short)-pitch;
    ras.traceOfs  = -*min * pitch;
    if ( pitch > 0 )
      ras.traceOfs += ( ras.target.rows - 1 ) * pitch;

    ras.gray_min_x = 0;
    ras.gray_max_x = 0;
  }


  static void
  Vertical_Sweep_Span( RAS_ARGS Short       y,
                                FT_F26Dot6  x1,
                                FT_F26Dot6  x2,
                                PProfile    left,
                                PProfile    right )
  {
    Long   e1, e2;
    int    c1, c2;
    Byte   f1, f2;
    Byte*  target;

    FT_UNUSED( y );
    FT_UNUSED( left );
    FT_UNUSED( right );


    /* Drop-out control */

    e1 = TRUNC( CEILING( x1 ) );

    if ( x2 - x1 - ras.precision <= ras.precision_jitter )
      e2 = e1;
    else
      e2 = TRUNC( FLOOR( x2 ) );

    if ( e2 >= 0 && e1 < ras.bWidth )
    {
      if ( e1 < 0 )
        e1 = 0;
      if ( e2 >= ras.bWidth )
        e2 = ras.bWidth - 1;

      c1 = (Short)( e1 >> 3 );
      c2 = (Short)( e2 >> 3 );

      f1 = (Byte)  ( 0xFF >> ( e1 & 7 ) );
      f2 = (Byte) ~( 0x7F >> ( e2 & 7 ) );

      if ( ras.gray_min_x > c1 )
        ras.gray_min_x = (short)c1;
      if ( ras.gray_max_x < c2 )
        ras.gray_max_x = (short)c2;

      target = ras.bTarget + ras.traceOfs + c1;
      c2 -= c1;

      if ( c2 > 0 )
      {
        target[0] |= f1;

        /* memset() is slower than the following code on many platforms. */
        /* This is due to the fact that, in the vast majority of cases,  */
        /* the span length in bytes is relatively small.                 */
        c2--;
        while ( c2 > 0 )
        {
          *(++target) = 0xFF;
          c2--;
        }
        target[1] |= f2;
      }
      else
        *target |= ( f1 & f2 );
    }
  }


  static void
  Vertical_Sweep_Drop( RAS_ARGS Short       y,
                                FT_F26Dot6  x1,
                                FT_F26Dot6  x2,
                                PProfile    left,
                                PProfile    right )
  {
    Long   e1, e2, pxl;
    Short  c1, f1;


    /* Drop-out control */

    /*   e2            x2                    x1           e1   */
    /*                                                         */
    /*                 ^                     |                 */
    /*                 |                     |                 */
    /*   +-------------+---------------------+------------+    */
    /*                 |                     |                 */
    /*                 |                     v                 */
    /*                                                         */
    /* pixel         contour              contour       pixel  */
    /* center                                           center */

    /* drop-out mode    scan conversion rules (as defined in OpenType) */
    /* --------------------------------------------------------------- */
    /*  0                1, 2, 3                                       */
    /*  1                1, 2, 4                                       */
    /*  2                1, 2                                          */
    /*  3                same as mode 2                                */
    /*  4                1, 2, 5                                       */
    /*  5                1, 2, 6                                       */
    /*  6, 7             same as mode 2                                */

    e1  = CEILING( x1 );
    e2  = FLOOR  ( x2 );
    pxl = e1;

    if ( e1 > e2 )
    {
      Int  dropOutControl = left->flags & 7;


      if ( e1 == e2 + ras.precision )
      {
        switch ( dropOutControl )
        {
        case 0: /* simple drop-outs including stubs */
          pxl = e2;
          break;

        case 4: /* smart drop-outs including stubs */
          pxl = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );
          break;

        case 1: /* simple drop-outs excluding stubs */
        case 5: /* smart drop-outs excluding stubs  */

          /* Drop-out Control Rules #4 and #6 */

          /* The specification neither provides an exact definition */
          /* of a `stub' nor gives exact rules to exclude them.     */
          /*                                                        */
          /* Here the constraints we use to recognize a stub.       */
          /*                                                        */
          /*  upper stub:                                           */
          /*                                                        */
          /*   - P_Left and P_Right are in the same contour         */
          /*   - P_Right is the successor of P_Left in that contour */
          /*   - y is the top of P_Left and P_Right                 */
          /*                                                        */
          /*  lower stub:                                           */
          /*                                                        */
          /*   - P_Left and P_Right are in the same contour         */
          /*   - P_Left is the successor of P_Right in that contour */
          /*   - y is the bottom of P_Left                          */
          /*                                                        */
          /* We draw a stub if the following constraints are met.   */
          /*                                                        */
          /*   - for an upper or lower stub, there is top or bottom */
          /*     overshoot, respectively                            */
          /*   - the covered interval is greater or equal to a half */
          /*     pixel                                              */

          /* upper stub test */
          if ( left->next == right                &&
               left->height <= 0                  &&
               !( left->flags & Overshoot_Top   &&
                  x2 - x1 >= ras.precision_half ) )
            return;

          /* lower stub test */
          if ( right->next == left                 &&
               left->start == y                    &&
               !( left->flags & Overshoot_Bottom &&
                  x2 - x1 >= ras.precision_half  ) )
            return;

          if ( dropOutControl == 1 )
            pxl = e2;
          else
            pxl = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );
          break;

        default: /* modes 2, 3, 6, 7 */
          return;  /* no drop-out control */
        }

        /* check that the other pixel isn't set */
        e1 = pxl == e1 ? e2 : e1;

        e1 = TRUNC( e1 );

        c1 = (Short)( e1 >> 3 );
        f1 = (Short)( e1 &  7 );

        if ( e1 >= 0 && e1 < ras.bWidth                      &&
             ras.bTarget[ras.traceOfs + c1] & ( 0x80 >> f1 ) )
          return;
      }
      else
        return;
    }

    e1 = TRUNC( pxl );

    if ( e1 >= 0 && e1 < ras.bWidth )
    {
      c1 = (Short)( e1 >> 3 );
      f1 = (Short)( e1 & 7 );

      if ( ras.gray_min_x > c1 )
        ras.gray_min_x = c1;
      if ( ras.gray_max_x < c1 )
        ras.gray_max_x = c1;

      ras.bTarget[ras.traceOfs + c1] |= (char)( 0x80 >> f1 );
    }
  }


  static void
  Vertical_Sweep_Step( RAS_ARG )
  {
    ras.traceOfs += ras.traceIncr;
  }


  /***********************************************************************/
  /*                                                                     */
  /*  Horizontal Sweep Procedure Set                                     */
  /*                                                                     */
  /*  These four routines are used during the horizontal black/white     */
  /*  sweep phase by the generic Draw_Sweep() function.                  */
  /*                                                                     */
  /***********************************************************************/

  static void
  Horizontal_Sweep_Init( RAS_ARGS Short*  min,
                                  Short*  max )
  {
    /* nothing, really */
    FT_UNUSED_RASTER;
    FT_UNUSED( min );
    FT_UNUSED( max );
  }


  static void
  Horizontal_Sweep_Span( RAS_ARGS Short       y,
                                  FT_F26Dot6  x1,
                                  FT_F26Dot6  x2,
                                  PProfile    left,
                                  PProfile    right )
  {
    Long   e1, e2;
    PByte  bits;
    Byte   f1;

    FT_UNUSED( left );
    FT_UNUSED( right );


    if ( x2 - x1 < ras.precision )
    {
      e1 = CEILING( x1 );
      e2 = FLOOR  ( x2 );

      if ( e1 == e2 )
      {
        bits = ras.bTarget + ( y >> 3 );
        f1   = (Byte)( 0x80 >> ( y & 7 ) );

        e1 = TRUNC( e1 );

        if ( e1 >= 0 && e1 < ras.target.rows )
        {
          PByte  p;


          p = bits - e1 * ras.target.pitch;
          if ( ras.target.pitch > 0 )
            p += ( ras.target.rows - 1 ) * ras.target.pitch;

          p[0] |= f1;
        }
      }
    }
  }


  static void
  Horizontal_Sweep_Drop( RAS_ARGS Short       y,
                                  FT_F26Dot6  x1,
                                  FT_F26Dot6  x2,
                                  PProfile    left,
                                  PProfile    right )
  {
    Long   e1, e2, pxl;
    PByte  bits;
    Byte   f1;


    /* During the horizontal sweep, we only take care of drop-outs */

    /* e1     +       <-- pixel center */
    /*        |                        */
    /* x1  ---+-->    <-- contour      */
    /*        |                        */
    /*        |                        */
    /* x2  <--+---    <-- contour      */
    /*        |                        */
    /*        |                        */
    /* e2     +       <-- pixel center */

    e1  = CEILING( x1 );
    e2  = FLOOR  ( x2 );
    pxl = e1;

    if ( e1 > e2 )
    {
      Int  dropOutControl = left->flags & 7;


      if ( e1 == e2 + ras.precision )
      {
        switch ( dropOutControl )
        {
        case 0: /* simple drop-outs including stubs */
          pxl = e2;
          break;

        case 4: /* smart drop-outs including stubs */
          pxl = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );
          break;

        case 1: /* simple drop-outs excluding stubs */
        case 5: /* smart drop-outs excluding stubs  */
          /* see Vertical_Sweep_Drop for details */

          /* rightmost stub test */
          if ( left->next == right                &&
               left->height <= 0                  &&
               !( left->flags & Overshoot_Top   &&
                  x2 - x1 >= ras.precision_half ) )
            return;

          /* leftmost stub test */
          if ( right->next == left                 &&
               left->start == y                    &&
               !( left->flags & Overshoot_Bottom &&
                  x2 - x1 >= ras.precision_half  ) )
            return;

          if ( dropOutControl == 1 )
            pxl = e2;
          else
            pxl = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );
          break;

        default: /* modes 2, 3, 6, 7 */
          return;  /* no drop-out control */
        }

        /* check that the other pixel isn't set */
        e1 = pxl == e1 ? e2 : e1;

        e1 = TRUNC( e1 );

        bits = ras.bTarget + ( y >> 3 );
        f1   = (Byte)( 0x80 >> ( y & 7 ) );

        bits -= e1 * ras.target.pitch;
        if ( ras.target.pitch > 0 )
          bits += ( ras.target.rows - 1 ) * ras.target.pitch;

        if ( e1 >= 0              &&
             e1 < ras.target.rows &&
             *bits & f1           )
          return;
      }
      else
        return;
    }

    bits = ras.bTarget + ( y >> 3 );
    f1   = (Byte)( 0x80 >> ( y & 7 ) );

    e1 = TRUNC( pxl );

    if ( e1 >= 0 && e1 < ras.target.rows )
    {
      bits -= e1 * ras.target.pitch;
      if ( ras.target.pitch > 0 )
        bits += ( ras.target.rows - 1 ) * ras.target.pitch;

      bits[0] |= f1;
    }
  }


  static void
  Horizontal_Sweep_Step( RAS_ARG )
  {
    /* Nothing, really */
    FT_UNUSED_RASTER;
  }


#ifdef FT_RASTER_OPTION_ANTI_ALIASING


  /*************************************************************************/
  /*                                                                       */
  /*  Vertical Gray Sweep Procedure Set                                    */
  /*                                                                       */
  /*  These two routines are used during the vertical gray-levels sweep    */
  /*  phase by the generic Draw_Sweep() function.                          */
  /*                                                                       */
  /*  NOTES                                                                */
  /*                                                                       */
  /*  - The target pixmap's width *must* be a multiple of 4.               */
  /*                                                                       */
  /*  - You have to use the function Vertical_Sweep_Span() for the gray    */
  /*    span call.                                                         */
  /*                                                                       */
  /*************************************************************************/

  static void
  Vertical_Gray_Sweep_Init( RAS_ARGS Short*  min,
                                     Short*  max )
  {
    Long  pitch, byte_len;


    *min = *min & -2;
    *max = ( *max + 3 ) & -2;

    ras.traceOfs  = 0;
    pitch         = ras.target.pitch;
    byte_len      = -pitch;
    ras.traceIncr = (Short)byte_len;
    ras.traceG    = ( *min / 2 ) * byte_len;

    if ( pitch > 0 )
    {
      ras.traceG += ( ras.target.rows - 1 ) * pitch;
      byte_len    = -byte_len;
    }

    ras.gray_min_x =  (Short)byte_len;
    ras.gray_max_x = -(Short)byte_len;
  }


  static void
  Vertical_Gray_Sweep_Step( RAS_ARG )
  {
    Int     c1, c2;
    PByte   pix, bit, bit2;
    short*  count = (short*)count_table;
    Byte*   grays;


    ras.traceOfs += ras.gray_width;

    if ( ras.traceOfs > ras.gray_width )
    {
      pix   = ras.gTarget + ras.traceG + ras.gray_min_x * 4;
      grays = ras.grays;

      if ( ras.gray_max_x >= 0 )
      {
        Long  last_pixel = ras.target.width - 1;
        Int   last_cell  = last_pixel >> 2;
        Int   last_bit   = last_pixel & 3;
        Bool  over       = 0;


        if ( ras.gray_max_x >= last_cell && last_bit != 3 )
        {
          ras.gray_max_x = last_cell - 1;
          over = 1;
        }

        if ( ras.gray_min_x < 0 )
          ras.gray_min_x = 0;

        bit  = ras.bTarget + ras.gray_min_x;
        bit2 = bit + ras.gray_width;

        c1 = ras.gray_max_x - ras.gray_min_x;

        while ( c1 >= 0 )
        {
          c2 = count[*bit] + count[*bit2];

          if ( c2 )
          {
            pix[0] = grays[(c2 >> 12) & 0x000F];
            pix[1] = grays[(c2 >> 8 ) & 0x000F];
            pix[2] = grays[(c2 >> 4 ) & 0x000F];
            pix[3] = grays[ c2        & 0x000F];

            *bit  = 0;
            *bit2 = 0;
          }

          bit++;
          bit2++;
          pix += 4;
          c1--;
        }

        if ( over )
        {
          c2 = count[*bit] + count[*bit2];
          if ( c2 )
          {
            switch ( last_bit )
            {
            case 2:
              pix[2] = grays[(c2 >> 4 ) & 0x000F];
            case 1:
              pix[1] = grays[(c2 >> 8 ) & 0x000F];
            default:
              pix[0] = grays[(c2 >> 12) & 0x000F];
            }

            *bit  = 0;
            *bit2 = 0;
          }
        }
      }

      ras.traceOfs = 0;
      ras.traceG  += ras.traceIncr;

      ras.gray_min_x =  32000;
      ras.gray_max_x = -32000;
    }
  }


  static void
  Horizontal_Gray_Sweep_Span( RAS_ARGS Short       y,
                                       FT_F26Dot6  x1,
                                       FT_F26Dot6  x2,
                                       PProfile    left,
                                       PProfile    right )
  {
    /* nothing, really */
    FT_UNUSED_RASTER;
    FT_UNUSED( y );
    FT_UNUSED( x1 );
    FT_UNUSED( x2 );
    FT_UNUSED( left );
    FT_UNUSED( right );
  }


  static void
  Horizontal_Gray_Sweep_Drop( RAS_ARGS Short       y,
                                       FT_F26Dot6  x1,
                                       FT_F26Dot6  x2,
                                       PProfile    left,
                                       PProfile    right )
  {
    Long   e1, e2;
    PByte  pixel;
    Byte   color;


    /* During the horizontal sweep, we only take care of drop-outs */

    e1 = CEILING( x1 );
    e2 = FLOOR  ( x2 );

    if ( e1 > e2 )
    {
      Int  dropOutControl = left->flags & 7;


      if ( e1 == e2 + ras.precision )
      {
        switch ( dropOutControl )
        {
        case 0: /* simple drop-outs including stubs */
          e1 = e2;
          break;

        case 4: /* smart drop-outs including stubs */
          e1 = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );
          break;

        case 1: /* simple drop-outs excluding stubs */
        case 5: /* smart drop-outs excluding stubs  */
          /* see Vertical_Sweep_Drop for details */

          /* rightmost stub test */
          if ( left->next == right && left->height <= 0 )
            return;

          /* leftmost stub test */
          if ( right->next == left && left->start == y )
            return;

          if ( dropOutControl == 1 )
            e1 = e2;
          else
            e1 = FLOOR( ( x1 + x2 - 1 ) / 2 + ras.precision_half );

          break;

        default: /* modes 2, 3, 6, 7 */
          return;  /* no drop-out control */
        }
      }
      else
        return;
    }

    if ( e1 >= 0 )
    {
      if ( x2 - x1 >= ras.precision_half )
        color = ras.grays[2];
      else
        color = ras.grays[1];

      e1 = TRUNC( e1 ) / 2;
      if ( e1 < ras.target.rows )
      {
        pixel = ras.gTarget - e1 * ras.target.pitch + y / 2;
        if ( ras.target.pitch > 0 )
          pixel += ( ras.target.rows - 1 ) * ras.target.pitch;

        if ( pixel[0] == ras.grays[0] )
          pixel[0] = color;
      }
    }
  }


#endif /* FT_RASTER_OPTION_ANTI_ALIASING */


  /*************************************************************************/
  /*                                                                       */
  /*  Generic Sweep Drawing routine                                        */
  /*                                                                       */
  /*************************************************************************/

  static Bool
  Draw_Sweep( RAS_ARG )
  {
    Short         y, y_change, y_height;

    PProfile      P, Q, P_Left, P_Right;

    Short         min_Y, max_Y, top, bottom, dropouts;

    Long          x1, x2, xs, e1, e2;

    TProfileList  waiting;
    TProfileList  draw_left, draw_right;


    /* initialize empty linked lists */

    Init_Linked( &waiting );

    Init_Linked( &draw_left  );
    Init_Linked( &draw_right );

    /* first, compute min and max Y */

    P     = ras.fProfile;
    max_Y = (Short)TRUNC( ras.minY );
    min_Y = (Short)TRUNC( ras.maxY );

    while ( P )
    {
      Q = P->link;

      bottom = (Short)P->start;
      top    = (Short)( P->start + P->height - 1 );

      if ( min_Y > bottom )
        min_Y = bottom;
      if ( max_Y < top )
        max_Y = top;

      P->X = 0;
      InsNew( &waiting, P );

      P = Q;
    }

    /* check the Y-turns */
    if ( ras.numTurns == 0 )
    {
      ras.error = Raster_Err_Invalid;
      return FAILURE;
    }

    /* now initialize the sweep */

    ras.Proc_Sweep_Init( RAS_VARS &min_Y, &max_Y );

    /* then compute the distance of each profile from min_Y */

    P = waiting;

    while ( P )
    {
      P->countL = (UShort)( P->start - min_Y );
      P = P->link;
    }

    /* let's go */

    y        = min_Y;
    y_height = 0;

    if ( ras.numTurns > 0                     &&
         ras.sizeBuff[-ras.numTurns] == min_Y )
      ras.numTurns--;

    while ( ras.numTurns > 0 )
    {
      /* check waiting list for new activations */

      P = waiting;

      while ( P )
      {
        Q = P->link;
        P->countL -= y_height;
        if ( P->countL == 0 )
        {
          DelOld( &waiting, P );

          if ( P->flags & Flow_Up )
            InsNew( &draw_left,  P );
          else
            InsNew( &draw_right, P );
        }

        P = Q;
      }

      /* sort the drawing lists */

      Sort( &draw_left );
      Sort( &draw_right );

      y_change = (Short)ras.sizeBuff[-ras.numTurns--];
      y_height = (Short)( y_change - y );

      while ( y < y_change )
      {
        /* let's trace */

        dropouts = 0;

        P_Left  = draw_left;
        P_Right = draw_right;

        while ( P_Left )
        {
          x1 = P_Left ->X;
          x2 = P_Right->X;

          if ( x1 > x2 )
          {
            xs = x1;
            x1 = x2;
            x2 = xs;
          }

          e1 = FLOOR( x1 );
          e2 = CEILING( x2 );

          if ( x2 - x1 <= ras.precision &&
               e1 != x1 && e2 != x2     )
          {
            if ( e1 > e2 || e2 == e1 + ras.precision )
            {
              Int  dropOutControl = P_Left->flags & 7;


              if ( dropOutControl != 2 )
              {
                /* a drop-out was detected */

                P_Left ->X = x1;
                P_Right->X = x2;

                /* mark profile for drop-out processing */
                P_Left->countL = 1;
                dropouts++;
              }

              goto Skip_To_Next;
            }
          }

          ras.Proc_Sweep_Span( RAS_VARS y, x1, x2, P_Left, P_Right );

        Skip_To_Next:

          P_Left  = P_Left->link;
          P_Right = P_Right->link;
        }

        /* handle drop-outs _after_ the span drawing --       */
        /* drop-out processing has been moved out of the loop */
        /* for performance tuning                             */
        if ( dropouts > 0 )
          goto Scan_DropOuts;

      Next_Line:

        ras.Proc_Sweep_Step( RAS_VAR );

        y++;

        if ( y < y_change )
        {
          Sort( &draw_left  );
          Sort( &draw_right );
        }
      }

      /* now finalize the profiles that need it */

      P = draw_left;
      while ( P )
      {
        Q = P->link;
        if ( P->height == 0 )
          DelOld( &draw_left, P );
        P = Q;
      }

      P = draw_right;
      while ( P )
      {
        Q = P->link;
        if ( P->height == 0 )
          DelOld( &draw_right, P );
        P = Q;
      }
    }

    /* for gray-scaling, flush the bitmap scanline cache */
    while ( y <= max_Y )
    {
      ras.Proc_Sweep_Step( RAS_VAR );
      y++;
    }

    return SUCCESS;

  Scan_DropOuts:

    P_Left  = draw_left;
    P_Right = draw_right;

    while ( P_Left )
    {
      if ( P_Left->countL )
      {
        P_Left->countL = 0;
#if 0
        dropouts--;  /* -- this is useful when debugging only */
#endif
        ras.Proc_Sweep_Drop( RAS_VARS y,
                                      P_Left->X,
                                      P_Right->X,
                                      P_Left,
                                      P_Right );
      }

      P_Left  = P_Left->link;
      P_Right = P_Right->link;
    }

    goto Next_Line;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Render_Single_Pass                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Perform one sweep with sub-banding.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    flipped :: If set, flip the direction of the outline.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Renderer error code.                                               */
  /*                                                                       */
  static int
  Render_Single_Pass( RAS_ARGS Bool  flipped )
  {
    Short  i, j, k;


    while ( ras.band_top >= 0 )
    {
      ras.maxY = (Long)ras.band_stack[ras.band_top].y_max * ras.precision;
      ras.minY = (Long)ras.band_stack[ras.band_top].y_min * ras.precision;

      ras.top = ras.buff;

      ras.error = Raster_Err_None;

      if ( Convert_Glyph( RAS_VARS flipped ) )
      {
        if ( ras.error != Raster_Err_Overflow )
          return FAILURE;

        ras.error = Raster_Err_None;

        /* sub-banding */

#ifdef DEBUG_RASTER
        ClearBand( RAS_VARS TRUNC( ras.minY ), TRUNC( ras.maxY ) );
#endif

        i = ras.band_stack[ras.band_top].y_min;
        j = ras.band_stack[ras.band_top].y_max;

        k = (Short)( ( i + j ) / 2 );

        if ( ras.band_top >= 7 || k < i )
        {
          ras.band_top = 0;
          ras.error    = Raster_Err_Invalid;

          return ras.error;
        }

        ras.band_stack[ras.band_top + 1].y_min = k;
        ras.band_stack[ras.band_top + 1].y_max = j;

        ras.band_stack[ras.band_top].y_max = (Short)( k - 1 );

        ras.band_top++;
      }
      else
      {
        if ( ras.fProfile )
          if ( Draw_Sweep( RAS_VAR ) )
             return ras.error;
        ras.band_top--;
      }
    }

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Render_Glyph                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Render a glyph in a bitmap.  Sub-banding if needed.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  Render_Glyph( RAS_ARG )
  {
    FT_Error  error;


    Set_High_Precision( RAS_VARS ras.outline.flags &
                                 FT_OUTLINE_HIGH_PRECISION );
    ras.scale_shift = ras.precision_shift;

    if ( ras.outline.flags & FT_OUTLINE_IGNORE_DROPOUTS )
      ras.dropOutControl = 2;
    else
    {
      if ( ras.outline.flags & FT_OUTLINE_SMART_DROPOUTS )
        ras.dropOutControl = 4;
      else
        ras.dropOutControl = 0;

      if ( !( ras.outline.flags & FT_OUTLINE_INCLUDE_STUBS ) )
        ras.dropOutControl += 1;
    }

    ras.second_pass = (FT_Byte)( !( ras.outline.flags &
                                    FT_OUTLINE_SINGLE_PASS ) );

    /* Vertical Sweep */
    ras.Proc_Sweep_Init = Vertical_Sweep_Init;
    ras.Proc_Sweep_Span = Vertical_Sweep_Span;
    ras.Proc_Sweep_Drop = Vertical_Sweep_Drop;
    ras.Proc_Sweep_Step = Vertical_Sweep_Step;

    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = (short)( ras.target.rows - 1 );

    ras.bWidth  = (unsigned short)ras.target.width;
    ras.bTarget = (Byte*)ras.target.buffer;

    if ( ( error = Render_Single_Pass( RAS_VARS 0 ) ) != 0 )
      return error;

    /* Horizontal Sweep */
    if ( ras.second_pass && ras.dropOutControl != 2 )
    {
      ras.Proc_Sweep_Init = Horizontal_Sweep_Init;
      ras.Proc_Sweep_Span = Horizontal_Sweep_Span;
      ras.Proc_Sweep_Drop = Horizontal_Sweep_Drop;
      ras.Proc_Sweep_Step = Horizontal_Sweep_Step;

      ras.band_top            = 0;
      ras.band_stack[0].y_min = 0;
      ras.band_stack[0].y_max = (short)( ras.target.width - 1 );

      if ( ( error = Render_Single_Pass( RAS_VARS 1 ) ) != 0 )
        return error;
    }

    return Raster_Err_None;
  }


#ifdef FT_RASTER_OPTION_ANTI_ALIASING

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Render_Gray_Glyph                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Render a glyph with grayscaling.  Sub-banding if needed.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  Render_Gray_Glyph( RAS_ARG )
  {
    Long      pixel_width;
    FT_Error  error;


    Set_High_Precision( RAS_VARS ras.outline.flags &
                                 FT_OUTLINE_HIGH_PRECISION );
    ras.scale_shift = ras.precision_shift + 1;

    if ( ras.outline.flags & FT_OUTLINE_IGNORE_DROPOUTS )
      ras.dropOutControl = 2;
    else
    {
      if ( ras.outline.flags & FT_OUTLINE_SMART_DROPOUTS )
        ras.dropOutControl = 4;
      else
        ras.dropOutControl = 0;

      if ( !( ras.outline.flags & FT_OUTLINE_INCLUDE_STUBS ) )
        ras.dropOutControl += 1;
    }

    ras.second_pass = !( ras.outline.flags & FT_OUTLINE_SINGLE_PASS );

    /* Vertical Sweep */

    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = 2 * ras.target.rows - 1;

    ras.bWidth  = ras.gray_width;
    pixel_width = 2 * ( ( ras.target.width + 3 ) >> 2 );

    if ( ras.bWidth > pixel_width )
      ras.bWidth = pixel_width;

    ras.bWidth  = ras.bWidth * 8;
    ras.bTarget = (Byte*)ras.gray_lines;
    ras.gTarget = (Byte*)ras.target.buffer;

    ras.Proc_Sweep_Init = Vertical_Gray_Sweep_Init;
    ras.Proc_Sweep_Span = Vertical_Sweep_Span;
    ras.Proc_Sweep_Drop = Vertical_Sweep_Drop;
    ras.Proc_Sweep_Step = Vertical_Gray_Sweep_Step;

    error = Render_Single_Pass( RAS_VARS 0 );
    if ( error )
      return error;

    /* Horizontal Sweep */
    if ( ras.second_pass && ras.dropOutControl != 2 )
    {
      ras.Proc_Sweep_Init = Horizontal_Sweep_Init;
      ras.Proc_Sweep_Span = Horizontal_Gray_Sweep_Span;
      ras.Proc_Sweep_Drop = Horizontal_Gray_Sweep_Drop;
      ras.Proc_Sweep_Step = Horizontal_Sweep_Step;

      ras.band_top            = 0;
      ras.band_stack[0].y_min = 0;
      ras.band_stack[0].y_max = ras.target.width * 2 - 1;

      error = Render_Single_Pass( RAS_VARS 1 );
      if ( error )
        return error;
    }

    return Raster_Err_None;
  }

#else /* !FT_RASTER_OPTION_ANTI_ALIASING */

  FT_LOCAL_DEF( FT_Error )
  Render_Gray_Glyph( RAS_ARG )
  {
    FT_UNUSED_RASTER;

    return Raster_Err_Unsupported;
  }

#endif /* !FT_RASTER_OPTION_ANTI_ALIASING */


  static void
  ft_black_init( PRaster  raster )
  {
#ifdef FT_RASTER_OPTION_ANTI_ALIASING
    FT_UInt  n;


    /* set default 5-levels gray palette */
    for ( n = 0; n < 5; n++ )
      raster->grays[n] = n * 255 / 4;

    raster->gray_width = RASTER_GRAY_LINES / 2;
#else
    FT_UNUSED( raster );
#endif
  }


  /**** RASTER OBJECT CREATION: In standalone mode, we simply use *****/
  /****                         a static object.                  *****/


#ifdef _STANDALONE_


  static int
  ft_black_new( void*       memory,
                FT_Raster  *araster )
  {
     static TRaster  the_raster;
     FT_UNUSED( memory );


     *araster = (FT_Raster)&the_raster;
     FT_MEM_ZERO( &the_raster, sizeof ( the_raster ) );
     ft_black_init( &the_raster );

     return 0;
  }


  static void
  ft_black_done( FT_Raster  raster )
  {
    /* nothing */
    FT_UNUSED( raster );
  }


#else /* !_STANDALONE_ */


  static int
  ft_black_new( FT_Memory   memory,
                PRaster    *araster )
  {
    FT_Error  error;
    PRaster   raster = NULL;


    *araster = 0;
    if ( !FT_NEW( raster ) )
    {
      raster->memory = memory;
      ft_black_init( raster );

      *araster = raster;
    }

    return error;
  }


  static void
  ft_black_done( PRaster  raster )
  {
    FT_Memory  memory = (FT_Memory)raster->memory;
    FT_FREE( raster );
  }


#endif /* !_STANDALONE_ */


  static void
  ft_black_reset( PRaster  raster,
                  char*    pool_base,
                  long     pool_size )
  {
    if ( raster )
    {
      if ( pool_base && pool_size >= (long)sizeof(TWorker) + 2048 )
      {
        PWorker  worker = (PWorker)pool_base;


        raster->buffer      = pool_base + ( ( sizeof ( *worker ) + 7 ) & ~7 );
        raster->buffer_size = pool_base + pool_size - (char*)raster->buffer;
        raster->worker      = worker;
      }
      else
      {
        raster->buffer      = NULL;
        raster->buffer_size = 0;
        raster->worker      = NULL;
      }
    }
  }


  static int /* XXX EMSCRIPTEN */
  ft_black_set_mode( PRaster        raster,
                     unsigned long  mode,
                     const char*    palette )
  {
#ifdef FT_RASTER_OPTION_ANTI_ALIASING

    if ( mode == FT_MAKE_TAG( 'p', 'a', 'l', '5' ) )
    {
      /* set 5-levels gray palette */
      raster->grays[0] = palette[0];
      raster->grays[1] = palette[1];
      raster->grays[2] = palette[2];
      raster->grays[3] = palette[3];
      raster->grays[4] = palette[4];
    }

#else

    FT_UNUSED( raster );
    FT_UNUSED( mode );
    FT_UNUSED( palette );

#endif
	return 0; /* XXX EMSCRIPTEN */
  }


  static int
  ft_black_render( PRaster                  raster,
                   const FT_Raster_Params*  params )
  {
    const FT_Outline*  outline    = (const FT_Outline*)params->source;
    const FT_Bitmap*   target_map = params->target;
    PWorker            worker;


    if ( !raster || !raster->buffer || !raster->buffer_size )
      return Raster_Err_Not_Ini;

    if ( !outline )
      return Raster_Err_Invalid;

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return Raster_Err_None;

    if ( !outline->contours || !outline->points )
      return Raster_Err_Invalid;

    if ( outline->n_points !=
           outline->contours[outline->n_contours - 1] + 1 )
      return Raster_Err_Invalid;

    worker = raster->worker;

    /* this version of the raster does not support direct rendering, sorry */
    if ( params->flags & FT_RASTER_FLAG_DIRECT )
      return Raster_Err_Unsupported;

    if ( !target_map )
      return Raster_Err_Invalid;

    /* nothing to do */
    if ( !target_map->width || !target_map->rows )
      return Raster_Err_None;

    if ( !target_map->buffer )
      return Raster_Err_Invalid;

    ras.outline = *outline;
    ras.target  = *target_map;

    worker->buff       = (PLong) raster->buffer;
    worker->sizeBuff   = worker->buff +
                           raster->buffer_size / sizeof ( Long );
#ifdef FT_RASTER_OPTION_ANTI_ALIASING
    worker->grays      = raster->grays;
    worker->gray_width = raster->gray_width;

    FT_MEM_ZERO( worker->gray_lines, worker->gray_width * 2 );
#endif

    return ( params->flags & FT_RASTER_FLAG_AA )
           ? Render_Gray_Glyph( RAS_VAR )
           : Render_Glyph( RAS_VAR );
  }


  FT_DEFINE_RASTER_FUNCS( ft_standard_raster,
    FT_GLYPH_FORMAT_OUTLINE,
    (FT_Raster_New_Func)     ft_black_new,
    (FT_Raster_Reset_Func)   ft_black_reset,
    (FT_Raster_Set_Mode_Func)ft_black_set_mode,
    (FT_Raster_Render_Func)  ft_black_render,
    (FT_Raster_Done_Func)    ft_black_done
  )


/* END */
