#ifndef GC_CPP_H
#define GC_CPP_H
/****************************************************************************
Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 
THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 
Permission is hereby granted to use or copy this program for any
purpose, provided the above notices are retained on all copies.
Permission to modify the code and to distribute modified code is
granted, provided the above notices are retained, and a notice that
the code was modified is included with the above copyright notice.
****************************************************************************

C++ Interface to the Boehm Collector

    John R. Ellis and Jesse Hull 

This interface provides access to the Boehm collector.  It provides
basic facilities similar to those described in "Safe, Efficient
Garbage Collection for C++", by John R. Elis and David L. Detlefs
(ftp://ftp.parc.xerox.com/pub/ellis/gc).

All heap-allocated objects are either "collectable" or
"uncollectable".  Programs must explicitly delete uncollectable
objects, whereas the garbage collector will automatically delete
collectable objects when it discovers them to be inaccessible.
Collectable objects may freely point at uncollectable objects and vice
versa.

Objects allocated with the built-in "::operator new" are uncollectable.

Objects derived from class "gc" are collectable.  For example:

    class A: public gc {...};
    A* a = new A;       // a is collectable. 

Collectable instances of non-class types can be allocated using the GC
(or UseGC) placement:

    typedef int A[ 10 ];
    A* a = new (GC) A;

Uncollectable instances of classes derived from "gc" can be allocated
using the NoGC placement:

    class A: public gc {...};
    A* a = new (NoGC) A;   // a is uncollectable.

The new(PointerFreeGC) syntax allows the allocation of collectable
objects that are not scanned by the collector.  This useful if you
are allocating compressed data, bitmaps, or network packets.  (In
the latter case, it may remove danger of unfriendly network packets
intentionally containing values that cause spurious memory retention.)

Both uncollectable and collectable objects can be explicitly deleted
with "delete", which invokes an object's destructors and frees its
storage immediately.

A collectable object may have a clean-up function, which will be
invoked when the collector discovers the object to be inaccessible.
An object derived from "gc_cleanup" or containing a member derived
from "gc_cleanup" has a default clean-up function that invokes the
object's destructors.  Explicit clean-up functions may be specified as
an additional placement argument:

    A* a = ::new (GC, MyCleanup) A;

An object is considered "accessible" by the collector if it can be
reached by a path of pointers from static variables, automatic
variables of active functions, or from some object with clean-up
enabled; pointers from an object to itself are ignored.

Thus, if objects A and B both have clean-up functions, and A points at
B, B is considered accessible.  After A's clean-up is invoked and its
storage released, B will then become inaccessible and will have its
clean-up invoked.  If A points at B and B points to A, forming a
cycle, then that's considered a storage leak, and neither will be
collectable.  See the interface gc.h for low-level facilities for
handling such cycles of objects with clean-up.

The collector cannot guarantee that it will find all inaccessible
objects.  In practice, it finds almost all of them.


Cautions:

1. Be sure the collector has been augmented with "make c++" or
"--enable-cplusplus".

2.  If your compiler supports the new "operator new[]" syntax, then
add -DGC_OPERATOR_NEW_ARRAY to the Makefile.

If your compiler doesn't support "operator new[]", beware that an
array of type T, where T is derived from "gc", may or may not be
allocated as a collectable object (it depends on the compiler).  Use
the explicit GC placement to make the array collectable.  For example:

    class A: public gc {...};
    A* a1 = new A[ 10 ];        // collectable or uncollectable?
    A* a2 = new (GC) A[ 10 ];   // collectable

3. The destructors of collectable arrays of objects derived from
"gc_cleanup" will not be invoked properly.  For example:

    class A: public gc_cleanup {...};
    A* a = new (GC) A[ 10 ];    // destructors not invoked correctly

Typically, only the destructor for the first element of the array will
be invoked when the array is garbage-collected.  To get all the
destructors of any array executed, you must supply an explicit
clean-up function:

    A* a = new (GC, MyCleanUp) A[ 10 ];

(Implementing clean-up of arrays correctly, portably, and in a way
that preserves the correct exception semantics requires a language
extension, e.g. the "gc" keyword.)

4. Compiler bugs (now hopefully history):

* Solaris 2's CC (SC3.0) doesn't implement t->~T() correctly, so the
destructors of classes derived from gc_cleanup won't be invoked.
You'll have to explicitly register a clean-up function with
new-placement syntax.

* Evidently cfront 3.0 does not allow destructors to be explicitly
invoked using the ANSI-conforming syntax t->~T().  If you're using
cfront 3.0, you'll have to comment out the class gc_cleanup, which
uses explicit invocation.

5. GC name conflicts:

Many other systems seem to use the identifier "GC" as an abbreviation
for "Graphics Context".  Since version 5.0, GC placement has been replaced
by UseGC.  GC is an alias for UseGC, unless GC_NAME_CONFLICT is defined.

****************************************************************************/

#include "gc.h"

#ifndef THINK_CPLUS
#  define GC_cdecl
#else
#  define GC_cdecl _cdecl
#endif

#if ! defined( GC_NO_OPERATOR_NEW_ARRAY ) \
    && !defined(_ENABLE_ARRAYNEW) /* Digimars */ \
    && (defined(__BORLANDC__) && (__BORLANDC__ < 0x450) \
	|| (defined(__GNUC__) && \
	    (__GNUC__ < 2 || __GNUC__ == 2 && __GNUC_MINOR__ < 6)) \
	|| (defined(__WATCOMC__) && __WATCOMC__ < 1050))
#   define GC_NO_OPERATOR_NEW_ARRAY
#endif

#if !defined(GC_NO_OPERATOR_NEW_ARRAY) && !defined(GC_OPERATOR_NEW_ARRAY)
#   define GC_OPERATOR_NEW_ARRAY
#endif

#if    ! defined ( __BORLANDC__ )  /* Confuses the Borland compiler. */ \
    && ! defined ( __sgi )
#  define GC_PLACEMENT_DELETE
#endif

enum GCPlacement {UseGC,
#ifndef GC_NAME_CONFLICT
		  GC=UseGC,
#endif
                  NoGC, PointerFreeGC};

class gc {public:
    inline void* operator new( size_t size );
    inline void* operator new( size_t size, GCPlacement gcp );
    inline void* operator new( size_t size, void *p );
    	/* Must be redefined here, since the other overloadings	*/
    	/* hide the global definition.				*/
    inline void operator delete( void* obj );
#   ifdef GC_PLACEMENT_DELETE
      inline void operator delete( void*, GCPlacement );
      	/* called if construction fails.	*/
      inline void operator delete( void*, void* );
#   endif

#ifdef GC_OPERATOR_NEW_ARRAY
    inline void* operator new[]( size_t size );
    inline void* operator new[]( size_t size, GCPlacement gcp );
    inline void* operator new[]( size_t size, void *p );
    inline void operator delete[]( void* obj );
#   ifdef GC_PLACEMENT_DELETE
      inline void operator delete[]( void*, GCPlacement );
      inline void operator delete[]( void*, void* );
#   endif
#endif /* GC_OPERATOR_NEW_ARRAY */
    };    
    /*
    Instances of classes derived from "gc" will be allocated in the 
    collected heap by default, unless an explicit NoGC placement is
    specified. */
/*
class gc_cleanup: virtual public gc {public:
    inline gc_cleanup();
    inline virtual ~gc_cleanup();
private:
    inline static void GC_cdecl cleanup( void* obj, void* clientData );};*/
    /*
    Instances of classes derived from "gc_cleanup" will be allocated
    in the collected heap by default.  When the collector discovers an
    inaccessible object derived from "gc_cleanup" or containing a
    member derived from "gc_cleanup", its destructors will be
    invoked. */

extern "C" {typedef void (*GCCleanUpFunc)( void* obj, void* clientData );}

#ifdef _MSC_VER
  // Disable warning that "no matching operator delete found; memory will
  // not be freed if initialization throws an exception"
# pragma warning(disable:4291)
#endif

inline void* operator new( 
    size_t size, 
    GCPlacement gcp,
    GCCleanUpFunc cleanup = 0,
    void* clientData = 0 );
    /*
    Allocates a collectable or uncollected object, according to the
    value of "gcp".

    For collectable objects, if "cleanup" is non-null, then when the
    allocated object "obj" becomes inaccessible, the collector will
    invoke the function "cleanup( obj, clientData )" but will not
    invoke the object's destructors.  It is an error to explicitly
    delete an object allocated with a non-null "cleanup".

    It is an error to specify a non-null "cleanup" with NoGC or for
    classes derived from "gc_cleanup" or containing members derived
    from "gc_cleanup". */

#   ifdef GC_PLACEMENT_DELETE
      inline void operator delete( void*, GCPlacement, GCCleanUpFunc, void * );
#   endif

#ifdef _MSC_VER
 /** This ensures that the system default operator new[] doesn't get
  *  undefined, which is what seems to happen on VC++ 6 for some reason
  *  if we define a multi-argument operator new[].
  *  There seems to be no way to redirect new in this environment without
  *  including this everywhere. 
  */
 void *operator new[]( size_t size );
 
 void operator delete[](void* obj);

 void* operator new( size_t size);

 void operator delete(void* obj);

 // This new operator is used by VC++ in case of Debug builds !
 void* operator new(  size_t size,
		      int ,//nBlockUse,
		      const char * szFileName,
		      int nLine );
#endif /* _MSC_VER */


#ifdef GC_OPERATOR_NEW_ARRAY

inline void* operator new[](
    size_t size, 
    GCPlacement gcp,
    GCCleanUpFunc cleanup = 0,
    void* clientData = 0 );
    /*
    The operator new for arrays, identical to the above. */

#endif /* GC_OPERATOR_NEW_ARRAY */

/****************************************************************************

Inline implementation

****************************************************************************/

inline void* gc::operator new( size_t size ) {
    return GC_MALLOC( size );}
    
inline void* gc::operator new( size_t size, GCPlacement gcp ) {
    if (gcp == UseGC) 
        return GC_MALLOC( size );
    else if (gcp == PointerFreeGC)
	return GC_MALLOC_ATOMIC( size );
    else
        return GC_MALLOC_UNCOLLECTABLE( size );}

inline void* gc::operator new( size_t size, void *p ) {
    return p;}

inline void gc::operator delete( void* obj ) {
    GC_FREE( obj );}
    
#ifdef GC_PLACEMENT_DELETE
  inline void gc::operator delete( void*, void* ) {}

  inline void gc::operator delete( void* p, GCPlacement gcp ) {
    GC_FREE(p);
  }
#endif

#ifdef GC_OPERATOR_NEW_ARRAY

inline void* gc::operator new[]( size_t size ) {
    return gc::operator new( size );}
    
inline void* gc::operator new[]( size_t size, GCPlacement gcp ) {
    return gc::operator new( size, gcp );}

inline void* gc::operator new[]( size_t size, void *p ) {
    return p;}

inline void gc::operator delete[]( void* obj ) {
    gc::operator delete( obj );}

#ifdef GC_PLACEMENT_DELETE
  inline void gc::operator delete[]( void*, void* ) {}

  inline void gc::operator delete[]( void* p, GCPlacement gcp ) {
    gc::operator delete(p); }

#endif
    
#endif /* GC_OPERATOR_NEW_ARRAY */

/*
inline gc_cleanup::~gc_cleanup() {
    GC_register_finalizer_ignore_self( GC_base(this), 0, 0, 0, 0 );}

inline void gc_cleanup::cleanup( void* obj, void* displ ) {
    ((gc_cleanup*) ((char*) obj + (ptrdiff_t) displ))->~gc_cleanup();}

inline gc_cleanup::gc_cleanup() {
    GC_finalization_proc oldProc;
    void* oldData;
    void* base = GC_base( (void *) this );
    if (0 != base)  {
      // Don't call the debug version, since this is a real base address.
      GC_register_finalizer_ignore_self( 
        base, (GC_finalization_proc)cleanup, (void*) ((char*) this - (char*) base), 
        &oldProc, &oldData );
      if (0 != oldProc) {
        GC_register_finalizer_ignore_self( base, oldProc, oldData, 0, 0 );}}}
        */

inline void* operator new( 
    size_t size, 
    GCPlacement gcp,
    GCCleanUpFunc cleanup,
    void* clientData )
{
    void* obj;

    if (gcp == UseGC) {
        obj = GC_MALLOC( size );
      /* if (cleanup != 0) 
            GC_REGISTER_FINALIZER_IGNORE_SELF( 
                obj, cleanup, clientData, 0, 0 );*/}
    else if (gcp == PointerFreeGC) {
        obj = GC_MALLOC_ATOMIC( size );}
    else {
        obj = GC_MALLOC_UNCOLLECTABLE( size );};
    return obj;}
        
# ifdef GC_PLACEMENT_DELETE
inline void operator delete ( 
    void *p, 
    GCPlacement gcp,
    GCCleanUpFunc cleanup,
    void* clientData )
{
    GC_FREE(p);
}
# endif

#ifdef GC_OPERATOR_NEW_ARRAY

inline void* operator new[]( 
    size_t size, 
    GCPlacement gcp,
    GCCleanUpFunc cleanup,
    void* clientData )
{
    return ::operator new( size, gcp, cleanup, clientData );}

#endif /* GC_OPERATOR_NEW_ARRAY */


#endif /* GC_CPP_H */

