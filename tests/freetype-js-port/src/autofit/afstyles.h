/***************************************************************************/
/*                                                                         */
/*  afstyles.h                                                             */
/*                                                                         */
/*    Auto-fitter styles (specification only).                             */
/*                                                                         */
/*  Copyright 2013-2016 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /* The following part can be included multiple times. */
  /* Define `STYLE' as needed.                          */


  /* Add new styles here.  The first and second arguments are the  */
  /* style name in lowercase and uppercase, respectively, followed */
  /* by a description string.  The next arguments are the          */
  /* corresponding writing system, script, blue stringset, and     */
  /* coverage.                                                     */
  /*                                                               */
  /* Note that styles using `AF_COVERAGE_DEFAULT' should always    */
  /* come after styles with other coverages.  Also note that       */
  /* fallback scripts only use `AF_COVERAGE_DEFAULT' for its       */
  /* style.                                                        */
  /*                                                               */
  /* Example:                                                      */
  /*                                                               */
  /*   STYLE( cyrl_dflt, CYRL_DFLT,                                */
  /*          "Cyrillic default style",                            */
  /*          AF_WRITING_SYSTEM_LATIN,                             */
  /*          AF_SCRIPT_CYRL,                                      */
  /*          AF_BLUE_STRINGSET_CYRL,                              */
  /*          AF_COVERAGE_DEFAULT )                                */

#undef  STYLE_LATIN
#define STYLE_LATIN( s, S, f, F, ds, df, C ) \
          STYLE( s ## _ ## f, S ## _ ## F,   \
                 ds " " df " style",         \
                 AF_WRITING_SYSTEM_LATIN,    \
                 AF_SCRIPT_ ## S,            \
                 AF_BLUE_STRINGSET_ ## S,    \
                 AF_COVERAGE_ ## C )

#undef  META_STYLE_LATIN
#define META_STYLE_LATIN( s, S, ds )                     \
          STYLE_LATIN( s, S, c2cp, C2CP, ds,             \
                       "petite capticals from capitals", \
                       PETITE_CAPITALS_FROM_CAPITALS )   \
          STYLE_LATIN( s, S, c2sc, C2SC, ds,             \
                       "small capticals from capitals",  \
                       SMALL_CAPITALS_FROM_CAPITALS )    \
          STYLE_LATIN( s, S, ordn, ORDN, ds,             \
                       "ordinals",                       \
                       ORDINALS )                        \
          STYLE_LATIN( s, S, pcap, PCAP, ds,             \
                       "petite capitals",                \
                       PETITE_CAPITALS )                 \
          STYLE_LATIN( s, S, sinf, SINF, ds,             \
                       "scientific inferiors",           \
                       SCIENTIFIC_INFERIORS )            \
          STYLE_LATIN( s, S, smcp, SMCP, ds,             \
                       "small capitals",                 \
                       SMALL_CAPITALS )                  \
          STYLE_LATIN( s, S, subs, SUBS, ds,             \
                       "subscript",                      \
                       SUBSCRIPT )                       \
          STYLE_LATIN( s, S, sups, SUPS, ds,             \
                       "superscript",                    \
                       SUPERSCRIPT )                     \
          STYLE_LATIN( s, S, titl, TITL, ds,             \
                       "titling",                        \
                       TITLING )                         \
          STYLE_LATIN( s, S, dflt, DFLT, ds,             \
                       "default",                        \
                       DEFAULT )


  STYLE( arab_dflt, ARAB_DFLT,
         "Arabic default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_ARAB,
         AF_BLUE_STRINGSET_ARAB,
         AF_COVERAGE_DEFAULT )

  STYLE( beng_dflt, BENG_DFLT,
         "Bengali default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_BENG,
         AF_BLUE_STRINGSET_BENG,
         AF_COVERAGE_DEFAULT )

  META_STYLE_LATIN( cyrl, CYRL, "Cyrillic" )

  STYLE( deva_dflt, DEVA_DFLT,
         "Devanagari default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_DEVA,
         AF_BLUE_STRINGSET_DEVA,
         AF_COVERAGE_DEFAULT )

  META_STYLE_LATIN( grek, GREK, "Greek" )

  STYLE( hebr_dflt, HEBR_DFLT,
         "Hebrew default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_HEBR,
         AF_BLUE_STRINGSET_HEBR,
         AF_COVERAGE_DEFAULT )

  STYLE( knda_dflt, KNDA_DFLT,
         "Kannada default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_KNDA,
         AF_BLUE_STRINGSET_KNDA,
         AF_COVERAGE_DEFAULT )

  STYLE( khmr_dflt, KHMR_DFLT,
         "Khmer default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_KHMR,
         AF_BLUE_STRINGSET_KHMR,
         AF_COVERAGE_DEFAULT )

  STYLE( khms_dflt, KHMS_DFLT,
         "Khmer Symbols default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_KHMS,
         AF_BLUE_STRINGSET_KHMS,
         AF_COVERAGE_DEFAULT )

  STYLE( lao_dflt, LAO_DFLT,
         "Lao default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_LAO,
         AF_BLUE_STRINGSET_LAO,
         AF_COVERAGE_DEFAULT )

  META_STYLE_LATIN( latn, LATN, "Latin" )

  STYLE( latb_dflt, LATB_DFLT,
         "Latin subscript fallback default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_LATB,
         AF_BLUE_STRINGSET_LATB,
         AF_COVERAGE_DEFAULT )

  STYLE( latp_dflt, LATP_DFLT,
         "Latin superscript fallback default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_LATP,
         AF_BLUE_STRINGSET_LATP,
         AF_COVERAGE_DEFAULT )

#ifdef FT_OPTION_AUTOFIT2
  STYLE( ltn2_dflt, LTN2_DFLT,
         "Latin 2 default style",
         AF_WRITING_SYSTEM_LATIN2,
         AF_SCRIPT_LATN,
         AF_BLUE_STRINGSET_LATN,
         AF_COVERAGE_DEFAULT )
#endif

  STYLE( mymr_dflt, MYMR_DFLT,
         "Myanmar default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_MYMR,
         AF_BLUE_STRINGSET_MYMR,
         AF_COVERAGE_DEFAULT )

  STYLE( none_dflt, NONE_DFLT,
         "no style",
         AF_WRITING_SYSTEM_DUMMY,
         AF_SCRIPT_NONE,
         (AF_Blue_Stringset)0,
         AF_COVERAGE_DEFAULT )

  STYLE( telu_dflt, TELU_DFLT,
         "Telugu default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_TELU,
         AF_BLUE_STRINGSET_TELU,
         AF_COVERAGE_DEFAULT )

  STYLE( thai_dflt, THAI_DFLT,
         "Thai default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_THAI,
         AF_BLUE_STRINGSET_THAI,
         AF_COVERAGE_DEFAULT )

#ifdef AF_CONFIG_OPTION_INDIC

  /* no blue stringset support for the Indic writing system yet */
#undef  STYLE_DEFAULT_INDIC
#define STYLE_DEFAULT_INDIC( s, S, d )    \
          STYLE( s ## _dflt, S ## _DFLT,  \
                 d " default style",      \
                 AF_WRITING_SYSTEM_INDIC, \
                 AF_SCRIPT_ ## S,         \
                 (AF_Blue_Stringset)0,    \
                 AF_COVERAGE_DEFAULT )

  STYLE_DEFAULT_INDIC( gujr, GUJR, "Gujarati" )
  STYLE_DEFAULT_INDIC( guru, GURU, "Gurmukhi" )
  STYLE_DEFAULT_INDIC( limb, LIMB, "Limbu" )
  STYLE_DEFAULT_INDIC( mlym, MLYM, "Malayalam" )
  STYLE_DEFAULT_INDIC( orya, ORYA, "Oriya" )
  STYLE_DEFAULT_INDIC( sinh, SINH, "Sinhala" )
  STYLE_DEFAULT_INDIC( sund, SUND, "Sundanese" )
  STYLE_DEFAULT_INDIC( sylo, SYLO, "Syloti Nagri" )
  STYLE_DEFAULT_INDIC( taml, TAML, "Tamil" )
  STYLE_DEFAULT_INDIC( tibt, TIBT, "Tibetan" )

#endif /* AF_CONFIG_OPTION_INDIC */

#ifdef AF_CONFIG_OPTION_CJK

  STYLE( hani_dflt, HANI_DFLT,
         "CJKV ideographs default style",
         AF_WRITING_SYSTEM_CJK,
         AF_SCRIPT_HANI,
         AF_BLUE_STRINGSET_HANI,
         AF_COVERAGE_DEFAULT )

#endif /* AF_CONFIG_OPTION_CJK */


/* END */
