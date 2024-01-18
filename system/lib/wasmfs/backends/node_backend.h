// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <sys/stat.h> // for mode_t

extern "C" {

// These helper functions are defined in library_wasmfs_node.js.

// Fill `entries` and return 0 or an error code.
int _wasmfs_node_readdir(const char* path, void* entries
                         /* std::vector<Directory::Entry>*/);
// Write `mode` and return 0 or an error code.
int _wasmfs_node_get_mode(const char* path, mode_t* mode);

// Write `size` and return 0 or an error code.
int _wasmfs_node_stat_size(const char* path, uint32_t* size);
int _wasmfs_node_fstat_size(int fd, uint32_t* size);

// Create a new file system entry and return 0 or an error code.
int _wasmfs_node_insert_file(const char* path, mode_t mode);
int _wasmfs_node_insert_directory(const char* path, mode_t mode);

// Unlink the given file and return 0 or an error code.
int _wasmfs_node_unlink(const char* path);
int _wasmfs_node_rmdir(const char* path);

// Open the file and return the underlying file descriptor.
[[nodiscard]] int _wasmfs_node_open(const char* path, const char* mode);

// Close the underlying file descriptor.
[[nodiscard]] int _wasmfs_node_close(int fd);

// Read up to `size` bytes into `buf` from position `pos` in the file, writing
// the number of bytes read to `nread`. Return 0 on success or an error code.
int _wasmfs_node_read(
  int fd, void* buf, uint32_t len, uint32_t pos, uint32_t* nread);

// Write up to `size` bytes from `buf` at position `pos` in the file, writing
// the number of bytes written to `nread`. Return 0 on success or an error code.
int _wasmfs_node_write(
  int fd, const void* buf, uint32_t len, uint32_t pos, uint32_t* nwritten);
}
