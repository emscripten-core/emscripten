// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"
#include "special_files.h"

namespace wasmfs {

FileTable::FileTable() {
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_RDONLY, SpecialFiles::getStdin()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, SpecialFiles::getStdout()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, SpecialFiles::getStderr()));
}

std::shared_ptr<OpenFileState> FileTable::Handle::getEntry(__wasi_fd_t fd) {
  if (fd >= fileTable.entries.size() || fd < 0) {
    return nullptr;
  }
  return fileTable.entries[fd];
}

void FileTable::Handle::setEntry(__wasi_fd_t fd,
                                 std::shared_ptr<OpenFileState> openFile) {
  assert(fd >= 0);
  if (fd >= fileTable.entries.size()) {
    fileTable.entries.resize(fd + 1);
  }
  fileTable.entries[fd] = openFile;
}

__wasi_fd_t
FileTable::Handle::addEntry(std::shared_ptr<OpenFileState> openFileState) {
  // TODO: add freelist to avoid linear lookup time.
  for (__wasi_fd_t i = 0;; i++) {
    if (!getEntry(i)) {
      setEntry(i, openFileState);
      return i;
    }
  }
  return -EBADF;
}

} // namespace wasmfs
