#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Prints the contents of the whole ASMFS filesystem to console. This can be very slow,
// useful for debugging purposes.
void emscripten_asmfs_dump(void);

// Discards (deletes) the ASMFS virtual filesystem rooted at the given path.
// For example, call emscripten_asmfs_discard_tree("/"); to drop the whole filesystem
// from memory after page load is complete, if none of the files are needed any more.
// Even though this is essentially the same as unlink(), this functionality is offered
// under a separate API name, since doing a unlink("/"); in any application is a
// portability timebomb waiting to happen.
void emscripten_asmfs_discard_tree(const char *path);

#ifdef __cplusplus
}
#endif
