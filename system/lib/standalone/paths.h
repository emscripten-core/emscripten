#ifndef STANDALONE_PATHS_H
#define STANDALONE_PATHS_H

#include <stdbool.h>
#include <stddef.h>

#include <wasi/api.h>

//
// Resolve a (dirfd, relative/absolute path) pair.
//
// Arguments:
// - `need_unlock`: Is set by the function. If `true`, must make sure to call
//   `__paths_unlock()` when done with the `resolved_dirfd`.
// - `resolved_dirfd`:
//   - as input: input dirfd, may be `AT_FDCWD`
//   - as output: resolved dirfd, may be either a preopened fd or a fd
//     representing the cwd.
// - `path_ptr`:
//   - as input: pointer to a relative or absolute path
//   - as output: a path relative to `resolved_dirfd`
//
// Returns: `true` if resolution was successful, `false` otherwise.
//
bool __paths_resolve_path(bool* need_unlock,
                          int* resolved_dirfd,
                          const char** path_ptr);

// Must be called by the user when `need_unlock` of `__paths_resolve_path()`
// was true, and `resolved_dirfd` is no longer needed.
void __paths_unlock();

// Changes the current working directory to `path`, which may be either an
// absolute path or a path relative to the current working directory.
__wasi_errno_t __paths_chdir(const char* path);

// Changes the current working directory to the directory represented by `fd`.
__wasi_errno_t __paths_fchdir(int fd);

// Puts a string representing the current working directory into the buffer
// `buf`. `size` is an in/out parameter: input is the size of the buffer `buf`,
// output is the number of characters written into `buf`, including a
// terminating zero.
__wasi_errno_t __paths_getcwd(char* buf, size_t* size);

#endif
