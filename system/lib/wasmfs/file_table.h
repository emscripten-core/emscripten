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

class OpenFileDescriptor {
  std::shared_ptr<File> file;
  __wasi_filedelta_t offset;
  // An OpenFileDescriptor needs a mutex if there are concurrent accesses on one open file
  // descriptor. This could occur if there are multiple seeks on the same open file descriptor.
  std::mutex mutex;

public:
  OpenFileDescriptor(uint32_t offset, std::shared_ptr<File> file) : offset(offset), file(file) {}

  std::shared_ptr<File>& getFile();
};

class FileTable {
  static std::vector<std::shared_ptr<OpenFileDescriptor>> entries;
  std::mutex mutex;

  FileTable() {
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdinFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdoutFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StderrFile>()));
  }

public:
  // Handle represents an RAII wrapper object. Access to the global FileTable must go through a
  // Handle. A Handle holds the single global FileTable's lock for the duration of its lifetime.
  class Handle {
    FileTable& fileTable;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(FileTable& fileTable) : fileTable(fileTable), lock(fileTable.mutex) {}

    // Returns size of the associated FileTable.
    size_t size() { return fileTable.entries.size(); }

    // Adds given OpenFileDescriptor to FileTable entries. Returns fd (insertion index in entries).
    __wasi_fd_t addOpenFile(std::shared_ptr<OpenFileDescriptor> ptr);

    // Removes OpenFileDescriptor in FileTable entries corresponding to given fd.
    void removeOpenFile(__wasi_fd_t fd);

    // Entry is used to override the subscript [] operator.
    // This allows the user to get and set values in the FileTable entries vector, even entries
    // outside the bounds of the FileTable.
    struct Entry {
      Handle& fileTableHandle;
      __wasi_fd_t fd;
      operator std::shared_ptr<OpenFileDescriptor>&() const {
        assert(fd < fileTableHandle.size() && fd >= 0);

        return fileTableHandle.fileTable.entries[fd];
      }
      Entry& operator=(std::shared_ptr<OpenFileDescriptor> ptr) {
        assert(fd >= 0);

        if (fd >= fileTableHandle.size()) {
          fileTableHandle.fileTable.entries.resize(fd + 1);
        }
        fileTableHandle.fileTable.entries[fd] = ptr;

        return *this;
      }
      Entry& operator=(Entry& entry) { return *this = std::shared_ptr<OpenFileDescriptor>(entry); }
      std::shared_ptr<OpenFileDescriptor>& operator->() {
        assert(fd < fileTableHandle.size() && fd >= 0);

        return fileTableHandle.fileTable.entries[fd];
      }
      // Check whether the entry exists (i.e. contains an OpenFileDescriptor).
      operator bool() const {
        if (fd >= fileTableHandle.size() || fd < 0) {
          return false;
        }

        return fileTableHandle.fileTable.entries[fd] != nullptr;
      }
    };

    Entry operator[](__wasi_fd_t fd) { return Entry{*this, fd}; };
  };

  // This get method is responsible for lazily initializing the FileTable.
  // There is only ever one FileTable in the system.
  static Handle get();
};
