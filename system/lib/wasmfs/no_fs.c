#include <emscripten.h>
#include <wasi/api.h>

#include "wasmfs_internal.h"

#define WEAK __attribute__((weak))

// Import the outside (JS or VM) fd_write under a different name. We must
// implement __wasi_fd_write in this file so that all the normal WasmFS code is
// not included - that is the point of no-fs mode - since if we don't implement
// it here then the normal WasmFS code for that function will be linked in. But
// in the cases where this file is linked in, we just want to direct all calls
// to the outside fd_write.
__attribute__((import_module("wasi_snapshot_preview1"),
               import_name("fd_write"))) __wasi_errno_t
imported__wasi_fd_write(__wasi_fd_t fd,
                        const __wasi_ciovec_t* iovs,
                        size_t iovs_len,
                        __wasi_size_t* nwritten);

WEAK
__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd,
                               const __wasi_ciovec_t* iovs,
                               size_t iovs_len,
                               __wasi_size_t* nwritten) {
  return imported__wasi_fd_write(fd, iovs, iovs_len, nwritten);
}

// TODO: is this needed in EXIT_RUNTIME?
WEAK
__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat) {
  return __WASI_ERRNO_SUCCESS;
}

// The following is really just needed due to the current architecture of
// WasmFS and how it calls back from JS into wasm. When we remove those calls
// (when we move that code to wasm) it should not be needed.

WEAK
void* _wasmfs_read_file(char* path) {
  return NULL;
}
