// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"

namespace wasmfs {

std::vector<std::shared_ptr<FileDescriptor>> FileTable::entries;

std::shared_ptr<File>& FileDescriptor::getFile() { return file; }

__wasi_fd_t FileTable::add(std::shared_ptr<FileDescriptor> ptr) {
  for (__wasi_fd_t i = 0; i < entries.size(); i++) {
    if (!entries[i]) {
      // Free open file entry.
      fileTable.entries[i] = ptr;
      return i;
    }
  }

  // Could not find an empty open file table entry.
  fileTable.entries.push_back(ptr);

  return fileTable.entries.size() - 1;
}

void FileTable::remove(__wasi_fd_t fd) {
  // Check if the file descriptor is invalid.
  assert(fd < entries.size() && fd >= 0);

  entries[fd] = nullptr;
}

Locked<FileTable> FileTable::get() {
  static Lockable<FileTable> fileTable;
  return fileTable.get();
};

// Operator Overloading for FileTable::Entry
FileTable::Entry::operator std::shared_ptr<FileDescriptor>() const {
  if (fd >= fileTable.entries.size() || fd < 0) {
    return nullptr;
  }
  return fileTable.entries[fd];
}

FileTable::Entry& FileTable::Entry::operator=(std::shared_ptr<FileDescriptor> ptr) {
  assert(fd >= 0);

  if (fd >= fileTable.entries.size()) {
    fileTable.entries.resize(fd + 1);
  }
  fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<FileDescriptor>& FileTable::Entry::operator->() {
  assert(fd < fileTable.entries.size() && fd >= 0);

  fileTable.entries[fd] = nullptr;
}
} // namespace wasmfs
