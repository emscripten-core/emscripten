#ifndef __em_asm_h__
#define __em_asm_h__

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
// We can use the _Generic clang extension / C11 feature to decide at compile
// time what operations to apply. Unfortunately each branch needs to compile for
// all possible types, even though only one is selected. Fortunately, we can use
// a function inside the _Generic expression, and use that do do our conversion.
// All pointer types should go through the default: case.
#define _EA_CAST(x) _Generic((x), \
    float: _ea_cast_double, \
    double: _ea_cast_double, \
    int: _ea_cast_int, \
    unsigned: _ea_cast_uint, \
    long: _ea_cast_int, \
    unsigned long: _ea_cast_uint, \
    default: _ea_cast_ptr)(x)
inline double _ea_cast_double(double x) {
  return x;
}
inline double _ea_cast_int(long x) {
  return (double)x;
}
inline double _ea_cast_uint(unsigned long x) {
  return (double)x;
}
inline double _ea_cast_ptr(const void* x) {
  return (double)(long)x;
}

// This indirection is needed to allow us to concatenate computed results, e.g.
//   #define BAR(N) _EA_CONCATENATE(FOO_, N)
//   BAR(3) // rewritten to BAR_3
// whereas using ## or _EA_CONCATENATE_ directly would result in BAR_N
#define _EA_CONCATENATE(a, b) _EA_CONCATENATE_(a, b)
#define _EA_CONCATENATE_(a, b) a##b

// Counts arguments. We use $$ as a sentinel value to enable using ##__VA_ARGS__
// which omits a comma in the event that we have 0 arguments passed, which is
// necessary to keep the count correct.
// TODO(jgravelle): increase the max number of args to 32 or so
#define _EA_COUNT_ARGS_EXP(_$,_0,_1,_2,_3,_4,n,...) n
#define _EA_COUNT_ARGS(...) _EA_COUNT_ARGS_EXP($$,##__VA_ARGS__,5,4,3,2,1,0)

// Promote each argument to double. We lead with commas to avoid adding a comma
// in the 0-argument case, which messes up the argument parsing.
// Note that we omit a comma separating calls to _EA_PROMOTE_ARGS as well.
#define _EA_PROMOTE_ARGS_0(x, ...)
#define _EA_PROMOTE_ARGS_1(x, ...) , _EA_CAST(x)
#define _EA_PROMOTE_ARGS_2(x, ...) , _EA_CAST(x) _EA_PROMOTE_ARGS_1(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_3(x, ...) , _EA_CAST(x) _EA_PROMOTE_ARGS_2(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_4(x, ...) , _EA_CAST(x) _EA_PROMOTE_ARGS_3(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_5(x, ...) , _EA_CAST(x) _EA_PROMOTE_ARGS_4(__VA_ARGS__)
#define _EA_PROMOTE_ARGS(N, ...) \
  _EA_CONCATENATE(_EA_PROMOTE_ARGS_,N)(__VA_ARGS__)

#define _EA_PREP_ARGS(...) \
  _EA_PROMOTE_ARGS(_EA_COUNT_ARGS(__VA_ARGS__), ##__VA_ARGS__)

#ifndef __cplusplus
// In C, declare these as non-prototype declarations. This is obsolete K&R C
// (that is still supported) that causes the C frontend to consider any calls
// to them as valid, and avoids using the vararg calling convention.
void emscripten_asm_const();
int emscripten_asm_const_int();
double emscripten_asm_const_double();
#else
// C++ interprets an empty parameter list as a function taking no arguments,
// instead of a K&R C function declaration. Variadic templates are lowered as
// non-vararg calls to the instantiated templated function, which we then
// replace in s2wasm.
template <typename... Args> void emscripten_asm_const(const char* code, Args...);
template <typename... Args> int emscripten_asm_const_int(const char* code, Args...);
template <typename... Args> double emscripten_asm_const_double(const char* code, Args...);
#endif // !__cplusplus

#else // __asmjs
#define _EA_PREP_ARGS(...) , ##__VA_ARGS__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int emscripten_asm_const_int(const char* code, ...);
double emscripten_asm_const_double(const char* code, ...);

int emscripten_asm_const_int_sync_on_main_thread(const char* code, ...);
double emscripten_asm_const_double_sync_on_main_thread(const char* code, ...);

void emscripten_asm_const_async_on_main_thread(const char* code, ...);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __asmjs

// Note: If the code block in the EM_ASM() family of functions below contains a comma,
// then wrap the whole code block inside parentheses (). See tests/core/test_em_asm_2.cpp
// for example code snippets.

// Runs the given JavaScript code on the calling thread (synchronously), and returns no value back.
#define EM_ASM(code, ...) ((void)emscripten_asm_const_int(#code _EA_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code on the calling thread (synchronously), and returns an integer back.
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(#code _EA_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code on the calling thread (synchronously), and returns a double back.
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(#code _EA_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns no value back.
// Call this function for example to access DOM elements in a pthread/web worker. Avoid calling this
// function in performance sensitive code, because this will effectively sleep the calling thread until the
// main browser thread is able to service the proxied function call. If you have multiple MAIN_THREAD_EM_ASM()
// code blocks to call in succession, it will likely be much faster to coalesce all the calls to a single
// MAIN_THREAD_EM_ASM() block. If you do not need synchronization nor a return value back, consider using
// the function MAIN_THREAD_ASYNC_EM_ASM() instead, which will not block.
#define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(#code _EA_PREP_ARGS(__VA_ARGS__)))

// Runs the given JavaScript code synchronously on the main browser thread, and returns an integer back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_INT(code, ...) emscripten_asm_const_int_sync_on_main_thread(#code _EA_PREP_ARGS(__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns a double back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double_sync_on_main_thread(#code _EA_PREP_ARGS(__VA_ARGS__))

// Asynchronously dispatches the given JavaScript code to be run on the main browser thread.
// If the calling thread is the main browser thread, then the specified JavaScript code is executed
// synchronously. Otherwise an event will be queued on the main browser thread to execute the call
// later (think postMessage()), and this call will immediately return without waiting. Be sure to
// guard any accesses to shared memory on the heap inside the JavaScript code with appropriate locking.
#define MAIN_THREAD_ASYNC_EM_ASM(code, ...) ((void)emscripten_asm_const_async_on_main_thread(#code _EA_PREP_ARGS(__VA_ARGS__)))

// Old forms for compatibility, no need to use these.
// Replace EM_ASM_, EM_ASM_ARGS and EM_ASM_INT_V with EM_ASM_INT,
// and EM_ASM_DOUBLE_V with EM_ASM_DOUBLE.
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code _EA_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code _EA_PREP_ARGS(__VA_ARGS__))
#define EM_ASM_INT_V(code) emscripten_asm_const_int(#code)
#define EM_ASM_DOUBLE_V(code) emscripten_asm_const_double(#code)

#endif // __em_asm_h__
