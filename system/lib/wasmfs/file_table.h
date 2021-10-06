// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include "synchronized.h"
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

public:
  FileTable() {
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdinFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StdoutFile>()));
    entries.push_back(std::make_shared<OpenFileDescriptor>(0, std::make_shared<StderrFile>()));
  }

  // Adds given OpenFileDescriptor to FileTable entries. Returns fd (insertion index in entries).
  __wasi_fd_t addOpenFile(std::shared_ptr<OpenFileDescriptor> ptr);

  // Removes OpenFileDescriptor in FileTable entries corresponding to given fd.
  void removeOpenFile(__wasi_fd_t fd);

  // Entry is used to override the subscript [] operator. This allows the user to get and set values
  // in the FileTable entries vector, even entries outside the bounds of the FileTable.
  struct Entry {
    FileTable& fileTable;
    __wasi_fd_t fd;

    operator std::shared_ptr<OpenFileDescriptor>() const;

    Entry& operator=(std::shared_ptr<OpenFileDescriptor> ptr);

    Entry& operator=(Entry& entry) { return *this = std::shared_ptr<OpenFileDescriptor>(entry); }

    std::shared_ptr<OpenFileDescriptor>& operator->();

    operator bool() const {
      if (fd >= fileTable.entries.size() || fd < 0) {
        return false;
      }

      return fileTable.entries[fd] != nullptr;
    }
  };

  Entry operator[](__wasi_fd_t fd) { return Entry{*this, fd}; };

  // Used to return a locked FileTable that holds a lock during file operations.
  static Locked<FileTable> get();
};
