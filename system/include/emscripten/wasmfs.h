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

// Creates a JSFile Backend in the new file system.
backend_t create_js_file_backend();

// Creates a new file in the new file system under a specific backend.
uint32_t wasmfs_create(char* pathname, mode_t mode, backend_t backend);

// Creates a new directory in the new file system under a specific backend.
long wasmfs_mkdir(char* path, long mode, backend_t backend);

#ifdef __cplusplus
}
#endif
