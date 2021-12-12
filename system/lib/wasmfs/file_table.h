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

// Access mode, file creation and file status flags for open.
using oflags_t = uint32_t;

class OpenFileState : public std::enable_shared_from_this<OpenFileState> {
  std::shared_ptr<File> file;
  off_t position;
  oflags_t flags; // RD_ONLY, WR_ONLY, RDWR
  // An OpenFileState needs a mutex if there are concurrent accesses on one open
  // file descriptor. This could occur if there are multiple seeks on the same
  // open file descriptor.
  std::recursive_mutex mutex;

public:
  OpenFileState(size_t position, oflags_t flags, std::shared_ptr<File> file)
    : position(position), flags(flags), file(file) {}

  class Handle {
    std::shared_ptr<OpenFileState> openFileState;
    std::unique_lock<std::recursive_mutex> lock;

  public:
    Handle(std::shared_ptr<OpenFileState> openFileState)
      : openFileState(openFileState), lock(openFileState->mutex) {}

    std::shared_ptr<File>& getFile() { return openFileState->file; };

    off_t& position() { return openFileState->position; };
  };
};

class FileTable {
  // FileTable's constructor is private so WasmFS must be a friend class to
  // allow a global FileTable singleton to be defined in the WasmFS object.
  friend class WasmFS;

  std::vector<std::shared_ptr<OpenFileState>> entries;
  std::recursive_mutex mutex;

  FileTable();

public:
  class Entry;

  // Handle represents an RAII wrapper object. Access to the global FileTable
  // must go through a Handle. A Handle holds the single global FileTable's lock
  // for the duration of its lifetime. This is necessary because a FileTable may
  // have atomic operations where the lock must be held across multiple methods.
  // By providing access through the handle, callers of file table methods do
  // not need to remember to take a lock for every access.
  class Handle {
  protected:
    FileTable& fileTable;
    std::unique_lock<std::recursive_mutex> lock;

  public:
    Handle(FileTable& fileTable)
      : fileTable(fileTable), lock(fileTable.mutex) {}

    Entry operator[](__wasi_fd_t fd);

    __wasi_fd_t add(std::shared_ptr<OpenFileState> openFileState);
  };
};

// The Entry class abstracts over the list of entries, providing a simple
// and safe interface that looks much like accessing a std::map, in that
// table[x] = y will allocate a new entry if one is not already present
// there. One minor difference from std::map is that table[x] does not
// return a reference, and can be used to check for the lack of an item
// there without allocation (similar to how table[x] works on a JS object),
// which keeps syntax concise.
class FileTable::Entry : public FileTable::Handle {
  // Handle must be a friend to access the private constructor of Entry.
  friend FileTable::Handle;

public:
  operator std::shared_ptr<OpenFileState>() const;

  Entry& operator=(std::shared_ptr<OpenFileState> ptr);

  Entry& operator=(Entry& entry) {
    return *this = std::shared_ptr<OpenFileState>(entry);
  }

  // Return a locked Handle to access OpenFileState members.
  OpenFileState::Handle locked() {
    assert(fd < fileTable.entries.size() && fd >= 0);
    return OpenFileState::Handle(unlocked());
  }

  // Return an OpenFileState without member access.
  std::shared_ptr<OpenFileState> unlocked();

  // Check whether the entry exists (i.e. contains an OpenFileState).
  operator bool() const;

private:
  __wasi_fd_t fd;

  Entry(FileTable& fileTable, __wasi_fd_t fd)
    : FileTable::Handle(fileTable), fd(fd) {}
};
} // namespace wasmfs
