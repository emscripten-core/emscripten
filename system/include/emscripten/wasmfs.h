/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Backend* backend_t;

// Obtains the backend_t of a specified path.
backend_t wasmfs_get_backend_by_path(char* path __attribute__((nonnull)));

// Obtains the backend_t of a specified fd.
backend_t wasmfs_get_backend_by_fd(int fd);

// Creates and opens a new file in the new file system under a specific backend.
// Returns the file descriptor for the new file like `open`. Returns a negative
// value on error. TODO: It might be worth returning a more specialized type
// like __wasi_fd_t here.
// TODO: Remove this function so that only directories can be mounted.
int wasmfs_create_file(const char* pathname __attribute__((nonnull)), mode_t mode, backend_t backend);

// Creates a new directory in the new file system under a specific backend.
// Returns 0 on success like `mkdir`, or a negative value on error.
// TODO: Add an alias with wasmfs_mount.
int wasmfs_create_directory(const char* path __attribute__((nonnull)), mode_t mode, backend_t backend);

// Unmounts the directory (Which must be a valid mountpoint) at a specific path.
// Returns 0 on success, or a negative value on error.
int wasmfs_unmount(intptr_t path);

// Backend creation

// Creates a JSFile Backend in the new file system.
backend_t wasmfs_create_js_file_backend(void);

// A function that receives a void* and returns a backend.
typedef backend_t (*backend_constructor_t)(void*);

backend_t wasmfs_create_memory_backend(void);

// Note: this cannot be called on the browser main thread because it might
// deadlock while waiting for its dedicated worker thread to be spawned.
//
// Note: This function blocks on the main browser thread returning to its event
// loop. Calling this function while holding a lock the main thread is waiting
// to acquire will cause a deadlock.
//
// TODO: Add an async version of this function that will work on the main
// thread.
backend_t wasmfs_create_fetch_backend(const char* base_url __attribute__((nonnull)));

backend_t wasmfs_create_node_backend(const char* root __attribute__((nonnull)));

// Note: this cannot be called on the browser main thread because it might
// deadlock while waiting for the OPFS dedicated worker thread to be spawned.
//
// Note: This function blocks on the main browser thread returning to its event
// loop. Calling this function while holding a lock the main thread is waiting
// to acquire will cause a deadlock.
//
// TODO: Add an async version of this function that will work on the main
// thread.
backend_t wasmfs_create_opfs_backend(void);

// Creates a generic JSIMPL backend in the new file system.
backend_t wasmfs_create_jsimpl_backend(void);

backend_t wasmfs_create_icase_backend(backend_t backend);

// Similar to fflush(0), but also flushes all internal buffers inside WasmFS.
// This is necessary because in a Web environment we must buffer at an
// additional level after libc, since console.log() prints entire lines, that
// is, we can't print individual characters as libc feeds them to us, so we
// buffer them and call console.log() only after a newline. This function will
// actually flush all buffers and add newlines as necessary to get everything
// printed out.
void wasmfs_flush(void);

// Hooks

// A hook users can do to create the root directory. Overriding this allows the
// user to set a particular backend as the root. If this is not set then the
// default backend is used.
backend_t wasmfs_create_root_dir(void);

// A hook users can do to run code during WasmFS startup. This hook happens
// before file preloading, so user code could create backends and mount them,
// which would then affect in which backend the preloaded files are loaded (the
// preloaded files have paths, and so they are added to that path and whichever
// backend is present there).
void wasmfs_before_preload(void);

#ifdef __cplusplus
}
#endif
