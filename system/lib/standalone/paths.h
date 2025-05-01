#ifndef STANDALONE_PATHS_H
#define STANDALONE_PATHS_H

#include <stdbool.h>

//
// Resolve a (dirfd, relative/absolute path) pair.
//
// Arguments:
// - `resolved_dirfd`:
//   - as input: input dirfd, may be `AT_FDCWD`
//   - as output: resolved dirfd (which always is a preopened fd)
// - `path_ptr`:
//   - as input: pointer to a relative or absolute path
//   - as output: a path relative to `resolved_dirfd`
//
// Returns: `true` if resolution was successful, `false` otherwise.
//
bool __paths_resolve_path(int* resolved_dirfd, const char** path_ptr);

#endif
