#ifndef __em_asm_h__
#define __em_asm_h__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef __asmjs
// When calling EM_ASM functions, we're calling out to JS. JS only has doubles
// natively, so we can convert all C types to double before the call. By
// converting all arguments to double, we can store them in a vararg buffer,
// and we only need to pass the number of args in order to read them in JS.

// When converting arguments to double, we need to:
//   - upcast floats to doubles, so we have consistent bit widths
//   - convert ints to doubles (via f64.convert_s/i32 and such)
//   - cast pointers to ints, and then convert (C disallows direct pointer to
//     double conversions)
// We can use the generic selection C11 feature (that clang supports pre-C11
// as an extension) to decide at compile time what operations to apply.
// Unfortunately each branch needs to compile for all possible types, even
// though only one is selected. Fortunately, we can use a function inside the
// _Generic expression, and use that do do our conversion.
// All pointer types should go through the default case.
#define _EM_ASM_CAST(x) _Generic((x), \
    float: _em_asm_cast_double, \
    double: _em_asm_cast_double, \
    int: _em_asm_cast_int, \
    unsigned: _em_asm_cast_uint, \
    long: _em_asm_cast_int, \
    unsigned long: _em_asm_cast_uint, \
    default: _em_asm_cast_ptr)(x)
inline double _em_asm_cast_double(double x) {
  return x;
}
inline double _em_asm_cast_int(long x) {
  return (double)x;
}
inline double _em_asm_cast_uint(unsigned long x) {
  return (double)x;
}
inline double _em_asm_cast_ptr(const void* x) {
  return (double)(long)x;
}

// This indirection is needed to allow us to concatenate computed results, e.g.
//   #define BAR(N) _EM_ASM_CONCATENATE(FOO_, N)
//   BAR(3) // rewritten to BAR_3
// whereas using ## or _EM_ASM_CONCATENATE_ directly would result in BAR_N
#define _EM_ASM_CONCATENATE(a, b) _EM_ASM_CONCATENATE_(a, b)
#define _EM_ASM_CONCATENATE_(a, b) a##b

// Counts arguments. We use $$ as a sentinel value to enable using ##__VA_ARGS__
// which omits a comma in the event that we have 0 arguments passed, which is
// necessary to keep the count correct.
// TODO(jgravelle): increase the max number of args to 32 or so
#define _EM_ASM_COUNT_ARGS_EXP(_$,_0,_1,_2,_3,_4,n,...) n
#define _EM_ASM_COUNT_ARGS(...) _EM_ASM_COUNT_ARGS_EXP($$,##__VA_ARGS__,5,4,3,2,1,0)

// Promote each argument to double. We lead with commas to avoid adding a comma
// in the 0-argument case, which messes up the argument parsing.
// Note that we omit a comma separating calls to _EM_ASM_PROMOTE_ARGS as well.
#define _EM_ASM_PROMOTE_ARGS_0(x, ...)
#define _EM_ASM_PROMOTE_ARGS_1(x, ...) , _EM_ASM_CAST(x)
#define _EM_ASM_PROMOTE_ARGS_2(x, ...) , _EM_ASM_CAST(x) _EM_ASM_PROMOTE_ARGS_1(__VA_ARGS__)
#define _EM_ASM_PROMOTE_ARGS_3(x, ...) , _EM_ASM_CAST(x) _EM_ASM_PROMOTE_ARGS_2(__VA_ARGS__)
#define _EM_ASM_PROMOTE_ARGS_4(x, ...) , _EM_ASM_CAST(x) _EM_ASM_PROMOTE_ARGS_3(__VA_ARGS__)
#define _EM_ASM_PROMOTE_ARGS_5(x, ...) , _EM_ASM_CAST(x) _EM_ASM_PROMOTE_ARGS_4(__VA_ARGS__)
#define _EM_ASM_PROMOTE_ARGS(N, ...) \
  _EM_ASM_CONCATENATE(_EM_ASM_PROMOTE_ARGS_,N)(__VA_ARGS__)

#define _EM_ASM_PREP_ARGS(...) \
  , _EM_ASM_COUNT_ARGS(__VA_ARGS__) \
    _EM_ASM_PROMOTE_ARGS(_EM_ASM_COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__)

void emscripten_asm_const(const char* code, int num_args, ...);
int emscripten_asm_const_int(const char* code, int num_args, ...);
double emscripten_asm_const_double(const char* code, int num_args, ...);

#else // __asmjs
#define _EM_ASM_PREP_ARGS(...) , ##__VA_ARGS__

int emscripten_asm_const_int(const char* code, ...);
double emscripten_asm_const_double(const char* code, ...);

int emscripten_asm_const_int_sync_on_main_thread(const char* code, ...);
double emscripten_asm_const_double_sync_on_main_thread(const char* code, ...);

void emscripten_asm_const_async_on_main_thread(const char* code, ...);

#endif // __asmjs

#ifdef __cplusplus
}
#endif // __cplusplus

// Note: If the code block in the EM_ASM() family of functions below contains a comma,
// then wrap the whole code block inside parentheses (). See tests/core/test_em_asm_2.cpp
// for example code snippets.

// Runs the given JavaScript code on the calling thread (synchronously), and returns no value back.
#define EM_ASM(code, ...) ((void)emscripten_asm_const_int(#code _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code on the calling thread (synchronously), and returns an integer back.
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code on the calling thread (synchronously), and returns a double back.
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns no value back.
// Call this function for example to access DOM elements in a pthread/web worker. Avoid calling this
// function in performance sensitive code, because this will effectively sleep the calling thread until the
// main browser thread is able to service the proxied function call. If you have multiple MAIN_THREAD_EM_ASM()
// code blocks to call in succession, it will likely be much faster to coalesce all the calls to a single
// MAIN_THREAD_EM_ASM() block. If you do not need synchronization nor a return value back, consider using
// the function MAIN_THREAD_ASYNC_EM_ASM() instead, which will not block.
#define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(#code _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code synchronously on the main browser thread, and returns an integer back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_INT(code, ...) emscripten_asm_const_int_sync_on_main_thread(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns a double back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double_sync_on_main_thread(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))

// Asynchronously dispatches the given JavaScript code to be run on the main browser thread.
// If the calling thread is the main browser thread, then the specified JavaScript code is executed
// synchronously. Otherwise an event will be queued on the main browser thread to execute the call
// later (think postMessage()), and this call will immediately return without waiting. Be sure to
// guard any accesses to shared memory on the heap inside the JavaScript code with appropriate locking.
#define MAIN_THREAD_ASYNC_EM_ASM(code, ...) ((void)emscripten_asm_const_async_on_main_thread(#code _EM_ASM_PREP_ARGS(__VA_ARGS__)))

// Old forms for compatibility, no need to use these.
// Replace EM_ASM_, EM_ASM_ARGS and EM_ASM_INT_V with EM_ASM_INT,
// and EM_ASM_DOUBLE_V with EM_ASM_DOUBLE.
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code _EM_ASM_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_INT_V(code) EM_ASM_INT(#code)
#define EM_ASM_DOUBLE_V(code) EM_ASM_DOUBLE(#code)

#endif // __em_asm_h__
