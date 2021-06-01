/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __wasi_emscripten_helpers_h
#define __wasi_emscripten_helpers_h

#ifdef __cplusplus
extern "C" {
#endif

// Converts a wasi return code to a musl syscall return code (-1 if
// error, 0 otherwise), and sets errno accordingly.
int __wasi_syscall_ret(__wasi_errno_t code);

// Check if a wasi file descriptor is valid, returning 1 if valid and 0 if
// not. If not, also sets errno to EBADF.
int __wasi_fd_is_valid(__wasi_fd_t fd);

struct timespec __wasi_timestamp_to_timespec(__wasi_timestamp_t timestamp);

#ifdef __cplusplus
}
#endif

#endif // __wasi_emscripten_helpers_h
