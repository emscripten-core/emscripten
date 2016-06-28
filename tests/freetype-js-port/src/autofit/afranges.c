/***************************************************************************/
/*                                                                         */
/*  afranges.c                                                             */
/*                                                                         */
/*    Auto-fitter Unicode script ranges (body).                            */
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


#include "afranges.h"

  /*
   * The algorithm for assigning properties and styles to the `glyph_styles'
   * array is as follows (cf. the implementation in
   * `af_face_globals_compute_style_coverage').
   *
   *   Walk over all scripts (as listed in `afscript.h').
   *
   *   For a given script, walk over all styles (as listed in `afstyles.h').
   *   The order of styles is important and should be as follows.
   *
   *   - First come styles based on OpenType features (small caps, for
   *     example).  Since features rely on glyph indices, thus completely
   *     bypassing character codes, no properties are assigned.
   *
   *   - Next comes the default style, using the character ranges as defined
   *     below.  This also assigns properties.
   *
   *   Note that there also exist fallback scripts, mainly covering
   *   superscript and subscript glyphs of a script that are not present as
   *   OpenType features.  Fallback scripts are defined below, also
   *   assigning properties; they are applied after the corresponding
   *   script.
   *
   */


  /* XXX Check base character ranges again:                        */
  /*     Right now, they are quickly derived by visual inspection. */
  /*     I can imagine that fine-tuning is necessary.              */

  /* for the auto-hinter, a `non-base character' is something that should */
  /* not be affected by blue zones, regardless of whether this is a       */
  /* spacing or no-spacing glyph                                          */

  /* the `ta_xxxx_nonbase_uniranges' ranges must be strict subsets */
  /* of the corresponding `ta_xxxx_uniranges' ranges               */


  const AF_Script_UniRangeRec  af_arab_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0600UL,  0x06FFUL ),  /* Arabic                                 */
    AF_UNIRANGE_REC(  0x0750UL,  0x07FFUL ),  /* Arabic Supplement                      */
    AF_UNIRANGE_REC(  0x08A0UL,  0x08FFUL ),  /* Arabic Extended-A                      */
    AF_UNIRANGE_REC(  0xFB50UL,  0xFDFFUL ),  /* Arabic Presentation Forms-A            */
    AF_UNIRANGE_REC(  0xFE70UL,  0xFEFFUL ),  /* Arabic Presentation Forms-B            */
    AF_UNIRANGE_REC( 0x1EE00UL, 0x1EEFFUL ),  /* Arabic Mathematical Alphabetic Symbols */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_arab_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0600UL,  0x0605UL ),
    AF_UNIRANGE_REC(  0x0610UL,  0x061AUL ),
    AF_UNIRANGE_REC(  0x064BUL,  0x065FUL ),
    AF_UNIRANGE_REC(  0x0670UL,  0x0670UL ),
    AF_UNIRANGE_REC(  0x06D6UL,  0x06DCUL ),
    AF_UNIRANGE_REC(  0x06DFUL,  0x06E4UL ),
    AF_UNIRANGE_REC(  0x06E7UL,  0x06E8UL ),
    AF_UNIRANGE_REC(  0x06EAUL,  0x06EDUL ),
    AF_UNIRANGE_REC(  0x08E3UL,  0x08FFUL ),
    AF_UNIRANGE_REC(  0xFBB2UL,  0xFBC1UL ),
    AF_UNIRANGE_REC(  0xFE70UL,  0xFE70UL ),
    AF_UNIRANGE_REC(  0xFE72UL,  0xFE72UL ),
    AF_UNIRANGE_REC(  0xFE74UL,  0xFE74UL ),
    AF_UNIRANGE_REC(  0xFE76UL,  0xFE76UL ),
    AF_UNIRANGE_REC(  0xFE78UL,  0xFE78UL ),
    AF_UNIRANGE_REC(  0xFE7AUL,  0xFE7AUL ),
    AF_UNIRANGE_REC(  0xFE7CUL,  0xFE7CUL ),
    AF_UNIRANGE_REC(  0xFE7EUL,  0xFE7EUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_beng_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0980UL,  0x09FFUL ),  /* Bengali */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_beng_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0981UL,  0x0981UL ),
    AF_UNIRANGE_REC(  0x09BCUL,  0x09BCUL ),
    AF_UNIRANGE_REC(  0x09C1UL,  0x09C4UL ),
    AF_UNIRANGE_REC(  0x09CDUL,  0x09CDUL ),
    AF_UNIRANGE_REC(  0x09E2UL,  0x09E3UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_cyrl_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0400UL,  0x04FFUL ),  /* Cyrillic            */
    AF_UNIRANGE_REC(  0x0500UL,  0x052FUL ),  /* Cyrillic Supplement */
    AF_UNIRANGE_REC(  0x2DE0UL,  0x2DFFUL ),  /* Cyrillic Extended-A */
    AF_UNIRANGE_REC(  0xA640UL,  0xA69FUL ),  /* Cyrillic Extended-B */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_cyrl_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0483UL,  0x0489UL ),
    AF_UNIRANGE_REC(  0x2DE0UL,  0x2DFFUL ),
    AF_UNIRANGE_REC(  0xA66FUL,  0xA67FUL ),
    AF_UNIRANGE_REC(  0xA69EUL,  0xA69FUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  /* There are some characters in the Devanagari Unicode block that are    */
  /* generic to Indic scripts; we omit them so that their presence doesn't */
  /* trigger Devanagari.                                                   */

  const AF_Script_UniRangeRec  af_deva_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0900UL,  0x093BUL ),  /* Devanagari          */
    /* omitting U+093C nukta */
    AF_UNIRANGE_REC(  0x093DUL,  0x0950UL ),  /* ... continued       */
    /* omitting U+0951 udatta, U+0952 anudatta */
    AF_UNIRANGE_REC(  0x0953UL,  0x0963UL ),  /* ... continued       */
    /* omitting U+0964 danda, U+0965 double danda */
    AF_UNIRANGE_REC(  0x0966UL,  0x097FUL ),  /* ... continued       */
    AF_UNIRANGE_REC(  0x20B9UL,  0x20B9UL ),  /* (new) Rupee sign    */
    AF_UNIRANGE_REC(  0xA8E0UL,  0xA8FFUL ),  /* Devanagari Extended */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_deva_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0900UL,  0x0902UL ),
    AF_UNIRANGE_REC(  0x093AUL,  0x093AUL ),
    AF_UNIRANGE_REC(  0x0941UL,  0x0948UL ),
    AF_UNIRANGE_REC(  0x094DUL,  0x094DUL ),
    AF_UNIRANGE_REC(  0x0953UL,  0x0957UL ),
    AF_UNIRANGE_REC(  0x0962UL,  0x0963UL ),
    AF_UNIRANGE_REC(  0xA8E0UL,  0xA8F1UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_grek_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0370UL,  0x03FFUL ),  /* Greek and Coptic */
    AF_UNIRANGE_REC(  0x1F00UL,  0x1FFFUL ),  /* Greek Extended   */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_grek_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x037AUL,  0x037AUL ),
    AF_UNIRANGE_REC(  0x0384UL,  0x0385UL ),
    AF_UNIRANGE_REC(  0x1FBDUL,  0x1FC1UL ),
    AF_UNIRANGE_REC(  0x1FCDUL,  0x1FCFUL ),
    AF_UNIRANGE_REC(  0x1FDDUL,  0x1FDFUL ),
    AF_UNIRANGE_REC(  0x1FEDUL,  0x1FEFUL ),
    AF_UNIRANGE_REC(  0x1FFDUL,  0x1FFEUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_hebr_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0590UL,  0x05FFUL ),  /* Hebrew                          */
    AF_UNIRANGE_REC(  0xFB1DUL,  0xFB4FUL ),  /* Alphab. Present. Forms (Hebrew) */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_hebr_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0591UL,  0x05BFUL ),
    AF_UNIRANGE_REC(  0x05C1UL,  0x05C2UL ),
    AF_UNIRANGE_REC(  0x05C4UL,  0x05C5UL ),
    AF_UNIRANGE_REC(  0x05C7UL,  0x05C7UL ),
    AF_UNIRANGE_REC(  0xFB1EUL,  0xFB1EUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_knda_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0C80UL,  0x0CFFUL ),  /* Kannada */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_knda_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0C81UL,  0x0C81UL ),
    AF_UNIRANGE_REC(  0x0CBCUL,  0x0CBCUL ),
    AF_UNIRANGE_REC(  0x0CBFUL,  0x0CBFUL ),
    AF_UNIRANGE_REC(  0x0CC6UL,  0x0CC6UL ),
    AF_UNIRANGE_REC(  0x0CCCUL,  0x0CCDUL ),
    AF_UNIRANGE_REC(  0x0CE2UL,  0x0CE3UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_khmr_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1780UL,  0x17FFUL ),  /* Khmer */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_khmr_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x17B7UL,  0x17BDUL ),
    AF_UNIRANGE_REC(  0x17C6UL,  0x17C6UL ),
    AF_UNIRANGE_REC(  0x17C9UL,  0x17D3UL ),
    AF_UNIRANGE_REC(  0x17DDUL,  0x17DDUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_khms_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x19E0UL,  0x19FFUL ),  /* Khmer Symbols */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_khms_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC( 0UL, 0UL )
  };


  const AF_Script_UniRangeRec  af_lao_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0E80UL,  0x0EFFUL ),  /* Lao */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_lao_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0EB1UL,  0x0EB1UL ),
    AF_UNIRANGE_REC(  0x0EB4UL,  0x0EBCUL ),
    AF_UNIRANGE_REC(  0x0EC8UL,  0x0ECDUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_latn_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0020UL,  0x007FUL ),  /* Basic Latin (no control chars)         */
    AF_UNIRANGE_REC(  0x00A0UL,  0x00A9UL ),  /* Latin-1 Supplement (no control chars)  */
    AF_UNIRANGE_REC(  0x00ABUL,  0x00B1UL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x00B4UL,  0x00B8UL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x00BBUL,  0x00FFUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x0100UL,  0x017FUL ),  /* Latin Extended-A                       */
    AF_UNIRANGE_REC(  0x0180UL,  0x024FUL ),  /* Latin Extended-B                       */
    AF_UNIRANGE_REC(  0x0250UL,  0x02AFUL ),  /* IPA Extensions                         */
    AF_UNIRANGE_REC(  0x02B9UL,  0x02DFUL ),  /* Spacing Modifier Letters               */
    AF_UNIRANGE_REC(  0x02E5UL,  0x02FFUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x0300UL,  0x036FUL ),  /* Combining Diacritical Marks            */
    AF_UNIRANGE_REC(  0x1AB0UL,  0x1ABEUL ),  /* Combining Diacritical Marks Extended   */
    AF_UNIRANGE_REC(  0x1D00UL,  0x1D2BUL ),  /* Phonetic Extensions                    */
    AF_UNIRANGE_REC(  0x1D6BUL,  0x1D77UL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x1D79UL,  0x1D7FUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x1D80UL,  0x1D9AUL ),  /* Phonetic Extensions Supplement         */
    AF_UNIRANGE_REC(  0x1DC0UL,  0x1DFFUL ),  /* Combining Diacritical Marks Supplement */
    AF_UNIRANGE_REC(  0x1E00UL,  0x1EFFUL ),  /* Latin Extended Additional              */
    AF_UNIRANGE_REC(  0x2000UL,  0x206FUL ),  /* General Punctuation                    */
    AF_UNIRANGE_REC(  0x20A0UL,  0x20B8UL ),  /* Currency Symbols ...                   */
    AF_UNIRANGE_REC(  0x20BAUL,  0x20CFUL ),  /* ... except new Rupee sign              */
    AF_UNIRANGE_REC(  0x2150UL,  0x218FUL ),  /* Number Forms                           */
    AF_UNIRANGE_REC(  0x2C60UL,  0x2C7BUL ),  /* Latin Extended-C                       */
    AF_UNIRANGE_REC(  0x2C7EUL,  0x2C7FUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0x2E00UL,  0x2E7FUL ),  /* Supplemental Punctuation               */
    AF_UNIRANGE_REC(  0xA720UL,  0xA76FUL ),  /* Latin Extended-D                       */
    AF_UNIRANGE_REC(  0xA771UL,  0xA7F7UL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0xA7FAUL,  0xA7FFUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0xAB30UL,  0xAB5BUL ),  /* Latin Extended-E                       */
    AF_UNIRANGE_REC(  0xAB60UL,  0xAB6FUL ),  /* ... continued                          */
    AF_UNIRANGE_REC(  0xFB00UL,  0xFB06UL ),  /* Alphab. Present. Forms (Latin Ligs)    */
    AF_UNIRANGE_REC( 0x1D400UL, 0x1D7FFUL ),  /* Mathematical Alphanumeric Symbols      */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_latn_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x005EUL,  0x0060UL ),
    AF_UNIRANGE_REC(  0x007EUL,  0x007EUL ),
    AF_UNIRANGE_REC(  0x00A8UL,  0x00A9UL ),
    AF_UNIRANGE_REC(  0x00AEUL,  0x00B0UL ),
    AF_UNIRANGE_REC(  0x00B4UL,  0x00B4UL ),
    AF_UNIRANGE_REC(  0x00B8UL,  0x00B8UL ),
    AF_UNIRANGE_REC(  0x00BCUL,  0x00BEUL ),
    AF_UNIRANGE_REC(  0x02B9UL,  0x02DFUL ),
    AF_UNIRANGE_REC(  0x02E5UL,  0x02FFUL ),
    AF_UNIRANGE_REC(  0x0300UL,  0x036FUL ),
    AF_UNIRANGE_REC(  0x1AB0UL,  0x1ABEUL ),
    AF_UNIRANGE_REC(  0x1DC0UL,  0x1DFFUL ),
    AF_UNIRANGE_REC(  0x2017UL,  0x2017UL ),
    AF_UNIRANGE_REC(  0x203EUL,  0x203EUL ),
    AF_UNIRANGE_REC(  0xA788UL,  0xA788UL ),
    AF_UNIRANGE_REC(  0xA7F8UL,  0xA7FAUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_latb_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1D62UL,  0x1D6AUL ),  /* some small subscript letters   */
    AF_UNIRANGE_REC(  0x2080UL,  0x209CUL ),  /* subscript digits and letters   */
    AF_UNIRANGE_REC(  0x2C7CUL,  0x2C7CUL ),  /* latin subscript small letter j */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_latb_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC( 0UL, 0UL )
  };


  const AF_Script_UniRangeRec  af_latp_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x00AAUL,  0x00AAUL ),  /* feminine ordinal indicator          */
    AF_UNIRANGE_REC(  0x00B2UL,  0x00B3UL ),  /* superscript two and three           */
    AF_UNIRANGE_REC(  0x00B9UL,  0x00BAUL ),  /* superscript one, masc. ord. indic.  */
    AF_UNIRANGE_REC(  0x02B0UL,  0x02B8UL ),  /* some latin superscript mod. letters */
    AF_UNIRANGE_REC(  0x02E0UL,  0x02E4UL ),  /* some IPA modifier letters           */
    AF_UNIRANGE_REC(  0x1D2CUL,  0x1D61UL ),  /* latin superscript modifier letters  */
    AF_UNIRANGE_REC(  0x1D78UL,  0x1D78UL ),  /* modifier letter cyrillic en         */
    AF_UNIRANGE_REC(  0x1D9BUL,  0x1DBFUL ),  /* more modifier letters               */
    AF_UNIRANGE_REC(  0x2070UL,  0x207FUL ),  /* superscript digits and letters      */
    AF_UNIRANGE_REC(  0x2C7DUL,  0x2C7DUL ),  /* modifier letter capital v           */
    AF_UNIRANGE_REC(  0xA770UL,  0xA770UL ),  /* modifier letter us                  */
    AF_UNIRANGE_REC(  0xA7F8UL,  0xA7F9UL ),  /* more modifier letters               */
    AF_UNIRANGE_REC(  0xAB5CUL,  0xAB5FUL ),  /* more modifier letters               */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_latp_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC( 0UL, 0UL )
  };


  const AF_Script_UniRangeRec  af_mymr_uniranges[] =
  {
    AF_UNIRANGE_REC( 0x1000UL, 0x109FUL ),    /* Myanmar            */
    AF_UNIRANGE_REC( 0xA9E0UL, 0xA9FFUL ),    /* Myanmar Extended-B */
    AF_UNIRANGE_REC( 0xAA60UL, 0xAA7FUL ),    /* Myanmar Extended-A */
    AF_UNIRANGE_REC(      0UL,      0UL )
  };

  const AF_Script_UniRangeRec  af_mymr_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC( 0x102DUL, 0x1030UL ),
    AF_UNIRANGE_REC( 0x1032UL, 0x1037UL ),
    AF_UNIRANGE_REC( 0x103AUL, 0x103AUL ),
    AF_UNIRANGE_REC( 0x103DUL, 0x103EUL ),
    AF_UNIRANGE_REC( 0x1058UL, 0x1059UL ),
    AF_UNIRANGE_REC( 0x105EUL, 0x1060UL ),
    AF_UNIRANGE_REC( 0x1071UL, 0x1074UL ),
    AF_UNIRANGE_REC( 0x1082UL, 0x1082UL ),
    AF_UNIRANGE_REC( 0x1085UL, 0x1086UL ),
    AF_UNIRANGE_REC( 0x108DUL, 0x108DUL ),
    AF_UNIRANGE_REC( 0xA9E5UL, 0xA9E5UL ),
    AF_UNIRANGE_REC( 0xAA7CUL, 0xAA7CUL ),
    AF_UNIRANGE_REC(      0UL,      0UL )
  };


  const AF_Script_UniRangeRec  af_none_uniranges[] =
  {
    AF_UNIRANGE_REC( 0UL, 0UL )
  };

  const AF_Script_UniRangeRec  af_none_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC( 0UL, 0UL )
  };


  const AF_Script_UniRangeRec  af_telu_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0C00UL,  0x0C7FUL ),  /* Telugu */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_telu_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0C00UL,  0x0C00UL ),
    AF_UNIRANGE_REC(  0x0C3EUL,  0x0C40UL ),
    AF_UNIRANGE_REC(  0x0C46UL,  0x0C56UL ),
    AF_UNIRANGE_REC(  0x0C62UL,  0x0C63UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_thai_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0E00UL,  0x0E7FUL ),  /* Thai */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_thai_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0E31UL,  0x0E31UL ),
    AF_UNIRANGE_REC(  0x0E34UL,  0x0E3AUL ),
    AF_UNIRANGE_REC(  0x0E47UL,  0x0E4EUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


#ifdef AF_CONFIG_OPTION_INDIC

  const AF_Script_UniRangeRec  af_gujr_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0A80UL,  0x0AFFUL ),  /* Gujarati */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_gujr_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0A81UL,  0x0A82UL ),
    AF_UNIRANGE_REC(  0x0ABCUL,  0x0ABCUL ),
    AF_UNIRANGE_REC(  0x0AC1UL,  0x0AC8UL ),
    AF_UNIRANGE_REC(  0x0ACDUL,  0x0ACDUL ),
    AF_UNIRANGE_REC(  0x0AE2UL,  0x0AE3UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_guru_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0A00UL,  0x0A7FUL ),  /* Gurmukhi */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_guru_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0A01UL,  0x0A02UL ),
    AF_UNIRANGE_REC(  0x0A3CUL,  0x0A3EUL ),
    AF_UNIRANGE_REC(  0x0A41UL,  0x0A51UL ),
    AF_UNIRANGE_REC(  0x0A70UL,  0x0A71UL ),
    AF_UNIRANGE_REC(  0x0A75UL,  0x0A75UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_limb_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1900UL,  0x194FUL ),  /* Limbu */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_limb_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1920UL,  0x1922UL ),
    AF_UNIRANGE_REC(  0x1927UL,  0x1934UL ),
    AF_UNIRANGE_REC(  0x1937UL,  0x193BUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_mlym_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0D00UL,  0x0D7FUL ),  /* Malayalam */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_mlym_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0D01UL,  0x0D01UL ),
    AF_UNIRANGE_REC(  0x0D4DUL,  0x0D4EUL ),
    AF_UNIRANGE_REC(  0x0D62UL,  0x0D63UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_orya_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0B00UL,  0x0B7FUL ),  /* Oriya */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_orya_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0B01UL,  0x0B02UL ),
    AF_UNIRANGE_REC(  0x0B3CUL,  0x0B3CUL ),
    AF_UNIRANGE_REC(  0x0B3FUL,  0x0B3FUL ),
    AF_UNIRANGE_REC(  0x0B41UL,  0x0B44UL ),
    AF_UNIRANGE_REC(  0x0B4DUL,  0x0B56UL ),
    AF_UNIRANGE_REC(  0x0B62UL,  0x0B63UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_sinh_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0D80UL,  0x0DFFUL ),  /* Sinhala */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_sinh_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0DCAUL,  0x0DCAUL ),
    AF_UNIRANGE_REC(  0x0DD2UL,  0x0DD6UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_sund_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1B80UL,  0x1BBFUL ),  /* Sundanese            */
    AF_UNIRANGE_REC(  0x1CC0UL,  0x1CCFUL ),  /* Sundanese Supplement */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_sund_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1B80UL,  0x1B82UL ),
    AF_UNIRANGE_REC(  0x1BA1UL,  0x1BADUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_sylo_uniranges[] =
  {
    AF_UNIRANGE_REC(  0xA800UL,  0xA82FUL ),  /* Syloti Nagri */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_sylo_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0xA802UL,  0xA802UL ),
    AF_UNIRANGE_REC(  0xA806UL,  0xA806UL ),
    AF_UNIRANGE_REC(  0xA80BUL,  0xA80BUL ),
    AF_UNIRANGE_REC(  0xA825UL,  0xA826UL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_taml_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0B80UL,  0x0BFFUL ),  /* Tamil */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_taml_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0B82UL,  0x0B82UL ),
    AF_UNIRANGE_REC(  0x0BC0UL,  0x0BC2UL ),
    AF_UNIRANGE_REC(  0x0BCDUL,  0x0BCDUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };


  const AF_Script_UniRangeRec  af_tibt_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0F00UL,  0x0FFFUL ),  /* Tibetan */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_tibt_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x0F18UL,  0x0F19UL ),
    AF_UNIRANGE_REC(  0x0F35UL,  0x0F35UL ),
    AF_UNIRANGE_REC(  0x0F37UL,  0x0F37UL ),
    AF_UNIRANGE_REC(  0x0F39UL,  0x0F39UL ),
    AF_UNIRANGE_REC(  0x0F3EUL,  0x0F3FUL ),
    AF_UNIRANGE_REC(  0x0F71UL,  0x0F7EUL ),
    AF_UNIRANGE_REC(  0x0F80UL,  0x0F84UL ),
    AF_UNIRANGE_REC(  0x0F86UL,  0x0F87UL ),
    AF_UNIRANGE_REC(  0x0F8DUL,  0x0FBCUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

#endif /* !AF_CONFIG_OPTION_INDIC */

#ifdef AF_CONFIG_OPTION_CJK

  /* this corresponds to Unicode 6.0 */

  const AF_Script_UniRangeRec  af_hani_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x1100UL,  0x11FFUL ),  /* Hangul Jamo                             */
    AF_UNIRANGE_REC(  0x2E80UL,  0x2EFFUL ),  /* CJK Radicals Supplement                 */
    AF_UNIRANGE_REC(  0x2F00UL,  0x2FDFUL ),  /* Kangxi Radicals                         */
    AF_UNIRANGE_REC(  0x2FF0UL,  0x2FFFUL ),  /* Ideographic Description Characters      */
    AF_UNIRANGE_REC(  0x3000UL,  0x303FUL ),  /* CJK Symbols and Punctuation             */
    AF_UNIRANGE_REC(  0x3040UL,  0x309FUL ),  /* Hiragana                                */
    AF_UNIRANGE_REC(  0x30A0UL,  0x30FFUL ),  /* Katakana                                */
    AF_UNIRANGE_REC(  0x3100UL,  0x312FUL ),  /* Bopomofo                                */
    AF_UNIRANGE_REC(  0x3130UL,  0x318FUL ),  /* Hangul Compatibility Jamo               */
    AF_UNIRANGE_REC(  0x3190UL,  0x319FUL ),  /* Kanbun                                  */
    AF_UNIRANGE_REC(  0x31A0UL,  0x31BFUL ),  /* Bopomofo Extended                       */
    AF_UNIRANGE_REC(  0x31C0UL,  0x31EFUL ),  /* CJK Strokes                             */
    AF_UNIRANGE_REC(  0x31F0UL,  0x31FFUL ),  /* Katakana Phonetic Extensions            */
    AF_UNIRANGE_REC(  0x3300UL,  0x33FFUL ),  /* CJK Compatibility                       */
    AF_UNIRANGE_REC(  0x3400UL,  0x4DBFUL ),  /* CJK Unified Ideographs Extension A      */
    AF_UNIRANGE_REC(  0x4DC0UL,  0x4DFFUL ),  /* Yijing Hexagram Symbols                 */
    AF_UNIRANGE_REC(  0x4E00UL,  0x9FFFUL ),  /* CJK Unified Ideographs                  */
    AF_UNIRANGE_REC(  0xA960UL,  0xA97FUL ),  /* Hangul Jamo Extended-A                  */
    AF_UNIRANGE_REC(  0xAC00UL,  0xD7AFUL ),  /* Hangul Syllables                        */
    AF_UNIRANGE_REC(  0xD7B0UL,  0xD7FFUL ),  /* Hangul Jamo Extended-B                  */
    AF_UNIRANGE_REC(  0xF900UL,  0xFAFFUL ),  /* CJK Compatibility Ideographs            */
    AF_UNIRANGE_REC(  0xFE10UL,  0xFE1FUL ),  /* Vertical forms                          */
    AF_UNIRANGE_REC(  0xFE30UL,  0xFE4FUL ),  /* CJK Compatibility Forms                 */
    AF_UNIRANGE_REC(  0xFF00UL,  0xFFEFUL ),  /* Halfwidth and Fullwidth Forms           */
    AF_UNIRANGE_REC( 0x1B000UL, 0x1B0FFUL ),  /* Kana Supplement                         */
    AF_UNIRANGE_REC( 0x1D300UL, 0x1D35FUL ),  /* Tai Xuan Hing Symbols                   */
    AF_UNIRANGE_REC( 0x20000UL, 0x2A6DFUL ),  /* CJK Unified Ideographs Extension B      */
    AF_UNIRANGE_REC( 0x2A700UL, 0x2B73FUL ),  /* CJK Unified Ideographs Extension C      */
    AF_UNIRANGE_REC( 0x2B740UL, 0x2B81FUL ),  /* CJK Unified Ideographs Extension D      */
    AF_UNIRANGE_REC( 0x2F800UL, 0x2FA1FUL ),  /* CJK Compatibility Ideographs Supplement */
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

  const AF_Script_UniRangeRec  af_hani_nonbase_uniranges[] =
  {
    AF_UNIRANGE_REC(  0x302AUL,  0x302FUL ),
    AF_UNIRANGE_REC(  0x3190UL,  0x319FUL ),
    AF_UNIRANGE_REC(       0UL,       0UL )
  };

#endif /* !AF_CONFIG_OPTION_CJK */

/* END */
