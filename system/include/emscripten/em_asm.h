#ifndef __em_asm_h__
#define __em_asm_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __asmjs
#define _EA_COUNT_ARGS_EXP(a,b,c,d,e,n,...) n
#define _EA_COUNT_ARGS(...) _EA_COUNT_ARGS_EXP(__VA_ARGS__,5,4,3,2,1,0)

#define _EA_PROMOTE_ARGS_0()
#define _EA_PROMOTE_ARGS_1(x, ...) (double)(x)
#define _EA_PROMOTE_ARGS_2(x, ...) (double)(x), _EA_PROMOTE_ARGS_1(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_3(x, ...) (double)(x), _EA_PROMOTE_ARGS_2(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_4(x, ...) (double)(x), _EA_PROMOTE_ARGS_3(__VA_ARGS__)
#define _EA_PROMOTE_ARGS_5(x, ...) (double)(x), _EA_PROMOTE_ARGS_4(__VA_ARGS__)
#define _EA_PROMOTE_ARGS(...) \
  _EA_CONCATENATE(_EA_PROMOTE_ARGS_,_EA_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define _EA_PREP_ARGS(code, ...) \
  #code, COUNT_ARGS(__VA_ARGS__), PROMOTE_ARGS(__VA_ARGS__)

int emscripten_asm_const_int(const char* code, int nargs, ...);
double emscripten_asm_const_double(const char* code, int nargs, ...);

#else // __asmjs
#define _EA_PREP_ARGS(code, ...) #code, ##__VA_ARGS__

int emscripten_asm_const_int(const char* code, ...);
double emscripten_asm_const_double(const char* code, ...);

int emscripten_asm_const_int_sync_on_main_thread(const char* code, ...);
double emscripten_asm_const_double_sync_on_main_thread(const char* code, ...);

void emscripten_asm_const_async_on_main_thread(const char* code, ...);

#endif // __asmjs

#ifdef __cplusplus
}
#endif

// Note: If the code block in the EM_ASM() family of functions below contains a comma,
// then wrap the whole code block inside parentheses (). See tests/core/test_em_asm_2.cpp
// for example code snippets.

// Runs the given JavaScript code on the calling thread (synchronously), and returns no value back.
#define EM_ASM(code, ...) ((void)emscripten_asm_const_int(_EA_PREP_ARGS(code, __VA_ARGS__)))

// Runs the given JavaScript code on the calling thread (synchronously), and returns an integer back.
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(_EA_PREP_ARGS(code, __VA_ARGS__))

// Runs the given JavaScript code on the calling thread (synchronously), and returns a double back.
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(_EA_PREP_ARGS(code, __VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns no value back.
// Call this function for example to access DOM elements in a pthread/web worker. Avoid calling this
// function in performance sensitive code, because this will effectively sleep the calling thread until the
// main browser thread is able to service the proxied function call. If you have multiple MAIN_THREAD_EM_ASM()
// code blocks to call in succession, it will likely be much faster to coalesce all the calls to a single
// MAIN_THREAD_EM_ASM() block. If you do not need synchronization nor a return value back, consider using
// the function MAIN_THREAD_ASYNC_EM_ASM() instead, which will not block.
#define MAIN_THREAD_EM_ASM(code, ...) ((void)emscripten_asm_const_int_sync_on_main_thread(#code, ##__VA_ARGS__))

// Runs the given JavaScript code synchronously on the main browser thread, and returns an integer back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_INT(code, ...) emscripten_asm_const_int_sync_on_main_thread(#code, ##__VA_ARGS__)

// Runs the given JavaScript code synchronously on the main browser thread, and returns a double back.
// The same considerations apply as with MAIN_THREAD_EM_ASM().
#define MAIN_THREAD_EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double_sync_on_main_thread(#code, ##__VA_ARGS__)

// Asynchronously dispatches the given JavaScript code to be run on the main browser thread.
// If the calling thread is the main browser thread, then the specified JavaScript code is executed
// synchronously. Otherwise an event will be queued on the main browser thread to execute the call
// later (think postMessage()), and this call will immediately return without waiting. Be sure to
// guard any accesses to shared memory on the heap inside the JavaScript code with appropriate locking.
#define MAIN_THREAD_ASYNC_EM_ASM(code, ...) ((void)emscripten_asm_const_async_on_main_thread(#code, ##__VA_ARGS__))

// Old forms for compatibility, no need to use these.
// Replace EM_ASM_, EM_ASM_ARGS and EM_ASM_INT_V with EM_ASM_INT,
// and EM_ASM_DOUBLE_V with EM_ASM_DOUBLE.
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_INT_V(code) emscripten_asm_const_int(#code)
#define EM_ASM_DOUBLE_V(code) emscripten_asm_const_double(#code)

#endif // __em_asm_h__
