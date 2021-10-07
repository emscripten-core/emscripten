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

class OpenFileInfo {
  std::shared_ptr<File> file;
  __wasi_filedelta_t offset;
  std::mutex mutex;

public:
  OpenFileInfo(uint32_t offset, std::shared_ptr<File> file) : offset(offset), file(file) {}

  class Handle {
    OpenFileInfo& openFileInfo;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(OpenFileInfo& openFileInfo) : openFileInfo(openFileInfo), lock(openFileInfo.mutex) {}

    std::shared_ptr<File>& getFile();
  };

  Handle get() { return Handle(*this); }
};

class FileTable {
  static std::vector<std::shared_ptr<OpenFileInfo>> entries;
  std::mutex mutex;

  FileTable();

public:
  // Handle represents an RAII wrapper object. Access to the global FileTable must go through a
  // Handle. A Handle holds the single global FileTable's lock for the duration of its lifetime.
  // This is necessary because a FileTable may have atomic oeprations where the lock must be held
  // across multiple methods. By providing access through the handle, callers of file table methods
  // do not need to remember to take a lock for every access.
  class Handle {
    FileTable& fileTable;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(FileTable& fileTable) : fileTable(fileTable), lock(fileTable.mutex) {}

    // Adds given OpenFileInfo to FileTable entries. Returns fd (insertion index in entries).
    __wasi_fd_t add(std::shared_ptr<OpenFileInfo> ptr);

    // Removes OpenFileInfo in FileTable entries corresponding to given fd.
    void remove(__wasi_fd_t fd);

    // Entry is used to override the subscript [] operator.
    // This allows the user to get and set values in the FileTable entries vector.
    struct Entry {
      // Need to store a reference to the single global filetable, which is a local static variable.
      Handle& fileTableHandle;
      __wasi_fd_t fd;

      operator std::shared_ptr<OpenFileInfo>() const;

      Entry& operator=(std::shared_ptr<OpenFileInfo> ptr);

      Entry& operator=(Entry& entry) { return *this = std::shared_ptr<OpenFileInfo>(entry); }

      std::shared_ptr<OpenFileInfo>& operator->();

      // Check whether the entry exists (i.e. contains an OpenFileInfo).
      operator bool() const;
    };

    Entry operator[](__wasi_fd_t fd) { return Entry{*this, fd}; };
  };

  // This get method is responsible for lazily initializing the FileTable.
  // There is only ever one FileTable in the system.
  static Handle get();
};
} // namespace wasmfs
