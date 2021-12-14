// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"
#include "streams.h"

namespace wasmfs {

FileTable::FileTable() {
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_RDONLY, StdinFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StdoutFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StderrFile::getSingleton()));
}

FileTable::Entry&
FileTable::Entry::operator=(std::shared_ptr<OpenFileState> ptr) {
  assert(fd >= 0);

  if (fd >= fileTable.entries.size()) {
    fileTable.entries.resize(fd + 1);
  }
  fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<OpenFileState> FileTable::Entry::unlocked() {
  if (fd >= fileTable.entries.size() || fd < 0) {
    return nullptr;
  }

  return fileTable.entries[fd];
}

FileTable::Entry::operator bool() const {
  if (fd >= fileTable.entries.size() || fd < 0) {
    return false;
  }

  return fileTable.entries[fd] != nullptr;
}

FileTable::Entry FileTable::Handle::operator[](__wasi_fd_t fd) {
  return Entry(fileTable, fd);
}

__wasi_fd_t
FileTable::Handle::add(std::shared_ptr<OpenFileState> openFileState) {
  Handle& self = *this;
  // TODO: add freelist to avoid linear lookup time.
  for (__wasi_fd_t i = 0;; i++) {
    if (!self[i]) {
      // Free open file entry.
      self[i] = openFileState;
      return i;
    }
  }
  return -EBADF;
}
} // namespace wasmfs
