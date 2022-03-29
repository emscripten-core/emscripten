// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <syscall_arch.h>
#include <unistd.h>

#include "backend.h"
#include "file.h"
#include "paths.h"

using namespace wasmfs;

extern "C" {

__wasi_fd_t wasmfs_create_file(char* pathname, mode_t mode, backend_t backend);

// Copy the file specified by the pathname into JS.
// Return a pointer to the JS buffer in HEAPU8.
// The buffer will also contain the file length.
// Caller must free the returned pointer.
void* _wasmfs_read_file(char* path) {
  struct stat file;
  int err = 0;
  err = stat(path, &file);
  if (err < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }

  // The function will return a pointer to a buffer with the file length in the
  // first 8 bytes. The remaining bytes will contain the buffer contents. This
  // allows the caller to use HEAPU8.subarray(buf + 8, buf + 8 + length).
  off_t size = file.st_size;
  uint8_t* result = (uint8_t*)malloc((size + sizeof(size)));
  *(uint32_t*)result = size;

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }
  int numRead = pread(fd, result + sizeof(size), size, 0);
  // TODO: Generalize this so that it is thread-proof.
  // Must guarantee that the file size has not changed by the time it is read.
  assert(numRead == size);
  err = close(fd);
  if (err < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }

  return result;
}

// Writes to a file, possibly creating it, and returns the number of bytes
// written successfully.
long _wasmfs_write_file(char* pathname, char* data, size_t data_size) {
  auto parsedParent = path::parseParent(pathname);
  if (parsedParent.getError()) {
    return 0;
  }
  auto& [parent, childNameView] = parsedParent.getParentChild();
  std::string childName(childNameView);

  std::shared_ptr<File> child;
  {
    auto lockedParent = parent->locked();
    child = lockedParent.getChild(childName);
    if (!child) {
      // Lookup failed; try creating the file.
      child = parent->getBackend()->createFile(0777);
      child = lockedParent.insertChild(childName, child);
      if (!child) {
        // File creation failed; nothing else to do.
        return 0;
      }
    }
  }

  auto dataFile = child->dynCast<DataFile>();
  if (!dataFile) {
    // There is something here but it isn't a data file.
    return 0;
  }

  auto result = dataFile->locked().write((uint8_t*)data, data_size, 0);
  if (result != __WASI_ERRNO_SUCCESS) {
    return 0;
  }
  return data_size;
}

long _wasmfs_mkdir(char* path, long mode) {
  return __syscall_mkdir((intptr_t)path, mode);
}

long _wasmfs_chdir(char* path) { return __syscall_chdir((intptr_t)path); }

void _wasmfs_symlink(char* old_path, char* new_path) {
  __syscall_symlink((intptr_t)old_path, (intptr_t)new_path);
}

long _wasmfs_chmod(char* path, mode_t mode) {
  return __syscall_chmod((intptr_t)path, mode);
}
}
