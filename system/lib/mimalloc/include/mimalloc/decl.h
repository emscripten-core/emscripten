/* ----------------------------------------------------------------------------
Copyright (c) 2019-2026 Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
  Compiler dependent definitions
---------------------------------------------------------------------------- */

#pragma once
#ifndef MI_DECL_H
#define MI_DECL_H

// --------------------------------------------------------------------------
// Compiler defines
// --------------------------------------------------------------------------
#if (defined(__APPLE__) && defined(__aarch64__)) || defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
#define MI_CACHE_LINE_SIZE      128
#define mi_decl_cache_align     mi_decl_align(128)          // must use a constant here
#else
#define MI_CACHE_LINE_SIZE      64
#define mi_decl_cache_align     mi_decl_align(64)           // must use a constant here
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4127)   // suppress constant conditional warning (due to MI_SECURE paths)
#pragma warning(disable:4324)   // structure was padded due to alignment specifier (intended for `mi_decl_cache_align`)
#pragma warning(disable:26812)  // unscoped enum warning
#define mi_decl_forceinline     __forceinline
#define mi_decl_noinline        __declspec(noinline)
#define mi_decl_thread          __declspec(thread)
#define mi_decl_noreturn        __declspec(noreturn)
#elif (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__clang__) // includes clang and icc
#if !MI_TRACK_ASAN
#define mi_decl_forceinline     __attribute__((always_inline)) inline
#else
#define mi_decl_forceinline     inline
#endif
#define mi_decl_noinline        __attribute__((noinline))
#define mi_decl_thread          __thread
#define mi_decl_noreturn        __attribute__((noreturn))
#elif __cplusplus >= 201103L    // c++11
#define mi_decl_forceinline     inline
#define mi_decl_noinline
#define mi_decl_thread          thread_local
#define mi_decl_noreturn        [[noreturn]]
#else
#define mi_decl_forceinline     inline
#define mi_decl_noinline
#define mi_decl_thread          __thread        // hope for the best :-)
#define mi_decl_noreturn
#endif

#if !defined(__MINGW32__) && !defined(__CYGWIN__) && ((defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__clang__))
#define mi_decl_hidden          __attribute__((visibility("hidden")))
#else
#define mi_decl_hidden
#endif

#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))) || defined(__clang__)
#define mi_prefetch_write(p)    __builtin_prefetch(p, 1, 3)
#define mi_prefetch_read(p)     __builtin_prefetch(p, 0, 3)
#define mi_decl_cold            __attribute__((cold))
#define mi_decl_weak            __attribute__((weak))
#define mi_assume_aligned(p,sz) __builtin_assume_aligned(p,sz)
#define mi_decl_func_align      __attribute__((aligned(64)))
#else
#define mi_prefetch_write(p)
#define mi_prefetch_read(p)
#define mi_decl_cold
#define mi_decl_weak
#define mi_assume_aligned(p,sz) (p)
#define mi_decl_func_align
#endif

#if defined(__GNUC__) || defined(__clang__)
#define mi_unlikely(x)          (__builtin_expect(!!(x),false))
#define mi_likely(x)            (__builtin_expect(!!(x),true))
#elif (defined(__cplusplus) && (__cplusplus >= 202002L)) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#define mi_unlikely(x)          (x) [[unlikely]]
#define mi_likely(x)            (x) [[likely]]
#else
#define mi_unlikely(x)          (x)
#define mi_likely(x)            (x)
#endif

#if (defined(__GNUC__) && (__GNUC__ >= 7)) || defined(__clang__)
#define mi_decl_maybe_unused    __attribute__((unused))
#elif __cplusplus >= 201703L    // c++17
#define mi_decl_maybe_unused    [[maybe_unused]]
#else
#define mi_decl_maybe_unused
#endif

#ifndef __has_builtin
#define __has_builtin(x)        0
#endif


// ------------------------------------------------------
// Define MI_DEBUG for assertion and invariant checking
// #define MI_DEBUG 1  // basic assertion checks and statistics, check double free, corrupted free list, and invalid pointer free. (cmake -DMI_DEBUG=ON)
// #define MI_DEBUG 2  // + internal assertion checks (cmake -DMI_DEBUG_INTERNAL=ON)
// #define MI_DEBUG 3  // + extensive internal invariant checking (cmake -DMI_DEBUG_FULL=ON)
#if !defined(MI_DEBUG)
#if defined(MI_BUILD_RELEASE) || defined(NDEBUG)
#define MI_DEBUG 0
#else
#define MI_DEBUG 2
#endif
#endif

#define MI_UNUSED(x)     (void)(x)
#if (MI_DEBUG>1)
#define MI_UNUSED_RELEASE(x)
#else
#define MI_UNUSED_RELEASE(x)  MI_UNUSED(x)
#endif

// ------------------------------------------------------
// Assertions and tracing
// ------------------------------------------------------

#if (MI_DEBUG>0)
void _mi_trace_message(const char* fmt, ...);
#define mi_trace_message(...)  _mi_trace_message(__VA_ARGS__)
#else
#define mi_trace_message(...)
#endif


#if (MI_DEBUG)
// use our own assertion to print without memory allocation
mi_decl_noreturn mi_decl_cold void _mi_assert_fail(const char* assertion, const char* fname, unsigned int line, const char* func) mi_attr_noexcept;
#define mi_assert(expr)     ((expr) ? (void)0 : _mi_assert_fail(#expr,__FILE__,__LINE__,__func__))
#else
#define mi_assert(x)
#endif

#if (MI_DEBUG>1)
#define mi_assert_internal    mi_assert
#else
#define mi_assert_internal(x)
#endif

#if (MI_DEBUG>2)
#define mi_assert_expensive   mi_assert
#else
#define mi_assert_expensive(x)
#endif


// ------------------------------------------------------
// Initialization macros
// ------------------------------------------------------

#define MI_INIT4(x)   x(),x(),x(),x()
#define MI_INIT8(x)   MI_INIT4(x),MI_INIT4(x)
#define MI_INIT16(x)  MI_INIT8(x),MI_INIT8(x)
#define MI_INIT32(x)  MI_INIT16(x),MI_INIT16(x)
#define MI_INIT64(x)  MI_INIT32(x),MI_INIT32(x)
#define MI_INIT128(x) MI_INIT64(x),MI_INIT64(x)
#define MI_INIT256(x) MI_INIT128(x),MI_INIT128(x)

#define MI_INIT74(x)  MI_INIT64(x),MI_INIT8(x),x(),x()
#define MI_INIT5(x)   MI_INIT4(x),x()
#define MI_INIT6(x)   MI_INIT4(x),x(),x()

// ------------------------------------------------------
// Interposing for macOS (DYLD)
// ------------------------------------------------------

#if defined(__APPLE__) && defined(__MACH__)
struct mi_interpose_s {
  const void* replacement;
  const void* target;
};
#define MI_INTERPOSE_FUN(oldfun,newfun) { (const void*)&newfun, (const void*)&oldfun }
#define MI_INTERPOSE_MI(fun)            MI_INTERPOSE_FUN(fun,mi_##fun)
#define MI_INTERPOSE_DECLS(name)        __attribute__((used)) static const struct mi_interpose_s name[]  __attribute__((section("__DATA, __interpose")))
#endif

#endif // MI_DECL_H
