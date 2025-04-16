/***************************************************************************/
/*                                                                         */
/*  ftmodapi.h                                                             */
/*                                                                         */
/*    FreeType modules public interface (specification).                   */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2006, 2008, 2009, 2010 by             */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTMODAPI_H__
#define __FTMODAPI_H__


#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    module_management                                                  */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Module Management                                                  */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    How to add, upgrade, and remove modules from FreeType.             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The definitions below are used to manage modules within FreeType.  */
  /*    Modules can be added, upgraded, and removed at runtime.            */
  /*                                                                       */
  /*************************************************************************/


  /* module bit flags */
#define FT_MODULE_FONT_DRIVER         1  /* this module is a font driver  */
#define FT_MODULE_RENDERER            2  /* this module is a renderer     */
#define FT_MODULE_HINTER              4  /* this module is a glyph hinter */
#define FT_MODULE_STYLER              8  /* this module is a styler       */

#define FT_MODULE_DRIVER_SCALABLE     0x100   /* the driver supports      */
                                              /* scalable fonts           */
#define FT_MODULE_DRIVER_NO_OUTLINES  0x200   /* the driver does not      */
                                              /* support vector outlines  */
#define FT_MODULE_DRIVER_HAS_HINTER   0x400   /* the driver provides its  */
                                              /* own hinter               */


  /* deprecated values */
#define ft_module_font_driver         FT_MODULE_FONT_DRIVER
#define ft_module_renderer            FT_MODULE_RENDERER
#define ft_module_hinter              FT_MODULE_HINTER
#define ft_module_styler              FT_MODULE_STYLER

#define ft_module_driver_scalable     FT_MODULE_DRIVER_SCALABLE
#define ft_module_driver_no_outlines  FT_MODULE_DRIVER_NO_OUTLINES
#define ft_module_driver_has_hinter   FT_MODULE_DRIVER_HAS_HINTER


  typedef FT_Pointer  FT_Module_Interface;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Constructor                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to initialize (not create) a new module object.    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to initialize.                                */
  /*                                                                       */
  typedef FT_Error
  (*FT_Module_Constructor)( FT_Module  module );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Destructor                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to finalize (not destroy) a given module object.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to finalize.                                  */
  /*                                                                       */
  typedef void
  (*FT_Module_Destructor)( FT_Module  module );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Requester                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to query a given module for a specific interface.  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to finalize.                                  */
  /*                                                                       */
  /*    name ::   The name of the interface in the module.                 */
  /*                                                                       */
  typedef FT_Module_Interface
  (*FT_Module_Requester)( FT_Module    module,
                          const char*  name );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Module_Class                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The module class descriptor.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    module_flags    :: Bit flags describing the module.                */
  /*                                                                       */
  /*    module_size     :: The size of one module object/instance in       */
  /*                       bytes.                                          */
  /*                                                                       */
  /*    module_name     :: The name of the module.                         */
  /*                                                                       */
  /*    module_version  :: The version, as a 16.16 fixed number            */
  /*                       (major.minor).                                  */
  /*                                                                       */
  /*    module_requires :: The version of FreeType this module requires,   */
  /*                       as a 16.16 fixed number (major.minor).  Starts  */
  /*                       at version 2.0, i.e., 0x20000.                  */
  /*                                                                       */
  /*    module_init     :: The initializing function.                      */
  /*                                                                       */
  /*    module_done     :: The finalizing function.                        */
  /*                                                                       */
  /*    get_interface   :: The interface requesting function.              */
  /*                                                                       */
  typedef struct  FT_Module_Class_
  {
    FT_ULong               module_flags;
    FT_Long                module_size;
    const FT_String*       module_name;
    FT_Fixed               module_version;
    FT_Fixed               module_requires;

    const void*            module_interface;

    FT_Module_Constructor  module_init;
    FT_Module_Destructor   module_done;
    FT_Module_Requester    get_interface;

  } FT_Module_Class;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Add_Module                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Add a new module to a given library instance.                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to the library object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    clazz   :: A pointer to class descriptor for the module.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    An error will be returned if a module already exists by that name, */
  /*    or if the module requires a version of FreeType that is too great. */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Add_Module( FT_Library              library,
                 const FT_Module_Class*  clazz );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Module                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Find a module by its name.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library     :: A handle to the library object.                     */
  /*                                                                       */
  /*    module_name :: The module's name (as an ASCII string).             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A module handle.  0~if none was found.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    FreeType's internal modules aren't documented very well, and you   */
  /*    should look up the source code for details.                        */
  /*                                                                       */
  FT_EXPORT( FT_Module )
  FT_Get_Module( FT_Library   library,
                 const char*  module_name );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Remove_Module                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Remove a given module from a library instance.                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to a library object.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module  :: A handle to a module object.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The module object is destroyed by the function in case of success. */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Remove_Module( FT_Library  library,
                    FT_Module   module );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Reference_Library                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A counter gets initialized to~1 at the time an @FT_Library         */
  /*    structure is created.  This function increments the counter.       */
  /*    @FT_Done_Library then only destroys a library if the counter is~1, */
  /*    otherwise it simply decrements the counter.                        */
  /*                                                                       */
  /*    This function helps in managing life-cycles of structures which    */
  /*    reference @FT_Library objects.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a target library object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.4.2                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Reference_Library( FT_Library  library );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Library                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to create a new FreeType library instance    */
  /*    from a given memory object.  It is thus possible to use libraries  */
  /*    with distinct memory allocators within the same program.           */
  /*                                                                       */
  /*    Normally, you would call this function (followed by a call to      */
  /*    @FT_Add_Default_Modules or a series of calls to @FT_Add_Module)    */
  /*    instead of @FT_Init_FreeType to initialize the FreeType library.   */
  /*                                                                       */
  /*    Don't use @FT_Done_FreeType but @FT_Done_Library to destroy a      */
  /*    library instance.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory   :: A handle to the original memory object.                */
  /*                                                                       */
  /* <Output>                                                              */
  /*    alibrary :: A pointer to handle of a new library object.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    See the discussion of reference counters in the description of     */
  /*    @FT_Reference_Library.                                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_New_Library( FT_Memory    memory,
                  FT_Library  *alibrary );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Library                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discard a given library object.  This closes all drivers and       */
  /*    discards all resource objects.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to the target library.                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    See the discussion of reference counters in the description of     */
  /*    @FT_Reference_Library.                                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Done_Library( FT_Library  library );

/* */

  typedef void
  (*FT_DebugHook_Func)( void*  arg );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Debug_Hook                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Set a debug hook function for debugging the interpreter of a font  */
  /*    format.                                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    hook_index :: The index of the debug hook.  You should use the     */
  /*                  values defined in `ftobjs.h', e.g.,                  */
  /*                  `FT_DEBUG_HOOK_TRUETYPE'.                            */
  /*                                                                       */
  /*    debug_hook :: The function used to debug the interpreter.          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Currently, four debug hook slots are available, but only two (for  */
  /*    the TrueType and the Type~1 interpreter) are defined.              */
  /*                                                                       */
  /*    Since the internal headers of FreeType are no longer installed,    */
  /*    the symbol `FT_DEBUG_HOOK_TRUETYPE' isn't available publicly.      */
  /*    This is a bug and will be fixed in a forthcoming release.          */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Set_Debug_Hook( FT_Library         library,
                     FT_UInt            hook_index,
                     FT_DebugHook_Func  debug_hook );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Add_Default_Modules                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Add the set of default drivers to a given library object.          */
  /*    This is only useful when you create a library object with          */
  /*    @FT_New_Library (usually to plug a custom memory manager).         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Add_Default_Modules( FT_Library  library );



  /**************************************************************************
   *
   * @section:
   *   truetype_engine
   *
   * @title:
   *   The TrueType Engine
   *
   * @abstract:
   *   TrueType bytecode support.
   *
   * @description:
   *   This section contains a function used to query the level of TrueType
   *   bytecode support compiled in this version of the library.
   *
   */


  /**************************************************************************
   *
   *  @enum:
   *     FT_TrueTypeEngineType
   *
   *  @description:
   *     A list of values describing which kind of TrueType bytecode
   *     engine is implemented in a given FT_Library instance.  It is used
   *     by the @FT_Get_TrueType_Engine_Type function.
   *
   *  @values:
   *     FT_TRUETYPE_ENGINE_TYPE_NONE ::
   *       The library doesn't implement any kind of bytecode interpreter.
   *
   *     FT_TRUETYPE_ENGINE_TYPE_UNPATENTED ::
   *       The library implements a bytecode interpreter that doesn't
   *       support the patented operations of the TrueType virtual machine.
   *
   *       Its main use is to load certain Asian fonts which position and
   *       scale glyph components with bytecode instructions.  It produces
   *       bad output for most other fonts.
   *
   *    FT_TRUETYPE_ENGINE_TYPE_PATENTED ::
   *       The library implements a bytecode interpreter that covers
   *       the full instruction set of the TrueType virtual machine (this
   *       was governed by patents until May 2010, hence the name).
   *
   *  @since:
   *       2.2
   *
   */
  typedef enum  FT_TrueTypeEngineType_
  {
    FT_TRUETYPE_ENGINE_TYPE_NONE = 0,
    FT_TRUETYPE_ENGINE_TYPE_UNPATENTED,
    FT_TRUETYPE_ENGINE_TYPE_PATENTED

  } FT_TrueTypeEngineType;


  /**************************************************************************
   *
   *  @func:
   *     FT_Get_TrueType_Engine_Type
   *
   *  @description:
   *     Return an @FT_TrueTypeEngineType value to indicate which level of
   *     the TrueType virtual machine a given library instance supports.
   *
   *  @input:
   *     library ::
   *       A library instance.
   *
   *  @return:
   *     A value indicating which level is supported.
   *
   *  @since:
   *     2.2
   *
   */
  FT_EXPORT( FT_TrueTypeEngineType )
  FT_Get_TrueType_Engine_Type( FT_Library  library );


  /* */


FT_END_HEADER

#endif /* __FTMODAPI_H__ */


/* END */
