//
// "No-FS" implementation for WasmFS. This file implements just enough of the
// syscall API to support printf. In particular, enough for a program that
// only does some file writes, but no open() or such. Then those writes must be
// to the standard streams, and we can implement very simple printing for them
// as opposed to linking in all of WasmFS.
//
// This uses weak linking, and as such it depends on all the normal syscalls
// being implemented in a single file, which is the case (syscalls.cpp). That
// way, if that single other file is included then all these methods get
// overridden. (Otherwise, if we had multiple files, we could end up with a mix
// of this file's functions and others.)
//

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

WEAK
__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  // The only possible file descriptors are the standard streams, and there is
  // nothing special to do to close them.
  return __WASI_ERRNO_SUCCESS;
}

WEAK
__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat) {
  // This operation is not supported (but it does appear in system libraries
  // even in hello world, even if it isn't actually called, so we do need to
  // implement this stub here).
  return __WASI_ERRNO_NOSYS;
}
