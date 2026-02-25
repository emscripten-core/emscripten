#include "emscripten/wasmfs.h"

// creates a squashfs backend, backed by a squashfs file inside the filesystem
backend_t wasmfs_create_squashfs_backend(const char* squashFSFile __attribute__((nonnull)));
