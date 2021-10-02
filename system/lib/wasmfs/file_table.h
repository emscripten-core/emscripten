// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "open_file_descriptor.h"
#include <assert.h>
#include <mutex>
#include <utility>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

class FileTable {
  static std::vector<std::shared_ptr<OpenFileDescriptor>> entries;
  std::mutex mutex;

  FileTable() {
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdinFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdoutFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StderrFile>()));
  }

public:
  class Handle {
    FileTable& fileTable;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(FileTable& fileTable) : fileTable(fileTable), lock(fileTable.mutex) {}

    __wasi_fd_t getSize() { return fileTable.entries.size(); }

    __wasi_fd_t addOpenFile(std::shared_ptr<OpenFileDescriptor> ptr);

    bool removeOpenFile(__wasi_fd_t fd);

    struct Entry {
      Handle& entryHandle;
      __wasi_fd_t fd;
      operator std::shared_ptr<OpenFileDescriptor> &() const {
        assert(fd < entryHandle.fileTable.entries.size() && fd >= 0);

        return entryHandle.fileTable.entries[fd];
      }
      Entry& operator=(std::shared_ptr<OpenFileDescriptor> ptr) {
        assert(fd >= 0);

        if (fd >= entryHandle.fileTable.entries.size()) {
          entryHandle.fileTable.entries.resize(fd + 1);
        }
        entryHandle.fileTable.entries[fd] = ptr;

        return *this;
      }
    };

    Entry operator[](__wasi_fd_t fd) { return Entry{*this, fd}; };
  };

  static Handle get();
};

#ifdef __cplusplus
}
#endif
