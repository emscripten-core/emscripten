/***************************************************************************/
/*                                                                         */
/*  ftccache.h                                                             */
/*                                                                         */
/*    FreeType internal cache interface (specification).                   */
/*                                                                         */
/*  Copyright 2000-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010 by */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCCACHE_H__
#define __FTCCACHE_H__


#include "ftcmru.h"

FT_BEGIN_HEADER

#define _FTC_FACE_ID_HASH( i )                              \
          ((FT_PtrDist)(( (FT_PtrDist)(i) >> 3 ) ^ ( (FT_PtrDist)(i) << 7 )))

  /* handle to cache object */
  typedef struct FTC_CacheRec_*  FTC_Cache;

  /* handle to cache class */
  typedef const struct FTC_CacheClassRec_*  FTC_CacheClass;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                   CACHE NODE DEFINITIONS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Each cache controls one or more cache nodes.  Each node is part of    */
  /* the global_lru list of the manager.  Its `data' field however is used */
  /* as a reference count for now.                                         */
  /*                                                                       */
  /* A node can be anything, depending on the type of information held by  */
  /* the cache.  It can be an individual glyph image, a set of bitmaps     */
  /* glyphs for a given size, some metrics, etc.                           */
  /*                                                                       */
  /*************************************************************************/

  /* structure size should be 20 bytes on 32-bits machines */
  typedef struct  FTC_NodeRec_
  {
    FTC_MruNodeRec  mru;          /* circular mru list pointer           */
    FTC_Node        link;         /* used for hashing                    */
    FT_PtrDist      hash;         /* used for hashing too                */
    FT_UShort       cache_index;  /* index of cache the node belongs to  */
    FT_Short        ref_count;    /* reference count for this node       */

  } FTC_NodeRec;


#define FTC_NODE( x )    ( (FTC_Node)(x) )
#define FTC_NODE_P( x )  ( (FTC_Node*)(x) )

#define FTC_NODE__NEXT( x )  FTC_NODE( (x)->mru.next )
#define FTC_NODE__PREV( x )  FTC_NODE( (x)->mru.prev )


#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
  FT_BASE( void )
  ftc_node_destroy( FTC_Node     node,
                    FTC_Manager  manager );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       CACHE DEFINITIONS                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* initialize a new cache node */
  typedef FT_Error
  (*FTC_Node_NewFunc)( FTC_Node    *pnode,
                       FT_Pointer   query,
                       FTC_Cache    cache );

  typedef FT_Offset
  (*FTC_Node_WeightFunc)( FTC_Node   node,
                          FTC_Cache  cache );

  /* compare a node to a given key pair */
  typedef FT_Bool
  (*FTC_Node_CompareFunc)( FTC_Node    node,
                           FT_Pointer  key,
                           FTC_Cache   cache );


  typedef void
  (*FTC_Node_FreeFunc)( FTC_Node   node,
                        FTC_Cache  cache );

  typedef FT_Error
  (*FTC_Cache_InitFunc)( FTC_Cache  cache );

  typedef void
  (*FTC_Cache_DoneFunc)( FTC_Cache  cache );


  typedef struct  FTC_CacheClassRec_
  {
    FTC_Node_NewFunc      node_new;
    FTC_Node_WeightFunc   node_weight;
    FTC_Node_CompareFunc  node_compare;
    FTC_Node_CompareFunc  node_remove_faceid;
    FTC_Node_FreeFunc     node_free;

    FT_Offset             cache_size;
    FTC_Cache_InitFunc    cache_init;
    FTC_Cache_DoneFunc    cache_done;

  } FTC_CacheClassRec;


  /* each cache really implements a dynamic hash table to manage its nodes */
  typedef struct  FTC_CacheRec_
  {
    FT_UFast           p;
    FT_UFast           mask;
    FT_Long            slack;
    FTC_Node*          buckets;

    FTC_CacheClassRec  clazz;       /* local copy, for speed  */

    FTC_Manager        manager;
    FT_Memory          memory;
    FT_UInt            index;       /* in manager's table     */

    FTC_CacheClass     org_class;   /* original class pointer */

  } FTC_CacheRec;


#define FTC_CACHE( x )    ( (FTC_Cache)(x) )
#define FTC_CACHE_P( x )  ( (FTC_Cache*)(x) )


  /* default cache initialize */
  FT_LOCAL( FT_Error )
  FTC_Cache_Init( FTC_Cache  cache );

  /* default cache finalizer */
  FT_LOCAL( void )
  FTC_Cache_Done( FTC_Cache  cache );

  /* Call this function to lookup the cache.  If no corresponding
   * node is found, a new one is automatically created.  This function
   * is capable of flushing the cache adequately to make room for the
   * new cache object.
   */

#ifndef FTC_INLINE
  FT_LOCAL( FT_Error )
  FTC_Cache_Lookup( FTC_Cache   cache,
                    FT_PtrDist  hash,
                    FT_Pointer  query,
                    FTC_Node   *anode );
#endif

  FT_LOCAL( FT_Error )
  FTC_Cache_NewNode( FTC_Cache   cache,
                     FT_PtrDist  hash,
                     FT_Pointer  query,
                     FTC_Node   *anode );

  /* Remove all nodes that relate to a given face_id.  This is useful
   * when un-installing fonts.  Note that if a cache node relates to
   * the face_id, but is locked (i.e., has `ref_count > 0'), the node
   * will _not_ be destroyed, but its internal face_id reference will
   * be modified.
   *
   * The final result will be that the node will never come back
   * in further lookup requests, and will be flushed on demand from
   * the cache normally when its reference count reaches 0.
   */
  FT_LOCAL( void )
  FTC_Cache_RemoveFaceID( FTC_Cache   cache,
                          FTC_FaceID  face_id );


#ifdef FTC_INLINE

#define FTC_CACHE_LOOKUP_CMP( cache, nodecmp, hash, query, node, error ) \
  FT_BEGIN_STMNT                                                         \
    FTC_Node             *_bucket, *_pnode, _node;                       \
    FTC_Cache             _cache   = FTC_CACHE(cache);                   \
    FT_PtrDist            _hash    = (FT_PtrDist)(hash);                 \
    FTC_Node_CompareFunc  _nodcomp = (FTC_Node_CompareFunc)(nodecmp);    \
    FT_UFast              _idx;                                          \
                                                                         \
                                                                         \
    error = FTC_Err_Ok;                                                  \
    node  = NULL;                                                        \
    _idx  = _hash & _cache->mask;                                        \
    if ( _idx < _cache->p )                                              \
      _idx = _hash & ( _cache->mask*2 + 1 );                             \
                                                                         \
    _bucket = _pnode = _cache->buckets + _idx;                           \
    for (;;)                                                             \
    {                                                                    \
      _node = *_pnode;                                                   \
      if ( _node == NULL )                                               \
        goto _NewNode;                                                   \
                                                                         \
      if ( _node->hash == _hash && _nodcomp( _node, query, _cache ) )    \
        break;                                                           \
                                                                         \
      _pnode = &_node->link;                                             \
    }                                                                    \
                                                                         \
    if ( _node != *_bucket )                                             \
    {                                                                    \
      *_pnode     = _node->link;                                         \
      _node->link = *_bucket;                                            \
      *_bucket    = _node;                                               \
    }                                                                    \
                                                                         \
    {                                                                    \
      FTC_Manager  _manager = _cache->manager;                           \
      void*        _nl      = &_manager->nodes_list;                     \
                                                                         \
                                                                         \
      if ( _node != _manager->nodes_list )                               \
        FTC_MruNode_Up( (FTC_MruNode*)_nl,                               \
                        (FTC_MruNode)_node );                            \
    }                                                                    \
    goto _Ok;                                                            \
                                                                         \
  _NewNode:                                                              \
    error = FTC_Cache_NewNode( _cache, _hash, query, &_node );           \
                                                                         \
  _Ok:                                                                   \
    node = _node;                                                        \
  FT_END_STMNT

#else /* !FTC_INLINE */

#define FTC_CACHE_LOOKUP_CMP( cache, nodecmp, hash, query, node, error ) \
  FT_BEGIN_STMNT                                                         \
    error = FTC_Cache_Lookup( FTC_CACHE( cache ), hash, query,           \
                              (FTC_Node*)&(node) );                      \
  FT_END_STMNT

#endif /* !FTC_INLINE */


  /*
   * This macro, together with FTC_CACHE_TRYLOOP_END, defines a retry
   * loop to flush the cache repeatedly in case of memory overflows.
   *
   * It is used when creating a new cache node, or within a lookup
   * that needs to allocate data (e.g., the sbit cache lookup).
   *
   * Example:
   *
   *   {
   *     FTC_CACHE_TRYLOOP( cache )
   *       error = load_data( ... );
   *     FTC_CACHE_TRYLOOP_END()
   *   }
   *
   */
#define FTC_CACHE_TRYLOOP( cache )                           \
  {                                                          \
    FTC_Manager  _try_manager = FTC_CACHE( cache )->manager; \
    FT_UInt      _try_count   = 4;                           \
                                                             \
                                                             \
    for (;;)                                                 \
    {                                                        \
      FT_UInt  _try_done;


#define FTC_CACHE_TRYLOOP_END()                                   \
      if ( !error || error != FTC_Err_Out_Of_Memory )             \
        break;                                                    \
                                                                  \
      _try_done = FTC_Manager_FlushN( _try_manager, _try_count ); \
      if ( _try_done == 0 )                                       \
        break;                                                    \
                                                                  \
      if ( _try_done == _try_count )                              \
      {                                                           \
        _try_count *= 2;                                          \
        if ( _try_count < _try_done              ||               \
            _try_count > _try_manager->num_nodes )                \
          _try_count = _try_manager->num_nodes;                   \
      }                                                           \
    }                                                             \
  }

 /* */

FT_END_HEADER


#endif /* __FTCCACHE_H__ */


/* END */
