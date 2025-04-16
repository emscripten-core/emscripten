/***************************************************************************/
/*                                                                         */
/*  gxvfeat.c                                                              */
/*                                                                         */
/*    TrueTypeGX/AAT feat table validation (body).                         */
/*                                                                         */
/*  Copyright 2004, 2005, 2008 by                                          */
/*  suzuki toshiya, Masatake YAMATO, Red Hat K.K.,                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout is supported by the Information-technology      */
/* Promotion Agency(IPA), Japan.                                           */
/*                                                                         */
/***************************************************************************/


#include "gxvalid.h"
#include "gxvcommn.h"
#include "gxvfeat.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvfeat


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      Data and Types                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  GXV_feat_DataRec_
  {
    FT_UInt    reserved_size;
    FT_UShort  feature;
    FT_UShort  setting;

  } GXV_feat_DataRec, *GXV_feat_Data;


#define GXV_FEAT_DATA( field )  GXV_TABLE_DATA( feat, field )


  typedef enum  GXV_FeatureFlagsMask_
  {
    GXV_FEAT_MASK_EXCLUSIVE_SETTINGS = 0x8000U,
    GXV_FEAT_MASK_DYNAMIC_DEFAULT    = 0x4000,
    GXV_FEAT_MASK_UNUSED             = 0x3F00,
    GXV_FEAT_MASK_DEFAULT_SETTING    = 0x00FF

  } GXV_FeatureFlagsMask;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      UTILITY FUNCTIONS                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  gxv_feat_registry_validate( FT_UShort      feature,
                              FT_UShort      nSettings,
                              FT_Bool        exclusive,
                              GXV_Validator  valid )
  {
    GXV_NAME_ENTER( "feature in registry" );

    GXV_TRACE(( " (feature = %u)\n", feature ));

    if ( feature >= gxv_feat_registry_length )
    {
      GXV_TRACE(( "feature number %d is out of range %d\n",
                  feature, gxv_feat_registry_length ));
      if ( valid->root->level == FT_VALIDATE_PARANOID )
        FT_INVALID_DATA;
      goto Exit;
    }

    if ( gxv_feat_registry[feature].existence == 0 )
    {
      GXV_TRACE(( "feature number %d is in defined range but doesn't exist\n",
                  feature ));
      if ( valid->root->level == FT_VALIDATE_PARANOID )
        FT_INVALID_DATA;
      goto Exit;
    }

    if ( gxv_feat_registry[feature].apple_reserved )
    {
      /* Don't use here. Apple is reserved. */
      GXV_TRACE(( "feature number %d is reserved by Apple\n", feature ));
      if ( valid->root->level >= FT_VALIDATE_TIGHT )
        FT_INVALID_DATA;
    }

    if ( nSettings != gxv_feat_registry[feature].nSettings )
    {
      GXV_TRACE(( "feature %d: nSettings %d != defined nSettings %d\n",
                  feature, nSettings,
                  gxv_feat_registry[feature].nSettings ));
      if ( valid->root->level >= FT_VALIDATE_TIGHT )
        FT_INVALID_DATA;
    }

    if ( exclusive != gxv_feat_registry[feature].exclusive )
    {
      GXV_TRACE(( "exclusive flag %d differs from predefined value\n",
                  exclusive ));
      if ( valid->root->level >= FT_VALIDATE_TIGHT )
        FT_INVALID_DATA;
    }

  Exit:
    GXV_EXIT;
  }


  static void
  gxv_feat_name_index_validate( FT_Bytes       table,
                                FT_Bytes       limit,
                                GXV_Validator  valid )
  {
    FT_Bytes  p = table;

    FT_Short  nameIndex;


    GXV_NAME_ENTER( "nameIndex" );

    GXV_LIMIT_CHECK( 2 );
    nameIndex = FT_NEXT_SHORT ( p );
    GXV_TRACE(( " (nameIndex = %d)\n", nameIndex ));

    gxv_sfntName_validate( (FT_UShort)nameIndex,
                           255,
                           32768U,
                           valid );

    GXV_EXIT;
  }


  static void
  gxv_feat_setting_validate( FT_Bytes       table,
                             FT_Bytes       limit,
                             FT_Bool        exclusive,
                             GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  setting;


    GXV_NAME_ENTER( "setting" );

    GXV_LIMIT_CHECK( 2 );

    setting = FT_NEXT_USHORT( p );

    /* If we have exclusive setting, the setting should be odd. */
    if ( exclusive && ( setting % 2 ) == 0 )
      FT_INVALID_DATA;

    gxv_feat_name_index_validate( p, limit, valid );

    GXV_FEAT_DATA( setting ) = setting;

    GXV_EXIT;
  }


  static void
  gxv_feat_name_validate( FT_Bytes       table,
                          FT_Bytes       limit,
                          GXV_Validator  valid )
  {
    FT_Bytes   p             = table;
    FT_UInt    reserved_size = GXV_FEAT_DATA( reserved_size );

    FT_UShort  feature;
    FT_UShort  nSettings;
    FT_ULong   settingTable;
    FT_UShort  featureFlags;

    FT_Bool    exclusive;
    FT_Int     last_setting;
    FT_UInt    i;


    GXV_NAME_ENTER( "name" );

    /* feature + nSettings + settingTable + featureFlags */
    GXV_LIMIT_CHECK( 2 + 2 + 4 + 2 );

    feature = FT_NEXT_USHORT( p );
    GXV_FEAT_DATA( feature ) = feature;

    nSettings    = FT_NEXT_USHORT( p );
    settingTable = FT_NEXT_ULONG ( p );
    featureFlags = FT_NEXT_USHORT( p );

    if ( settingTable < reserved_size )
      FT_INVALID_OFFSET;

    if ( valid->root->level == FT_VALIDATE_PARANOID   &&
         ( featureFlags & GXV_FEAT_MASK_UNUSED ) == 0 )
      FT_INVALID_DATA;

    exclusive = FT_BOOL( featureFlags & GXV_FEAT_MASK_EXCLUSIVE_SETTINGS );
    if ( exclusive )
    {
      FT_Byte  dynamic_default;


      if ( featureFlags & GXV_FEAT_MASK_DYNAMIC_DEFAULT )
        dynamic_default = (FT_Byte)( featureFlags &
                                     GXV_FEAT_MASK_DEFAULT_SETTING );
      else
        dynamic_default = 0;

      /* If exclusive, check whether default setting is in the range. */
      if ( !( dynamic_default < nSettings ) )
        FT_INVALID_FORMAT;
    }

    gxv_feat_registry_validate( feature, nSettings, exclusive, valid );

    gxv_feat_name_index_validate( p, limit, valid );

    p = valid->root->base + settingTable;
    for ( last_setting = -1, i = 0; i < nSettings; i++ )
    {
      gxv_feat_setting_validate( p, limit, exclusive, valid );

      if ( valid->root->level == FT_VALIDATE_PARANOID       &&
           (FT_Int)GXV_FEAT_DATA( setting ) <= last_setting )
        FT_INVALID_FORMAT;

      last_setting = (FT_Int)GXV_FEAT_DATA( setting );
      /* setting + nameIndex */
      p += ( 2 + 2 );
    }

    GXV_EXIT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         feat TABLE                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( void )
  gxv_feat_validate( FT_Bytes      table,
                     FT_Face       face,
                     FT_Validator  ftvalid )
  {
    GXV_ValidatorRec  validrec;
    GXV_Validator     valid = &validrec;

    GXV_feat_DataRec  featrec;
    GXV_feat_Data     feat = &featrec;

    FT_Bytes          p     = table;
    FT_Bytes          limit = 0;

    FT_UInt           featureNameCount;

    FT_UInt           i;
    FT_Int            last_feature;


    valid->root       = ftvalid;
    valid->table_data = feat;
    valid->face       = face;

    FT_TRACE3(( "validating `feat' table\n" ));
    GXV_INIT;

    feat->reserved_size = 0;

    /* version + featureNameCount + none_0 + none_1  */
    GXV_LIMIT_CHECK( 4 + 2 + 2 + 4 );
    feat->reserved_size += 4 + 2 + 2 + 4;

    if ( FT_NEXT_ULONG( p ) != 0x00010000UL ) /* Version */
      FT_INVALID_FORMAT;

    featureNameCount = FT_NEXT_USHORT( p );
    GXV_TRACE(( " (featureNameCount = %d)\n", featureNameCount ));

    if ( valid->root->level != FT_VALIDATE_PARANOID )
      p += 6;                   /* skip (none) and (none) */
    else
    {
      if ( FT_NEXT_USHORT( p ) != 0 )
        FT_INVALID_DATA;

      if ( FT_NEXT_ULONG( p )  != 0 )
        FT_INVALID_DATA;
    }

    feat->reserved_size += featureNameCount * ( 2 + 2 + 4 + 2 + 2 );

    for ( last_feature = -1, i = 0; i < featureNameCount; i++ )
    {
      gxv_feat_name_validate( p, limit, valid );

      if ( valid->root->level == FT_VALIDATE_PARANOID       &&
           (FT_Int)GXV_FEAT_DATA( feature ) <= last_feature )
        FT_INVALID_FORMAT;

      last_feature = GXV_FEAT_DATA( feature );
      p += 2 + 2 + 4 + 2 + 2;
    }

    FT_TRACE4(( "\n" ));
  }


/* END */
