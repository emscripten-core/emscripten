// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"

std::vector<std::shared_ptr<OpenFileDescriptor>> FileTable::entries;

std::shared_ptr<File>& OpenFileDescriptor::getFile() { return file; }

FileTable::Handle FileTable::get() {
  static FileTable fileTable;
  return FileTable::Handle(fileTable);
}

__wasi_fd_t FileTable::Handle::addOpenFile(std::shared_ptr<OpenFileDescriptor> ptr) {
  for (__wasi_fd_t i = 0; i < fileTable.entries.size(); i++) {
    if (!fileTable.entries[i]) {
      // Free open file entry.
      fileTable.entries[i] = ptr;
      return i;
    }
  }

  // Could not find an empty open file table entry.
  fileTable.entries.push_back(ptr);

  return fileTable.entries.size() - 1;
}

void FileTable::Handle::removeOpenFile(__wasi_fd_t fd) {
  // Check if the file descriptor is invalid.
  assert(fd < fileTable.entries.size() && fd >= 0);

  fileTable.entries[fd] = nullptr;
}
