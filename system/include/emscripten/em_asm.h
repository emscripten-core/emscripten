/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// You can use these functions by passing format string to arg_sigs.
// Note that `code` requires you to provide a const C string known at compile time,
// otherwise the "unable to find data for ASM/EM_JS const" error will be thrown.
// https://github.com/WebAssembly/binaryen/blob/51c8f2469f8fd05197b7694c65041b1567f2c6b5/src/wasm/wasm-emscripten.cpp#L183

// C++ needs the nothrow attribute so -O0 doesn't lower these calls as invokes.
__attribute__((nothrow))
int emscripten_asm_const_int(const char* code, const char* arg_sigs, ...);
__attribute__((nothrow))
double emscripten_asm_const_double(const char* code, const char* arg_sigs, ...);

__attribute__((nothrow))
int emscripten_asm_const_int_sync_on_main_thread(
  const char* code, const char* arg_sigs, ...);
__attribute__((nothrow))
double emscripten_asm_const_double_sync_on_main_thread(
  const char* code, const char* arg_sigs, ...);

__attribute__((nothrow))
void emscripten_asm_const_async_on_main_thread(
  const char* code, const char* arg_sigs, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

// EM_ASM does not work strict C mode.
#if !defined(__cplusplus) && defined(__STRICT_ANSI__)

#define EM_ASM_ERROR _Pragma("GCC error(\"EM_ASM does not work in -std=c* modes, use -std=gnu* modes instead\")")
#define EM_ASM(...) EM_ASM_ERROR
#define EM_ASM_INT(...) EM_ASM_ERROR
#define EM_ASM_DOUBLE(...) EM_ASM_ERROR
#define MAIN_THREAD_EM_ASM(...) EM_ASM_ERROR
#define MAIN_THREAD_EM_ASM_INT(...) EM_ASM_ERROR
#define MAIN_THREAD_EM_ASM_DOUBLE(...) EM_ASM_ERROR
#define MAIN_THREAD_ASYNC_EM_ASM(...) EM_ASM_ERROR
#define EM_ASM_(...) EM_ASM_ERROR
#define EM_ASM_ARGS(...) EM_ASM_ERROR
#define EM_ASM_INT_V(...) EM_ASM_ERROR
#define EM_ASM_DOUBLE_V(...) EM_ASM_ERROR

#else

// In wasm backend, we need to call the emscripten_asm_const_* functions with
// the C vararg calling convention, because we will call it with a variety of
// arguments, but need to generate a coherent import for the wasm module before
// binaryen can run over it to fix up any calls to emscripten_asm_const_*.  In
// order to read from a vararg buffer, we need to know the signatures to read.
// We can use compile-time trickery to generate a format string, and read that
// in JS in order to correctly handle the vararg buffer.

#ifndef __cplusplus

// We can use the generic selection C11 feature (that clang supports pre-C11
// as an extension) to emulate function overloading in C.
// All pointer types should go through the default case.
#define _EM_ASM_SIG_CHAR(x) _Generic((x), \
    float: 'f', \
    double: 'd', \
    int: 'i', \
    unsigned: 'i', \
    default: 'i')

// This indirection is needed to allow us to concatenate computed results, e.g.
//   #define BAR(N) _EM_ASM_CONCATENATE(FOO_, N)
//   BAR(3) // rewritten to BAR_3
// whereas using ## or _EM_ASM_CONCATENATE_ directly would result in BAR_N
#define _EM_ASM_CONCATENATE(a, b) _EM_ASM_CONCATENATE_(a, b)
#define _EM_ASM_CONCATENATE_(a, b) a##b

// Counts arguments. We use $$ as a sentinel value to enable using ##__VA_ARGS__
// which omits a comma in the event that we have 0 arguments passed, which is
// necessary to keep the count correct.
#define _EM_ASM_COUNT_ARGS_EXP(_$,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,n,...) n
#define _EM_ASM_COUNT_ARGS(...) \
    _EM_ASM_COUNT_ARGS_EXP($$,##__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

// Find the corresponding char for each argument.
#define _EM_ASM_ARG_SIGS_0(x, ...)
#define _EM_ASM_ARG_SIGS_1(x, ...) _EM_ASM_SIG_CHAR(x),
#define _EM_ASM_ARG_SIGS_2(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_1(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_3(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_2(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_4(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_3(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_5(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_4(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_6(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_5(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_7(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_6(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_8(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_7(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_9(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_8(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_10(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_9(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_11(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_10(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_12(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_11(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_13(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_12(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_14(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_13(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_15(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_14(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_16(x, ...) _EM_ASM_SIG_CHAR(x), _EM_ASM_ARG_SIGS_15(__VA_ARGS__)
#define _EM_ASM_ARG_SIGS_(N, ...) \
    ((char[]){ _EM_ASM_CONCATENATE(_EM_ASM_ARG_SIGS_,N)(__VA_ARGS__) '\0' })

#define _EM_ASM_ARG_SIGS(...) \
    _EM_ASM_ARG_SIGS_(_EM_ASM_COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__)

// We lead with commas to avoid adding an extra comma in the 0-argument case.
#define _EM_ASM_PREP_ARGS(...) , _EM_ASM_ARG_SIGS(__VA_ARGS__), ##__VA_ARGS__

#else // __cplusplus

// C++ needs to support vararg template parameter packs, e.g. like in
// tests/core/test_em_asm_parameter_pack.cpp. Because of that, a macro-only
// approach doesn't work (a macro applied to a parameter pack would expand
// incorrectly). So we can use a template class instead to build a temporary
// buffer of characters.

// As emscripten is require to build successfully with -std=c++03, we cannot
// use std::tuple or std::integral_constant. Using C++11 features is only a
// warning in modern Clang, which are ignored in system headers.
template<typename, typename = void> struct __em_asm_sig {};
template<> struct __em_asm_sig<float> { static const char value = 'd'; };
template<> struct __em_asm_sig<double> { static const char value = 'd'; };
template<> struct __em_asm_sig<char> { static const char value = 'i'; };
template<> struct __em_asm_sig<signed char> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned char> { static const char value = 'i'; };
template<> struct __em_asm_sig<short> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned short> { static const char value = 'i'; };
template<> struct __em_asm_sig<int> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned int> { static const char value = 'i'; };
template<> struct __em_asm_sig<long> { static const char value = 'i'; };
template<> struct __em_asm_sig<unsigned long> { static const char value = 'i'; };
template<> struct __em_asm_sig<bool> { static const char value = 'i'; };
template<> struct __em_asm_sig<wchar_t> { static const char value = 'i'; };
template<typename T> struct __em_asm_sig<T*> { static const char value = 'i'; };

// Explicit support for enums, they're passed as int via variadic arguments.
template<bool> struct __em_asm_if { };
template<> struct __em_asm_if<true> { typedef void type; };
template<typename T> struct __em_asm_sig<T, typename __em_asm_if<__is_enum(T)>::type> {
    static const char value = 'i';
};

// Instead of std::tuple
template<typename... Args>
struct __em_asm_type_tuple {};

// Instead of std::make_tuple
template<typename... Args>
__em_asm_type_tuple<Args...> __em_asm_make_type_tuple(Args... args) {
    return {};
}

template<typename>
struct __em_asm_sig_builder {};

template<typename... Args>
struct __em_asm_sig_builder<__em_asm_type_tuple<Args...> > {
  static const char buffer[sizeof...(Args) + 1];
};

template<typename... Args>
const char __em_asm_sig_builder<__em_asm_type_tuple<Args...> >::buffer[] = { __em_asm_sig<Args>::value..., 0 };

// We move to type level with decltype(make_tuple(...)) to avoid double
// evaluation of arguments. Use __typeof__ instead of decltype, though,
// because the header should be able to compile with clang's -std=c++03.
#define _EM_ASM_PREP_ARGS(...) \
    , __em_asm_sig_builder<__typeof__(__em_asm_make_type_tuple(__VA_ARGS__))>::buffer, ##__VA_ARGS__
#endif // __cplusplus

// Note: If the code block in the EM_ASM() family of functions below contains a comma,
// then wrap the whole code block inside parentheses (). See tests/core/test_em_asm_2.cpp
// for example code snippets.

#define CODE_EXPR(code) (__extension__({           \
    __attribute__((section("em_asm"), aligned(1))) \
    static const char x[] = code;                  \
    x;                                             \
  }))

// Runs the given JavaScript code on the calling thread (synchronously), and returns no value back.
#define EM_ASM(code, ...) ((void)emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code on the calling thread (synchronously), and returns an integer back.
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code on the calling thread (synchronously), and returns a double back.
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns no value back.
// Call this function for example to access DOM elements in a pthread when building with -s USE_PTHREADS=1.
// Avoid calling this function in performance sensitive code, because this will effectively sleep the
// calling thread until the main browser thread is able to service the proxied function call. If you have
// multiple MAIN_THREAD_EM_ASM() code blocks to call in succession, it will likely be much faster to
// coalesce all the calls to a single MAIN_THREAD_EM_ASM() block. If you do not need synchronization nor
// a return value back, consider using the function MAIN_THREAD_ASYNC_EM_ASM() instead, which will not block.
// In single-threaded builds (including proxy-to-worker), MAIN_THREAD_EM_ASM*()
// functions are direct aliases to the corresponding EM_ASM*() family of functions.
#define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code synchronously on the main browser thread, and returns an integer back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_INT(code, ...) emscripten_asm_const_int_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns a double back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double_sync_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Asynchronously dispatches the given JavaScript code to be run on the main browser thread.
// If the calling thread is the main browser thread, then the specified JavaScript code is executed
// synchronously. Otherwise an event will be queued on the main browser thread to execute the call
// later (think postMessage()), and this call will immediately return without waiting. Be sure to guard any accesses to shared memory on the heap inside the JavaScript code with appropriate locking.
#define MAIN_THREAD_ASYNC_EM_ASM(code, ...) ((void)emscripten_asm_const_async_on_main_thread(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Old forms for compatibility, no need to use these.
// Replace EM_ASM_, EM_ASM_ARGS and EM_ASM_INT_V with EM_ASM_INT,
// and EM_ASM_DOUBLE_V with EM_ASM_DOUBLE.
#define EM_ASM_(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(CODE_EXPR(#code) _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_INT_V(code) EM_ASM_INT(code)
#define EM_ASM_DOUBLE_V(code) EM_ASM_DOUBLE(code)

#endif // !defined(__cplusplus) && defined(__STRICT_ANSI__)
