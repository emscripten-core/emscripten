/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

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

#include "em_asm.h"
#include "em_macros.h"
#include "em_types.h"
#include "em_js.h"
#include "promise.h"
#include "wget.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

void emscripten_run_script(const char *script __attribute__((nonnull)));
int emscripten_run_script_int(const char *script __attribute__((nonnull)));
char *emscripten_run_script_string(const char *script __attribute__((nonnull)));
void emscripten_async_run_script(const char *script __attribute__((nonnull)), int millis);
void emscripten_async_load_script(const char *script __attribute__((nonnull)), em_callback_func onload, em_callback_func onerror);

void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);

#define EM_TIMING_SETTIMEOUT 0
#define EM_TIMING_RAF 1
#define EM_TIMING_SETIMMEDIATE 2

int emscripten_set_main_loop_timing(int mode, int value);
void emscripten_get_main_loop_timing(int *mode, int *value); // Pass a null pointer to skip receiving that particular value
void emscripten_set_main_loop_arg(em_arg_callback_func func, void *arg, int fps, int simulate_infinite_loop);
void emscripten_pause_main_loop(void);
void emscripten_resume_main_loop(void);
void emscripten_cancel_main_loop(void);

typedef void (*em_socket_callback)(int fd, void *userData);
typedef void (*em_socket_error_callback)(int fd, int err, const char* msg, void *userData);

void emscripten_set_socket_error_callback(void *userData, em_socket_error_callback callback);
void emscripten_set_socket_open_callback(void *userData, em_socket_callback callback);
void emscripten_set_socket_listen_callback(void *userData, em_socket_callback callback);
void emscripten_set_socket_connection_callback(void *userData, em_socket_callback callback);
void emscripten_set_socket_message_callback(void *userData, em_socket_callback callback);
void emscripten_set_socket_close_callback(void *userData, em_socket_callback callback);

void _emscripten_push_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name);
void _emscripten_push_uncounted_main_loop_blocker(em_arg_callback_func func, void *arg, const char *name);
#define emscripten_push_main_loop_blocker(func, arg) \
  _emscripten_push_main_loop_blocker(func, arg, #func)
#define emscripten_push_uncounted_main_loop_blocker(func, arg) \
  _emscripten_push_uncounted_main_loop_blocker(func, arg, #func)

void emscripten_set_main_loop_expected_blockers(int num);

void emscripten_async_call(em_arg_callback_func func, void *arg, int millis);

void emscripten_exit_with_live_runtime(void) __attribute__((__noreturn__));
void emscripten_force_exit(int status) __attribute__((__noreturn__));

double emscripten_get_device_pixel_ratio(void);

char *emscripten_get_window_title(void);
void emscripten_set_window_title(const char *);
void emscripten_get_screen_size(int *width __attribute__((nonnull)), int *height __attribute__((nonnull)));
void emscripten_hide_mouse(void);
void emscripten_set_canvas_size(int width, int height) __attribute__((deprecated("This variant does not allow specifying the target canvas", "Use emscripten_set_canvas_element_size() instead")));
void emscripten_get_canvas_size(int *width __attribute__((nonnull)), int *height __attribute__((nonnull)), int *isFullscreen __attribute__((nonnull))) __attribute__((deprecated("This variant does not allow specifying the target canvas", "Use emscripten_get_canvas_element_size() and emscripten_get_fullscreen_status() instead")));

double emscripten_get_now(void);
float emscripten_random(void);

// IDB

typedef void (*em_idb_onload_func)(void*, void*, int);
void emscripten_idb_async_load(const char *db_name __attribute__((nonnull)), const char *file_id __attribute__((nonnull)), void* arg, em_idb_onload_func onload, em_arg_callback_func onerror);
void emscripten_idb_async_store(const char *db_name __attribute__((nonnull)), const char *file_id __attribute__((nonnull)), void* ptr, int num, void* arg, em_arg_callback_func onstore, em_arg_callback_func onerror);
void emscripten_idb_async_delete(const char *db_name __attribute__((nonnull)), const char *file_id __attribute__((nonnull)), void* arg, em_arg_callback_func ondelete, em_arg_callback_func onerror);
typedef void (*em_idb_exists_func)(void*, int);
void emscripten_idb_async_exists(const char *db_name __attribute__((nonnull)), const char *file_id __attribute__((nonnull)), void* arg, em_idb_exists_func oncheck, em_arg_callback_func onerror);
void emscripten_idb_async_clear(const char *db_name __attribute__((nonnull)), void* arg, em_arg_callback_func onclear, em_arg_callback_func onerror);

// IDB "sync"

void emscripten_idb_load(const char *db_name, const char *file_id, void** pbuffer, int* pnum, int *perror);
void emscripten_idb_store(const char *db_name, const char *file_id, void* buffer, int num, int *perror);
void emscripten_idb_delete(const char *db_name, const char *file_id, int *perror);
void emscripten_idb_exists(const char *db_name, const char *file_id, int* pexists, int *perror);
void emscripten_idb_clear(const char *db_name, int *perror);

void emscripten_idb_load_blob(const char *db_name, const char *file_id, int* pblob, int *perror);
void emscripten_idb_store_blob(const char *db_name, const char *file_id, void* buffer, int num, int *perror);
void emscripten_idb_read_from_blob(int blob, int start, int num, void* buffer);
void emscripten_idb_free_blob(int blob);

// other async utilities

int emscripten_run_preload_plugins(const char* file, em_str_callback_func onload, em_str_callback_func onerror);

typedef void (*em_run_preload_plugins_data_onload_func)(void*, const char*);
void emscripten_run_preload_plugins_data(char* data, int size, const char *suffix, void *arg, em_run_preload_plugins_data_onload_func onload, em_arg_callback_func onerror);

void emscripten_lazy_load_code(void);

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

long emscripten_get_compiler_setting(const char *name);
int emscripten_has_asyncify(void);

void emscripten_debugger(void);

// Forward declare FILE from musl libc headers to avoid needing to #include <stdio.h> from emscripten.h
struct _IO_FILE;
typedef struct _IO_FILE FILE;

char *emscripten_get_preloaded_image_data(const char *path, int *w, int *h);
char *emscripten_get_preloaded_image_data_from_FILE(FILE *file, int *w, int *h);

#define EM_LOG_CONSOLE   1
#define EM_LOG_WARN      2
#define EM_LOG_ERROR     4
#define EM_LOG_C_STACK   8
#define EM_LOG_JS_STACK 16
#define EM_LOG_DEMANGLE 32  // deprecated
#pragma clang deprecated(EM_LOG_DEMANGLE)
#define EM_LOG_NO_PATHS 64
#define EM_LOG_FUNC_PARAMS 128  // deprecated
#pragma clang deprecated(EM_LOG_FUNC_PARAMS)
#define EM_LOG_DEBUG    256
#define EM_LOG_INFO     512

void emscripten_log(int flags, const char* format, ...);

int emscripten_get_callstack(int flags, char *out, int maxbytes);

int emscripten_print_double(double x, char *to, signed max);

typedef void (*em_scan_func)(void*, void*);
void emscripten_scan_registers(em_scan_func func);
void emscripten_scan_stack(em_scan_func func);

// Asynchronous version of dlopen.  Since WebAssembly module loading in general
// is asynchronous the normal dlopen function can't be used in all situations.
typedef void (*em_dlopen_callback)(void* handle, void* user_data);
void emscripten_dlopen(const char *filename, int flags, void* user_data, em_dlopen_callback onsuccess, em_arg_callback_func onerror);

// Promisified version of emscripten_dlopen
// The returned promise will resolve once the dso has been loaded.  Its up to
// the caller to call emscripten_promise_destroy on this promise.
em_promise_t emscripten_dlopen_promise(const char *filename, int flags);

void emscripten_throw_number(double number);
void emscripten_throw_string(const char *utf8String);

/* ===================================== */
/* Internal APIs. Be careful with these. */
/* ===================================== */

void emscripten_sleep(unsigned int ms);

#ifdef __cplusplus
}
#endif
