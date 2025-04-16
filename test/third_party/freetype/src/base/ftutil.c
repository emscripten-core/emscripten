/***************************************************************************/
/*                                                                         */
/*  ftutil.c                                                               */
/*                                                                         */
/*    FreeType utility file for memory and list management (body).         */
/*                                                                         */
/*  Copyright 2002, 2004, 2005, 2006, 2007 by                              */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_MEMORY_H
#include FT_INTERNAL_OBJECTS_H
#include FT_LIST_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_memory


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****               M E M O R Y   M A N A G E M E N T               *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  FT_BASE_DEF( FT_Pointer )
  ft_mem_alloc( FT_Memory  memory,
                FT_Long    size,
                FT_Error  *p_error )
  {
    FT_Error    error;
    FT_Pointer  block = ft_mem_qalloc( memory, size, &error );

    if ( !error && size > 0 )
      FT_MEM_ZERO( block, size );

    *p_error = error;
    return block;
  }


  FT_BASE_DEF( FT_Pointer )
  ft_mem_qalloc( FT_Memory  memory,
                 FT_Long    size,
                 FT_Error  *p_error )
  {
    FT_Error    error = FT_Err_Ok;
    FT_Pointer  block = NULL;


    if ( size > 0 )
    {
      block = memory->alloc( memory, size );
      if ( block == NULL )
        error = FT_Err_Out_Of_Memory;
    }
    else if ( size < 0 )
    {
      /* may help catch/prevent security issues */
      error = FT_Err_Invalid_Argument;
    }

    *p_error = error;
    return block;
  }


  FT_BASE_DEF( FT_Pointer )
  ft_mem_realloc( FT_Memory  memory,
                  FT_Long    item_size,
                  FT_Long    cur_count,
                  FT_Long    new_count,
                  void*      block,
                  FT_Error  *p_error )
  {
    FT_Error  error = FT_Err_Ok;

    block = ft_mem_qrealloc( memory, item_size,
                             cur_count, new_count, block, &error );
    if ( !error && new_count > cur_count )
      FT_MEM_ZERO( (char*)block + cur_count * item_size,
                   ( new_count - cur_count ) * item_size );

    *p_error = error;
    return block;
  }


  FT_BASE_DEF( FT_Pointer )
  ft_mem_qrealloc( FT_Memory  memory,
                   FT_Long    item_size,
                   FT_Long    cur_count,
                   FT_Long    new_count,
                   void*      block,
                   FT_Error  *p_error )
  {
    FT_Error  error = FT_Err_Ok;


    /* Note that we now accept `item_size == 0' as a valid parameter, in
     * order to cover very weird cases where an ALLOC_MULT macro would be
     * called.
     */
    if ( cur_count < 0 || new_count < 0 || item_size < 0 )
    {
      /* may help catch/prevent nasty security issues */
      error = FT_Err_Invalid_Argument;
    }
    else if ( new_count == 0 || item_size == 0 )
    {
      ft_mem_free( memory, block );
      block = NULL;
    }
    else if ( new_count > FT_INT_MAX/item_size )
    {
      error = FT_Err_Array_Too_Large;
    }
    else if ( cur_count == 0 )
    {
      FT_ASSERT( block == NULL );

      block = ft_mem_alloc( memory, new_count*item_size, &error );
    }
    else
    {
      FT_Pointer  block2;
      FT_Long     cur_size = cur_count*item_size;
      FT_Long     new_size = new_count*item_size;


      block2 = memory->realloc( memory, cur_size, new_size, block );
      if ( block2 == NULL )
        error = FT_Err_Out_Of_Memory;
      else
        block = block2;
    }

    *p_error = error;
    return block;
  }


  FT_BASE_DEF( void )
  ft_mem_free( FT_Memory   memory,
               const void *P )
  {
    if ( P )
      memory->free( memory, (void*)P );
  }


  FT_BASE_DEF( FT_Pointer )
  ft_mem_dup( FT_Memory    memory,
              const void*  address,
              FT_ULong     size,
              FT_Error    *p_error )
  {
    FT_Error    error;
    FT_Pointer  p = ft_mem_qalloc( memory, size, &error );


    if ( !error && address )
      ft_memcpy( p, address, size );

    *p_error = error;
    return p;
  }


  FT_BASE_DEF( FT_Pointer )
  ft_mem_strdup( FT_Memory    memory,
                 const char*  str,
                 FT_Error    *p_error )
  {
    FT_ULong  len = str ? (FT_ULong)ft_strlen( str ) + 1
                        : 0;


    return ft_mem_dup( memory, str, len, p_error );
  }


  FT_BASE_DEF( FT_Int )
  ft_mem_strcpyn( char*        dst,
                  const char*  src,
                  FT_ULong     size )
  {
    while ( size > 1 && *src != 0 )
    {
      *dst++ = *src++;
      size--;
    }

    *dst = 0;  /* always zero-terminate */

    return *src != 0;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****            D O U B L Y   L I N K E D   L I S T S              *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_list

  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( FT_ListNode )
  FT_List_Find( FT_List  list,
                void*    data )
  {
    FT_ListNode  cur;


    cur = list->head;
    while ( cur )
    {
      if ( cur->data == data )
        return cur;

      cur = cur->next;
    }

    return (FT_ListNode)0;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Add( FT_List      list,
               FT_ListNode  node )
  {
    FT_ListNode  before = list->tail;


    node->next = 0;
    node->prev = before;

    if ( before )
      before->next = node;
    else
      list->head = node;

    list->tail = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Insert( FT_List      list,
                  FT_ListNode  node )
  {
    FT_ListNode  after = list->head;


    node->next = after;
    node->prev = 0;

    if ( !after )
      list->tail = node;
    else
      after->prev = node;

    list->head = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Remove( FT_List      list,
                  FT_ListNode  node )
  {
    FT_ListNode  before, after;


    before = node->prev;
    after  = node->next;

    if ( before )
      before->next = after;
    else
      list->head = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Up( FT_List      list,
              FT_ListNode  node )
  {
    FT_ListNode  before, after;


    before = node->prev;
    after  = node->next;

    /* check whether we are already on top of the list */
    if ( !before )
      return;

    before->next = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;

    node->prev       = 0;
    node->next       = list->head;
    list->head->prev = node;
    list->head       = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( FT_Error )
  FT_List_Iterate( FT_List            list,
                   FT_List_Iterator   iterator,
                   void*              user )
  {
    FT_ListNode  cur   = list->head;
    FT_Error     error = FT_Err_Ok;


    while ( cur )
    {
      FT_ListNode  next = cur->next;


      error = iterator( cur, user );
      if ( error )
        break;

      cur = next;
    }

    return error;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Finalize( FT_List             list,
                    FT_List_Destructor  destroy,
                    FT_Memory           memory,
                    void*               user )
  {
    FT_ListNode  cur;


    cur = list->head;
    while ( cur )
    {
      FT_ListNode  next = cur->next;
      void*        data = cur->data;


      if ( destroy )
        destroy( memory, data, user );

      FT_FREE( cur );
      cur = next;
    }

    list->head = 0;
    list->tail = 0;
  }


  FT_BASE_DEF( FT_UInt32 )
  ft_highpow2( FT_UInt32  value )
  {
    FT_UInt32  value2;


    /*
     *  We simply clear the lowest bit in each iteration.  When
     *  we reach 0, we know that the previous value was our result.
     */
    for ( ;; )
    {
      value2 = value & (value - 1);  /* clear lowest bit */
      if ( value2 == 0 )
        break;

      value = value2;
    }
    return value;
  }


#ifdef FT_CONFIG_OPTION_OLD_INTERNALS

  FT_BASE_DEF( FT_Error )
  FT_Alloc( FT_Memory  memory,
            FT_Long    size,
            void*     *P )
  {
    FT_Error  error;


    (void)FT_ALLOC( *P, size );
    return error;
  }


  FT_BASE_DEF( FT_Error )
  FT_QAlloc( FT_Memory  memory,
             FT_Long    size,
             void*     *p )
  {
    FT_Error  error;


    (void)FT_QALLOC( *p, size );
    return error;
  }


  FT_BASE_DEF( FT_Error )
  FT_Realloc( FT_Memory  memory,
              FT_Long    current,
              FT_Long    size,
              void*     *P )
  {
    FT_Error  error;


    (void)FT_REALLOC( *P, current, size );
    return error;
  }


  FT_BASE_DEF( FT_Error )
  FT_QRealloc( FT_Memory  memory,
               FT_Long    current,
               FT_Long    size,
               void*     *p )
  {
    FT_Error  error;


    (void)FT_QREALLOC( *p, current, size );
    return error;
  }


  FT_BASE_DEF( void )
  FT_Free( FT_Memory  memory,
           void*     *P )
  {
    if ( *P )
      FT_MEM_FREE( *P );
  }

#endif /* FT_CONFIG_OPTION_OLD_INTERNALS */

/* END */
