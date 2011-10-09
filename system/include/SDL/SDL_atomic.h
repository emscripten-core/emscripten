/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 * \file SDL_atomic.h
 * 
 * Atomic operations.
 * 
 * IMPORTANT:
 * If you are not an expert in concurrent lockless programming, you should
 * only be using the atomic lock and reference counting functions in this
 * file.  In all other cases you should be protecting your data structures
 * with full mutexes.
 * 
 * The list of "safe" functions to use are:
 *  SDL_AtomicLock()
 *  SDL_AtomicUnlock()
 *  SDL_AtomicIncRef()
 *  SDL_AtomicDecRef()
 * 
 * Seriously, here be dragons!
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 * You can find out a little more about lockless programming and the 
 * subtle issues that can arise here:
 * http://msdn.microsoft.com/en-us/library/ee418650%28v=vs.85%29.aspx
 *
 * There's also lots of good information here:
 * http://www.1024cores.net/home/lock-free-algorithms
 *
 * These operations may or may not actually be implemented using
 * processor specific atomic operations. When possible they are
 * implemented as true processor specific atomic operations. When that
 * is not possible the are implemented using locks that *do* use the
 * available atomic operations.
 *
 * All of the atomic operations that modify memory are full memory barriers.
 */

#ifndef _SDL_atomic_h_
#define _SDL_atomic_h_

#include "SDL_stdinc.h"
#include "SDL_platform.h"

#include "begin_code.h"

/* Need to do this here because intrin.h has C++ code in it */
/* Visual Studio 2005 has a bug where intrin.h conflicts with winnt.h */
#if defined(_MSC_VER) && (_MSC_VER >= 1500) && !defined(_WIN32_WCE)
#include <intrin.h>
#define HAVE_MSC_ATOMICS 1
#endif

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 * \name SDL AtomicLock
 * 
 * The atomic locks are efficient spinlocks using CPU instructions,
 * but are vulnerable to starvation and can spin forever if a thread
 * holding a lock has been terminated.  For this reason you should
 * minimize the code executed inside an atomic lock and never do
 * expensive things like API or system calls while holding them.
 *
 * The atomic locks are not safe to lock recursively.
 *
 * Porting Note:
 * The spin lock functions and type are required and can not be
 * emulated because they are used in the atomic emulation code.
 */
/*@{*/

typedef int SDL_SpinLock;

/**
 * \brief Try to lock a spin lock by setting it to a non-zero value.
 * 
 * \param lock Points to the lock.
 *
 * \return SDL_TRUE if the lock succeeded, SDL_FALSE if the lock is already held.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_AtomicTryLock(SDL_SpinLock *lock);

/**
 * \brief Lock a spin lock by setting it to a non-zero value.
 * 
 * \param lock Points to the lock.
 */
extern DECLSPEC void SDLCALL SDL_AtomicLock(SDL_SpinLock *lock);

/**
 * \brief Unlock a spin lock by setting it to 0. Always returns immediately
 *
 * \param lock Points to the lock.
 */
extern DECLSPEC void SDLCALL SDL_AtomicUnlock(SDL_SpinLock *lock);

/*@}*//*SDL AtomicLock*/


/**
 * The compiler barrier prevents the compiler from reordering
 * reads and writes to globally visible variables across the call.
 */
#ifdef _MSC_VER
void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)
#define SDL_CompilerBarrier()   _ReadWriteBarrier()
#elif defined(__GNUC__)
#define SDL_CompilerBarrier()   __asm__ __volatile__ ("" : : : "memory")
#else
#define SDL_CompilerBarrier()   \
({ SDL_SpinLock _tmp = 0; SDL_AtomicLock(&_tmp); SDL_AtomicUnlock(&_tmp); })
#endif

/* Platform specific optimized versions of the atomic functions,
 * you can disable these by defining SDL_DISABLE_ATOMIC_INLINE
 */
#if defined(SDL_ATOMIC_DISABLED) && SDL_ATOMIC_DISABLED
#define SDL_DISABLE_ATOMIC_INLINE
#endif
#ifndef SDL_DISABLE_ATOMIC_INLINE

#ifdef HAVE_MSC_ATOMICS

#define SDL_AtomicSet(a, v)     _InterlockedExchange((long*)&(a)->value, (v))
#define SDL_AtomicAdd(a, v)     _InterlockedExchangeAdd((long*)&(a)->value, (v))
#define SDL_AtomicCAS(a, oldval, newval) (_InterlockedCompareExchange((long*)&(a)->value, (newval), (oldval)) == (oldval))
#define SDL_AtomicSetPtr(a, v)  _InterlockedExchangePointer((a), (v))
#if _M_IX86
#define SDL_AtomicCASPtr(a, oldval, newval) (_InterlockedCompareExchange((long*)(a), (long)(newval), (long)(oldval)) == (long)(oldval))
#else
#define SDL_AtomicCASPtr(a, oldval, newval) (_InterlockedCompareExchangePointer((a), (newval), (oldval)) == (oldval))
#endif

#elif defined(__MACOSX__)
#include <libkern/OSAtomic.h>

#define SDL_AtomicCAS(a, oldval, newval) OSAtomicCompareAndSwap32Barrier((oldval), (newval), &(a)->value)
#if SIZEOF_VOIDP == 4
#define SDL_AtomicCASPtr(a, oldval, newval) OSAtomicCompareAndSwap32Barrier((int32_t)(oldval), (int32_t)(newval), (int32_t*)(a))
#elif SIZEOF_VOIDP == 8
#define SDL_AtomicCASPtr(a, oldval, newval) OSAtomicCompareAndSwap64Barrier((int64_t)(oldval), (int64_t)(newval), (int64_t*)(a))
#endif

#elif defined(HAVE_GCC_ATOMICS)

#define SDL_AtomicSet(a, v)     __sync_lock_test_and_set(&(a)->value, v)
#define SDL_AtomicAdd(a, v)     __sync_fetch_and_add(&(a)->value, v)
#define SDL_AtomicSetPtr(a, v)  __sync_lock_test_and_set(a, v)
#define SDL_AtomicCAS(a, oldval, newval) __sync_bool_compare_and_swap(&(a)->value, oldval, newval)
#define SDL_AtomicCASPtr(a, oldval, newval) __sync_bool_compare_and_swap(a, oldval, newval)

#endif

#endif /* !SDL_DISABLE_ATOMIC_INLINE */


/**
 * \brief A type representing an atomic integer value.  It is a struct
 *        so people don't accidentally use numeric operations on it.
 */
#ifndef SDL_atomic_t_defined
typedef struct { int value; } SDL_atomic_t;
#endif

/**
 * \brief Set an atomic variable to a new value if it is currently an old value.
 *
 * \return SDL_TRUE if the atomic variable was set, SDL_FALSE otherwise.
 *
 * \note If you don't know what this function is for, you shouldn't use it!
*/
#ifndef SDL_AtomicCAS
#define SDL_AtomicCAS SDL_AtomicCAS_
#endif
extern DECLSPEC SDL_bool SDLCALL SDL_AtomicCAS_(SDL_atomic_t *a, int oldval, int newval);

/**
 * \brief Set an atomic variable to a value.
 *
 * \return The previous value of the atomic variable.
 */
#ifndef SDL_AtomicSet
static __inline__ int SDL_AtomicSet(SDL_atomic_t *a, int v)
{
    int value;
    do {
        value = a->value;
    } while (!SDL_AtomicCAS(a, value, v));
    return value;
}
#endif

/**
 * \brief Get the value of an atomic variable
 */
#ifndef SDL_AtomicGet
static __inline__ int SDL_AtomicGet(SDL_atomic_t *a)
{
    int value = a->value;
    SDL_CompilerBarrier();
    return value;
}
#endif

/**
 * \brief Add to an atomic variable.
 *
 * \return The previous value of the atomic variable.
 *
 * \note This same style can be used for any number operation
 */
#ifndef SDL_AtomicAdd
static __inline__ int SDL_AtomicAdd(SDL_atomic_t *a, int v)
{
    int value;
    do {
        value = a->value;
    } while (!SDL_AtomicCAS(a, value, (value + v)));
    return value;
}
#endif

/**
 * \brief Increment an atomic variable used as a reference count.
 */
#ifndef SDL_AtomicIncRef
#define SDL_AtomicIncRef(a)    SDL_AtomicAdd(a, 1)
#endif

/**
 * \brief Decrement an atomic variable used as a reference count.
 *
 * \return SDL_TRUE if the variable reached zero after decrementing,
 *         SDL_FALSE otherwise
 */
#ifndef SDL_AtomicDecRef
#define SDL_AtomicDecRef(a)    (SDL_AtomicAdd(a, -1) == 1)
#endif

/**
 * \brief Set a pointer to a new value if it is currently an old value.
 *
 * \return SDL_TRUE if the pointer was set, SDL_FALSE otherwise.
 *
 * \note If you don't know what this function is for, you shouldn't use it!
*/
#ifndef SDL_AtomicCASPtr
#define SDL_AtomicCASPtr SDL_AtomicCASPtr_
#endif
extern DECLSPEC SDL_bool SDLCALL SDL_AtomicCASPtr_(void* *a, void *oldval, void *newval);

/**
 * \brief Set a pointer to a value atomically.
 *
 * \return The previous value of the pointer.
 */
#ifndef SDL_AtomicSetPtr
static __inline__ void* SDL_AtomicSetPtr(void* *a, void* v)
{
    void* value;
    do {
        value = *a;
    } while (!SDL_AtomicCASPtr(a, value, v));
    return value;
}
#endif

/**
 * \brief Get the value of a pointer atomically.
 */
#ifndef SDL_AtomicGetPtr
static __inline__ void* SDL_AtomicGetPtr(void* *a)
{
    void* value = *a;
    SDL_CompilerBarrier();
    return value;
}
#endif


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#include "close_code.h"

#endif /* _SDL_atomic_h_ */

/* vi: set ts=4 sw=4 expandtab: */
