/*
 * Copyright (c) 1996-1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 2002
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/*
 * This implements standard-conforming allocators that interact with
 * the garbage collector.  Gc_alloctor<T> allocates garbage-collectable
 * objects of type T.  Traceable_allocator<T> allocates objects that
 * are not temselves garbage collected, but are scanned by the
 * collector for pointers to collectable objects.  Traceable_alloc
 * should be used for explicitly managed STL containers that may
 * point to collectable objects.
 *
 * This code was derived from an earlier version of the GNU C++ standard
 * library, which itself was derived from the SGI STL implementation.
 */

#ifndef GC_ALLOCATOR_H

#define GC_ALLOCATOR_H

#include "gc.h"
#include <new> // for placement new

#if defined(__GNUC__)
#  define GC_ATTR_UNUSED __attribute__((unused))
#else
#  define GC_ATTR_UNUSED
#endif

/* First some helpers to allow us to dispatch on whether or not a type
 * is known to be pointerfree.
 * These are private, except that the client may invoke the
 * GC_DECLARE_PTRFREE macro.
 */

struct GC_true_type {};
struct GC_false_type {};

template <class GC_tp>
struct GC_type_traits {
  GC_false_type GC_is_ptr_free;
};

# define GC_DECLARE_PTRFREE(T) \
template<> struct GC_type_traits<T> { GC_true_type GC_is_ptr_free; }

GC_DECLARE_PTRFREE(char);
GC_DECLARE_PTRFREE(signed char);
GC_DECLARE_PTRFREE(unsigned char);
GC_DECLARE_PTRFREE(signed short);
GC_DECLARE_PTRFREE(unsigned short);
GC_DECLARE_PTRFREE(signed int);
GC_DECLARE_PTRFREE(unsigned int);
GC_DECLARE_PTRFREE(signed long);
GC_DECLARE_PTRFREE(unsigned long);
GC_DECLARE_PTRFREE(float);
GC_DECLARE_PTRFREE(double);
GC_DECLARE_PTRFREE(long double);
/* The client may want to add others.	*/

// In the following GC_Tp is GC_true_type iff we are allocating a
// pointerfree object.
template <class GC_Tp>
inline void * GC_selective_alloc(size_t n, GC_Tp) {
    return GC_MALLOC(n);
}

template <>
inline void * GC_selective_alloc<GC_true_type>(size_t n, GC_true_type) {
    return GC_MALLOC_ATOMIC(n);
}

/* Now the public gc_allocator<T> class:
 */
template <class GC_Tp>
class gc_allocator {
public:
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
  typedef GC_Tp*       pointer;
  typedef const GC_Tp* const_pointer;
  typedef GC_Tp&       reference;
  typedef const GC_Tp& const_reference;
  typedef GC_Tp        value_type;

  template <class GC_Tp1> struct rebind {
    typedef gc_allocator<GC_Tp1> other;
  };

  gc_allocator()  {}
    gc_allocator(const gc_allocator&) throw() {}
# if !(GC_NO_MEMBER_TEMPLATES || 0 < _MSC_VER && _MSC_VER <= 1200)
  // MSVC++ 6.0 do not support member templates
  template <class GC_Tp1> gc_allocator(const gc_allocator<GC_Tp1>&) throw() {}
# endif
  ~gc_allocator() throw() {}

  pointer address(reference GC_x) const { return &GC_x; }
  const_pointer address(const_reference GC_x) const { return &GC_x; }

  // GC_n is permitted to be 0.  The C++ standard says nothing about what
  // the return value is when GC_n == 0.
  GC_Tp* allocate(size_type GC_n, const void* = 0) {
    GC_type_traits<GC_Tp> traits;
    return static_cast<GC_Tp *>
	    (GC_selective_alloc(GC_n * sizeof(GC_Tp),
			        traits.GC_is_ptr_free));
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type GC_ATTR_UNUSED GC_n)
    { GC_FREE(__p); }

  size_type max_size() const throw()
    { return size_t(-1) / sizeof(GC_Tp); }

  void construct(pointer __p, const GC_Tp& __val) { new(__p) GC_Tp(__val); }
  void destroy(pointer __p) { __p->~GC_Tp(); }
};

template<>
class gc_allocator<void> {
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class GC_Tp1> struct rebind {
    typedef gc_allocator<GC_Tp1> other;
  };
};


template <class GC_T1, class GC_T2>
inline bool operator==(const gc_allocator<GC_T1>&, const gc_allocator<GC_T2>&)
{
  return true;
}

template <class GC_T1, class GC_T2>
inline bool operator!=(const gc_allocator<GC_T1>&, const gc_allocator<GC_T2>&)
{
  return false;
}

/*
 * And the public traceable_allocator class.
 */

// Note that we currently don't specialize the pointer-free case, since a
// pointer-free traceable container doesn't make that much sense,
// though it could become an issue due to abstraction boundaries.
template <class GC_Tp>
class traceable_allocator {
public:
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
  typedef GC_Tp*       pointer;
  typedef const GC_Tp* const_pointer;
  typedef GC_Tp&       reference;
  typedef const GC_Tp& const_reference;
  typedef GC_Tp        value_type;

  template <class GC_Tp1> struct rebind {
    typedef traceable_allocator<GC_Tp1> other;
  };

  traceable_allocator() throw() {}
    traceable_allocator(const traceable_allocator&) throw() {}
# if !(GC_NO_MEMBER_TEMPLATES || 0 < _MSC_VER && _MSC_VER <= 1200)
  // MSVC++ 6.0 do not support member templates
  template <class GC_Tp1> traceable_allocator
	  (const traceable_allocator<GC_Tp1>&) throw() {}
# endif
  ~traceable_allocator() throw() {}

  pointer address(reference GC_x) const { return &GC_x; }
  const_pointer address(const_reference GC_x) const { return &GC_x; }

  // GC_n is permitted to be 0.  The C++ standard says nothing about what
  // the return value is when GC_n == 0.
  GC_Tp* allocate(size_type GC_n, const void* = 0) {
    return static_cast<GC_Tp*>(GC_MALLOC_UNCOLLECTABLE(GC_n * sizeof(GC_Tp)));
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type GC_ATTR_UNUSED GC_n)
    { GC_FREE(__p); }

  size_type max_size() const throw()
    { return size_t(-1) / sizeof(GC_Tp); }

  void construct(pointer __p, const GC_Tp& __val) { new(__p) GC_Tp(__val); }
  void destroy(pointer __p) { __p->~GC_Tp(); }
};

template<>
class traceable_allocator<void> {
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class GC_Tp1> struct rebind {
    typedef traceable_allocator<GC_Tp1> other;
  };
};


template <class GC_T1, class GC_T2>
inline bool operator==(const traceable_allocator<GC_T1>&, const traceable_allocator<GC_T2>&)
{
  return true;
}

template <class GC_T1, class GC_T2>
inline bool operator!=(const traceable_allocator<GC_T1>&, const traceable_allocator<GC_T2>&)
{
  return false;
}

#endif /* GC_ALLOCATOR_H */
