// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <dirent.h>
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
  static_assert(sizeof(off_t) == 8, "File offset type must be 64-bit");

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
  uint8_t* result = (uint8_t*)malloc(size + sizeof(size));
  *(off_t*)result = size;

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
// written successfully. If the file already exists, appends to it.
int _wasmfs_write_file(char* pathname, char* data, size_t data_size) {
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
      child = lockedParent.insertDataFile(childName, 0777);
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

  auto lockedFile = dataFile->locked();
  auto offset = lockedFile.getSize();
  auto result = lockedFile.write((uint8_t*)data, data_size, offset);
  if (result != __WASI_ERRNO_SUCCESS) {
    return 0;
  }
  return data_size;
}

int _wasmfs_mkdir(char* path, int mode) {
  return __syscall_mkdirat(AT_FDCWD, (intptr_t)path, mode);
}

int _wasmfs_chdir(char* path) { return __syscall_chdir((intptr_t)path); }

int _wasmfs_symlink(char* old_path, char* new_path) {
  return __syscall_symlink((intptr_t)old_path, (intptr_t)new_path);
}

int _wasmfs_chmod(char* path, mode_t mode) {
  return __syscall_chmod((intptr_t)path, mode);
}

// Helper method that identifies what a path is:
//   ENOENT - if nothing exists there
//   EISDIR - if it is a directory
//   EEXIST - if it is a normal file
int _wasmfs_identify(char* path) {
  struct stat file;
  int err = 0;
  err = stat(path, &file);
  if (err < 0) {
    return ENOENT;
  }
  if (S_ISDIR(file.st_mode)) {
    return EISDIR;
  }
  return EEXIST;
}

struct wasmfs_readdir_state {
  int i;
  int nentries;
  struct dirent** entries;
};

struct wasmfs_readdir_state* _wasmfs_readdir_start(char* path) {
  struct dirent** entries;
  int nentries = scandir(path, &entries, NULL, alphasort);
  if (nentries == -1) {
    return NULL;
  }
  struct wasmfs_readdir_state* state =
    (struct wasmfs_readdir_state*)malloc(sizeof(*state));
  if (state == NULL) {
    return NULL;
  }
  state->i = 0;
  state->nentries = nentries;
  state->entries = entries;
  return state;
}

const char* _wasmfs_readdir_get(struct wasmfs_readdir_state* state) {
  if (state->i < state->nentries) {
    return state->entries[state->i++]->d_name;
  }
  return NULL;
}

void _wasmfs_readdir_finish(struct wasmfs_readdir_state* state) {
  for (int i = 0; i < state->nentries; i++) {
    free(state->entries[i]);
  }
  free(state->entries);
  free(state);
}

} // extern "C"
