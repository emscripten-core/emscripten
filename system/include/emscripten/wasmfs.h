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
backend_t wasmfs_get_backend_by_path(char* path);

// Obtains the backend_t of a specified fd.
backend_t wasmfs_get_backend_by_fd(int fd);

// Creates a JSFile Backend in the new file system.
backend_t wasmfs_create_js_file_backend();

// Creates a Proxied Backend in the new file system.
backend_t wasmfs_create_proxied_backend(backend_t backend);

// Creates a new file in the new file system under a specific backend.
uint32_t wasmfs_create_file(char* pathname, mode_t mode, backend_t backend);

// Creates a new directory in the new file system under a specific backend.
long wasmfs_create_directory(char* path, long mode, backend_t backend);

#ifdef __cplusplus
}
#endif
