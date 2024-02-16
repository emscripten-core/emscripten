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

class FileTable;

class OpenFileState : public std::enable_shared_from_this<OpenFileState> {
  std::shared_ptr<File> file;
  off_t position = 0;
  oflags_t flags; // RD_ONLY, WR_ONLY, RDWR

  // An OpenFileState needs a mutex if there are concurrent accesses on one open
  // file descriptor. This could occur if there are multiple seeks on the same
  // open file descriptor.
  std::recursive_mutex mutex;

  // The number of times this OpenFileState appears in the table. Use this
  // instead of shared_ptr::use_count to avoid accidentally counting temporary
  // objects.
  int uses = 0;

  // We can't make the constructor private because std::make_shared needs to be
  // able to call it, but we can make it unusable publicly.
  struct private_key {
    explicit private_key(int) {}
  };

  // `uses` is protected by the FileTable lock and can be accessed directly by
  // `FileTable::Handle.
  friend FileTable;

public:
  // Cache directory entries at the moment the directory is opened so that
  // subsequent getdents calls have a stable view of the contents. Including
  // files removed after the open and excluding files added after the open is
  // allowed, and trying to recalculate the directory contents on each getdents
  // call could lead to missed directory entries if there are concurrent
  // deletions that effectively move entries back past the current read position
  // in the open directory.
  const std::vector<Directory::Entry> dirents;

  OpenFileState(private_key,
                oflags_t flags,
                std::shared_ptr<File> file,
                std::vector<Directory::Entry>&& dirents)
    : file(file), flags(flags), dirents(std::move(dirents)) {}

  [[nodiscard]] static int create(std::shared_ptr<File> file,
                                  oflags_t flags,
                                  std::shared_ptr<OpenFileState>& out);

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

    // Set the table slot at `fd` to the given file. If this overwrites the last
    // reference to an OpenFileState for a data file in the table, return the
    // file so it can be closed by the caller. Do not close the file directly in
    // this method so it can be closed later while the FileTable lock is not
    // held.
    [[nodiscard]] std::shared_ptr<DataFile>
    setEntry(__wasi_fd_t fd, std::shared_ptr<OpenFileState> openFile);
    __wasi_fd_t addEntry(std::shared_ptr<OpenFileState> openFileState);
  };

  Handle locked() { return Handle(*this); }
};

} // namespace wasmfs
