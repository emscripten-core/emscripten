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

std::shared_ptr<Directory> getRootDirectory();

class OpenFileState : public std::enable_shared_from_this<OpenFileState> {
  std::shared_ptr<File> file;
  __wasi_filedelta_t offset;
  // An OpenFileState needs a mutex if there are concurrent accesses on one open
  // file descriptor. This could occur if there are multiple seeks on the same
  // open file descriptor.
  std::mutex mutex;

public:
  OpenFileState(uint32_t offset, std::shared_ptr<File> file)
    : offset(offset), file(file) {}

  class Handle {
    std::shared_ptr<OpenFileState> openFileState;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(std::shared_ptr<OpenFileState> openFileState)
      : openFileState(openFileState), lock(openFileState->mutex) {}

    std::shared_ptr<File>& getFile() { return openFileState->file; };
  };

  Handle get() { return Handle(shared_from_this()); }
};

class FileTable {
  static std::vector<std::shared_ptr<OpenFileState>> entries;
  std::mutex mutex;

  FileTable();

public:
  // Handle represents an RAII wrapper object. Access to the global FileTable
  // must go through a Handle. A Handle holds the single global FileTable's lock
  // for the duration of its lifetime. This is necessary because a FileTable may
  // have atomic operations where the lock must be held across multiple methods.
  // By providing access through the handle, callers of file table methods do
  // not need to remember to take a lock for every access.
  class Handle {
    FileTable& fileTable;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(FileTable& fileTable)
      : fileTable(fileTable), lock(fileTable.mutex) {}

    // The Entry class abstracts over the list of entries, providing a simple
    // and safe interface that looks much like accessing a std::map, in that
    // table[x] = y will allocate a new entry if one is not already present
    // there. One minor difference from std::map is that table[x] does not
    // return a reference, and can be used to check for the lack of an item
    // there without allocation (similar to how table[x] works on a JS object),
    // which keeps syntax concise.
    struct Entry {
      // Need to store a reference to the single global filetable, which is a
      // local static variable.
      Handle& fileTableHandle;
      __wasi_fd_t fd;

      operator std::shared_ptr<OpenFileState>() const;

      Entry& operator=(std::shared_ptr<OpenFileState> ptr);

      Entry& operator=(Entry& entry) {
        return *this = std::shared_ptr<OpenFileState>(entry);
      }

      // Return a locked Handle to access OpenFileState members.
      OpenFileState::Handle locked() {
        assert(fd < fileTableHandle.fileTable.entries.size() && fd > 0);
        return unlocked()->get();
      }

      // Return an OpenFileState without member access.
      std::shared_ptr<OpenFileState> unlocked();

      // Check whether the entry exists (i.e. contains an OpenFileState).
      operator bool() const;
    };

    Entry operator[](__wasi_fd_t fd) { return Entry{*this, fd}; };

    __wasi_fd_t add(std::shared_ptr<OpenFileState> openFileState);
  };

  // This get method is responsible for lazily initializing the FileTable.
  // There is only ever one FileTable in the system.
  static Handle get();
};
} // namespace wasmfs
