// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include <assert.h>
#include <fcntl.h>
#include <mutex>
#include <utility>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {
static_assert(std::is_same<size_t, __wasi_size_t>::value,
              "size_t should be the same as __wasi_size_t");
static_assert(std::is_same<off_t, __wasi_filedelta_t>::value,
              "off_t should be the same as __wasi_filedelta_t");

// Overflow and underflow behaviour are only defined for unsigned types.
template<typename T> bool addWillOverFlow(T a, T b) {
  if (a > 0 && b > std::numeric_limits<T>::max() - a) {
    return true;
  }
  return false;
}

class OpenFileState : public std::enable_shared_from_this<OpenFileState> {
  std::shared_ptr<File> file;
  off_t position = 0;
  oflags_t flags; // RD_ONLY, WR_ONLY, RDWR

  // An OpenFileState needs a mutex if there are concurrent accesses on one open
  // file descriptor. This could occur if there are multiple seeks on the same
  // open file descriptor.
  std::recursive_mutex mutex;

  // We can't make the constructor private because std::make_shared needs to be
  // able to call it, but we can make it unusable publicly.
  struct private_key {
    explicit private_key(int) {}
  };

public:
  OpenFileState(private_key, oflags_t flags, std::shared_ptr<File> file)
    : flags(flags), file(file) {}

  [[nodiscard]] static int create(std::shared_ptr<File> file,
                                  oflags_t flags,
                                  std::shared_ptr<OpenFileState>& out) {
    assert(file);
    if (auto f = file->dynCast<DataFile>()) {
      if (int err = f->locked().open(flags & O_ACCMODE)) {
        return err;
      }
    }
    out = std::make_shared<OpenFileState>(private_key{0}, flags, file);
    return 0;
  }

  class Handle {
    std::shared_ptr<OpenFileState> openFileState;
    std::unique_lock<std::recursive_mutex> lock;

  public:
    Handle(std::shared_ptr<OpenFileState> openFileState)
      : openFileState(openFileState), lock(openFileState->mutex) {}

    std::shared_ptr<File>& getFile() { return openFileState->file; };

    off_t getPosition() const { return openFileState->position; };
    void setPosition(off_t pos) { openFileState->position = pos; };

    oflags_t getFlags() const { return openFileState->flags; };
    void setFlags(oflags_t flags) { openFileState->flags = flags; };
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class FileTable {
  // Allow WasmFS to construct the FileTable singleton.
  friend class WasmFS;

  std::vector<std::shared_ptr<OpenFileState>> entries;
  std::recursive_mutex mutex;

  FileTable();

public:
  // Access to the FileTable must go through a Handle, which holds its lock.
  class Handle {
    FileTable& fileTable;
    std::unique_lock<std::recursive_mutex> lock;

  public:
    Handle(FileTable& fileTable)
      : fileTable(fileTable), lock(fileTable.mutex) {}

    std::shared_ptr<OpenFileState> getEntry(__wasi_fd_t fd);

    // Set the table slot at `fd` to the given file. Return the result of
    // closing the file that was previously in that entry or 0 if there was no
    // previous file.
    [[nodiscard]] int setEntry(__wasi_fd_t fd,
                               std::shared_ptr<OpenFileState> openFile);
    __wasi_fd_t addEntry(std::shared_ptr<OpenFileState> openFileState);
  };

  Handle locked() { return Handle(*this); }
};

} // namespace wasmfs
