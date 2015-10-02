#ifndef __emscripten_h__
#define __emscripten_h__

/**
 * This file contains a few useful things for compiling C/C++ code
 * with Emscripten.
 *
 * Documentation for the public APIs defined in this file must be updated in: 
 *    site/source/docs/api_reference/emscripten.h.rst
 * A prebuilt local version of the documentation is available at: 
 *    site/build/text/docs/api_reference/emscripten.h.txt
 * You can also build docs locally as HTML or other formats in site/
 * An online HTML version (which may be of a different version of Emscripten)
 *    is up at http://kripken.github.io/emscripten-site/docs/api_reference/emscripten.h.html
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#if !__EMSCRIPTEN__
#include <SDL/SDL.h> /* for SDL_Delay in async_call */
#endif

#if __EMSCRIPTEN__
// This version of emscripten has <emscripten/vr.h> and associated support
#define EMSCRIPTEN_HAS_VR_SUPPORT 1
#endif

/* Typedefs */

typedef short __attribute__((aligned(1))) emscripten_align1_short;

typedef long long __attribute__((aligned(4))) emscripten_align4_int64;
typedef long long __attribute__((aligned(2))) emscripten_align2_int64;
typedef long long __attribute__((aligned(1))) emscripten_align1_int64;

typedef int __attribute__((aligned(2))) emscripten_align2_int;
typedef int __attribute__((aligned(1))) emscripten_align1_int;

typedef float __attribute__((aligned(2))) emscripten_align2_float;
typedef float __attribute__((aligned(1))) emscripten_align1_float;

typedef double __attribute__((aligned(4))) emscripten_align4_double;
typedef double __attribute__((aligned(2))) emscripten_align2_double;
typedef double __attribute__((aligned(1))) emscripten_align1_double;

typedef void (*em_callback_func)(void);
typedef void (*em_arg_callback_func)(void*);
typedef void (*em_str_callback_func)(const char *);


#define EM_ASM(...) emscripten_asm_const(#__VA_ARGS__)
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(#code, __VA_ARGS__)
#define EM_ASM_INT_V(code) emscripten_asm_const_int(#code)
#define EM_ASM_DOUBLE_V(code) emscripten_asm_const_double(#code)


#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

extern void emscripten_run_script(const char *script);
extern int emscripten_run_script_int(const char *script);
extern char *emscripten_run_script_string(const char *script);
extern void emscripten_async_run_script(const char *script, int millis);
extern void emscripten_async_load_script(const char *script, em_callback_func onload, em_callback_func onerror);

#if __EMSCRIPTEN__
extern void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);

#define EM_TIMING_SETTIMEOUT 0
#define EM_TIMING_RAF 1
#define EM_TIMING_SETIMMEDIATE 2

extern int emscripten_set_main_loop_timing(int mode, int value);
extern void emscripten_get_main_loop_timing(int *mode, int *value);
extern void emscripten_set_main_loop_arg(em_arg_callback_func func, void *arg, int fps, int simulate_infinite_loop);
extern void emscripten_pause_main_loop(void);
extern void emscripten_resume_main_loop(void);
extern void emscripten_cancel_main_loop(void);
#else
#define emscripten_set_main_loop(func, fps, simulateInfiniteLoop) \
  while (1) { func(); usleep(1000000/fps); }
#define emscripten_cancel_main_loop() exit(1);
#endif


typedef void (*em_socket_callback)(int fd, void *userData);
typedef void (*em_socket_error_callback)(int fd, int err, const char* msg, void *userData);

extern void emscripten_set_socket_error_callback(void *userData, em_socket_error_callback callback);
extern void emscripten_set_socket_open_callback(void *userData, em_socket_callback callback);
extern void emscripten_set_socket_listen_callback(void *userData, em_socket_callback callback);
extern void emscripten_set_socket_connection_callback(void *userData, em_socket_callback callback);
extern void emscripten_set_socket_message_callback(void *userData, em_socket_callback callback);
extern void emscripten_set_socket_close_callback(void *userData, em_socket_callback callback);


#if __EMSCRIPTEN__
extern void _emscripten_push_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name);
extern void _emscripten_push_uncounted_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name);
#else
inline void _emscripten_push_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name) {
  func(arg);
}
inline void _emscripten_push_uncounted_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name) {
  func(arg);
}
#endif
#define emscripten_push_main_loop_blocker(func, arg) \
  _emscripten_push_main_loop_blocker(func, arg, #func)
#define emscripten_push_uncounted_main_loop_blocker(func, arg) \
  _emscripten_push_uncounted_main_loop_blocker(func, arg, #func)

#if __EMSCRIPTEN__
extern void emscripten_set_main_loop_expected_blockers(int num);
#else
inline void emscripten_set_main_loop_expected_blockers(int num) {}
#endif


#if __EMSCRIPTEN__
extern void emscripten_async_call(em_arg_callback_func func, void *arg, int millis);
#else
inline void emscripten_async_call(em_arg_callback_func func, void *arg, int millis) {
  if (millis) SDL_Delay(millis);
  func(arg);
}
#endif


extern void emscripten_exit_with_live_runtime(void);
extern void emscripten_force_exit(int status);

double emscripten_get_device_pixel_ratio(void);

void emscripten_hide_mouse(void);
void emscripten_set_canvas_size(int width, int height);
void emscripten_get_canvas_size(int *width, int *height, int *isFullscreen);

#if __EMSCRIPTEN__
double emscripten_get_now(void);
#else
#include <time.h>
static inline double emscripten_get_now(void) {
  return (1000*clock())/(double)CLOCKS_PER_SEC;
}
#endif

float emscripten_random(void);

// wget

void emscripten_async_wget(const char* url, const char* file, em_str_callback_func onload, em_str_callback_func onerror);

typedef void (*em_async_wget_onload_func)(void*, void*, int);
void emscripten_async_wget_data(const char* url, void *arg, em_async_wget_onload_func onload, em_arg_callback_func onerror);

typedef void (*em_async_wget2_onload_func)(unsigned, void*, const char*);
typedef void (*em_async_wget2_onstatus_func)(unsigned, void*, int);

int emscripten_async_wget2(const char* url, const char* file,  const char* requesttype, const char* param, void *arg, em_async_wget2_onload_func onload, em_async_wget2_onstatus_func onerror, em_async_wget2_onstatus_func onprogress);

typedef void (*em_async_wget2_data_onload_func)(unsigned, void*, void*, unsigned);
typedef void (*em_async_wget2_data_onerror_func)(unsigned, void*, int, const char*);
typedef void (*em_async_wget2_data_onprogress_func)(unsigned, void*, int, int);

int emscripten_async_wget2_data(const char* url, const char* requesttype, const char* param, void *arg, int free, em_async_wget2_data_onload_func onload, em_async_wget2_data_onerror_func onerror, em_async_wget2_data_onprogress_func onprogress);

void emscripten_async_wget2_abort(int handle);

// wget "sync" (ASYNCIFY)

void emscripten_wget(const char* url, const char* file);

// wget data "sync" (EMTERPRETIFY_ASYNC)

void emscripten_wget_data(const char* url, void** pbuffer, int* pnum, int *perror);

// IDB

void emscripten_idb_async_load(const char *db_name, const char *file_id, void* arg, em_async_wget_onload_func onload, em_arg_callback_func onerror);
void emscripten_idb_async_store(const char *db_name, const char *file_id, void* ptr, int num, void* arg, em_arg_callback_func onstore, em_arg_callback_func onerror);
void emscripten_idb_async_delete(const char *db_name, const char *file_id, void* arg, em_arg_callback_func ondelete, em_arg_callback_func onerror);
typedef void (*em_idb_exists_func)(void*, int);
void emscripten_idb_async_exists(const char *db_name, const char *file_id, void* arg, em_idb_exists_func oncheck, em_arg_callback_func onerror);

// IDB "sync" (EMTERPRETIFY_ASYNC)

void emscripten_idb_load(const char *db_name, const char *file_id, void** pbuffer, int* pnum, int *perror);
void emscripten_idb_store(const char *db_name, const char *file_id, void* buffer, int num, int *perror);
void emscripten_idb_delete(const char *db_name, const char *file_id, int *perror);
void emscripten_idb_exists(const char *db_name, const char *file_id, int* pexists, int *perror);

void emscripten_idb_load_blob(const char *db_name, const char *file_id, int* pblob, int *perror);
void emscripten_idb_store_blob(const char *db_name, const char *file_id, void* buffer, int num, int *perror);
void emscripten_idb_read_from_blob(int blob, int start, int num, void* buffer);
void emscripten_idb_free_blob(int blob);

// other async utilities

int emscripten_run_preload_plugins(const char* file, em_str_callback_func onload, em_str_callback_func onerror);

typedef void (*em_run_preload_plugins_data_onload_func)(void*, const char*);
void emscripten_run_preload_plugins_data(char* data, int size, const char *suffix, void *arg, em_run_preload_plugins_data_onload_func onload, em_arg_callback_func onerror);

// show an error on some renamed methods
#define emscripten_async_prepare(...) _Pragma("GCC error(\"emscripten_async_prepare has been replaced by emscripten_run_preload_plugins\")")
#define emscripten_async_prepare_data(...) _Pragma("GCC error(\"emscripten_async_prepare_data has been replaced by emscripten_run_preload_plugins_data\")")

// worker APIs

typedef int worker_handle;

worker_handle emscripten_create_worker(const char *url);
void emscripten_destroy_worker(worker_handle worker);

typedef void (*em_worker_callback_func)(char*, int, void*);
void emscripten_call_worker(worker_handle worker, const char *funcname, char *data, int size, em_worker_callback_func callback, void *arg);
void emscripten_worker_respond(char *data, int size);
void emscripten_worker_respond_provisionally(char *data, int size);

int emscripten_get_worker_queue_size(worker_handle worker);

// misc.

int emscripten_get_compiler_setting(const char *name);

void emscripten_debugger();

char *emscripten_get_preloaded_image_data(const char *path, int *w, int *h);
char *emscripten_get_preloaded_image_data_from_FILE(FILE *file, int *w, int *h);

#define EM_LOG_CONSOLE   1
#define EM_LOG_WARN      2
#define EM_LOG_ERROR     4
#define EM_LOG_C_STACK   8
#define EM_LOG_JS_STACK 16
#define EM_LOG_DEMANGLE 32
#define EM_LOG_NO_PATHS 64
#define EM_LOG_FUNC_PARAMS 128

void emscripten_log(int flags, ...);

int emscripten_get_callstack(int flags, char *out, int maxbytes);

int emscripten_print_double(double x, char *to, signed max);

/* ===================================== */
/* Internal APIs. Be careful with these. */
/* ===================================== */

/* Helper API for EM_ASM - do not call this yourself */
void emscripten_asm_const(const char *code);
int emscripten_asm_const_int(const char *code, ...);
double emscripten_asm_const_double(const char *code, ...);

#if __EMSCRIPTEN__
void emscripten_sleep(unsigned int ms);
void emscripten_sleep_with_yield(unsigned int ms);
#else
#define emscripten_sleep SDL_Delay
#endif

typedef void * emscripten_coroutine;
emscripten_coroutine emscripten_coroutine_create(em_arg_callback_func func, void *arg, int stack_size);
int emscripten_coroutine_next(emscripten_coroutine);
void emscripten_yield(void);


#ifdef __cplusplus
}
#endif

#endif // __emscripten_h__

