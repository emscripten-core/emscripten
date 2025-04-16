/***************************************************************************/
/*                                                                         */
/*  ftvalid.h                                                              */
/*                                                                         */
/*    FreeType validation support (specification).                         */
/*                                                                         */
/*  Copyright 2004 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTVALID_H__
#define __FTVALID_H__

#include <ft2build.h>
#include FT_CONFIG_STANDARD_LIBRARY_H   /* for ft_setjmp and ft_longjmp */


FT_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                    V A L I D A T I O N                          ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* handle to a validation object */
  typedef struct FT_ValidatorRec_ volatile*  FT_Validator;


  /*************************************************************************/
  /*                                                                       */
  /* There are three distinct validation levels defined here:              */
  /*                                                                       */
  /* FT_VALIDATE_DEFAULT ::                                                */
  /*   A table that passes this validation level can be used reliably by   */
  /*   FreeType.  It generally means that all offsets have been checked to */
  /*   prevent out-of-bound reads, that array counts are correct, etc.     */
  /*                                                                       */
  /* FT_VALIDATE_TIGHT ::                                                  */
  /*   A table that passes this validation level can be used reliably and  */
  /*   doesn't contain invalid data.  For example, a charmap table that    */
  /*   returns invalid glyph indices will not pass, even though it can     */
  /*   be used with FreeType in default mode (the library will simply      */
  /*   return an error later when trying to load the glyph).               */
  /*                                                                       */
  /*   It also checks that fields which must be a multiple of 2, 4, or 8,  */
  /*   don't have incorrect values, etc.                                   */
  /*                                                                       */
  /* FT_VALIDATE_PARANOID ::                                               */
  /*   Only for font debugging.  Checks that a table follows the           */
  /*   specification by 100%.  Very few fonts will be able to pass this    */
  /*   level anyway but it can be useful for certain tools like font       */
  /*   editors/converters.                                                 */
  /*                                                                       */
  typedef enum  FT_ValidationLevel_
  {
    FT_VALIDATE_DEFAULT = 0,
    FT_VALIDATE_TIGHT,
    FT_VALIDATE_PARANOID

  } FT_ValidationLevel;


  /* validator structure */
  typedef struct  FT_ValidatorRec_
  {
    const FT_Byte*      base;        /* address of table in memory       */
    const FT_Byte*      limit;       /* `base' + sizeof(table) in memory */
    FT_ValidationLevel  level;       /* validation level                 */
    FT_Error            error;       /* error returned. 0 means success  */

    ft_jmp_buf          jump_buffer; /* used for exception handling      */

  } FT_ValidatorRec;


#define FT_VALIDATOR( x )  ((FT_Validator)( x ))


  FT_BASE( void )
  ft_validator_init( FT_Validator        valid,
                     const FT_Byte*      base,
                     const FT_Byte*      limit,
                     FT_ValidationLevel  level );

  /* Do not use this. It's broken and will cause your validator to crash */
  /* if you run it on an invalid font.                                   */
  FT_BASE( FT_Int )
  ft_validator_run( FT_Validator  valid );

  /* Sets the error field in a validator, then calls `longjmp' to return */
  /* to high-level caller.  Using `setjmp/longjmp' avoids many stupid    */
  /* error checks within the validation routines.                        */
  /*                                                                     */
  FT_BASE( void )
  ft_validator_error( FT_Validator  valid,
                      FT_Error      error );


  /* Calls ft_validate_error.  Assumes that the `valid' local variable */
  /* holds a pointer to the current validator object.                  */
  /*                                                                   */
  /* Use preprocessor prescan to pass FT_ERR_PREFIX.                   */
  /*                                                                   */
#define FT_INVALID( _prefix, _error )  FT_INVALID_( _prefix, _error )
#define FT_INVALID_( _prefix, _error ) \
          ft_validator_error( valid, _prefix ## _error )

  /* called when a broken table is detected */
#define FT_INVALID_TOO_SHORT \
          FT_INVALID( FT_ERR_PREFIX, Invalid_Table )

  /* called when an invalid offset is detected */
#define FT_INVALID_OFFSET \
          FT_INVALID( FT_ERR_PREFIX, Invalid_Offset )

  /* called when an invalid format/value is detected */
#define FT_INVALID_FORMAT \
          FT_INVALID( FT_ERR_PREFIX, Invalid_Table )

  /* called when an invalid glyph index is detected */
#define FT_INVALID_GLYPH_ID \
          FT_INVALID( FT_ERR_PREFIX, Invalid_Glyph_Index )

  /* called when an invalid field value is detected */
#define FT_INVALID_DATA \
          FT_INVALID( FT_ERR_PREFIX, Invalid_Table )


FT_END_HEADER

#endif /* __FTVALID_H__ */


/* END */
