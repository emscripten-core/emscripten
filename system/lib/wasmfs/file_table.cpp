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
  entries.emplace_back();
  (void)OpenFileState::create(
    SpecialFiles::getStdin(), O_RDONLY, entries.back());
  entries.emplace_back();
  (void)OpenFileState::create(
    SpecialFiles::getStdout(), O_WRONLY, entries.back());
  entries.emplace_back();
  (void)OpenFileState::create(
    SpecialFiles::getStderr(), O_WRONLY, entries.back());
}

std::shared_ptr<OpenFileState> FileTable::Handle::getEntry(__wasi_fd_t fd) {
  if (fd >= fileTable.entries.size() || fd < 0) {
    return nullptr;
  }
  return fileTable.entries[fd];
}

int FileTable::Handle::setEntry(__wasi_fd_t fd,
                                std::shared_ptr<OpenFileState> openFile) {
  assert(fd >= 0);
  if (fd >= fileTable.entries.size()) {
    fileTable.entries.resize(fd + 1);
  }
  int ret = 0;
  if (fileTable.entries[fd]) {
    auto file = fileTable.entries[fd]->locked().getFile();
    if (auto f = file->dynCast<DataFile>()) {
      ret = f->locked().close();
      assert(ret <= 0);
    }
  }
  fileTable.entries[fd] = openFile;
  return ret;
}

__wasi_fd_t
FileTable::Handle::addEntry(std::shared_ptr<OpenFileState> openFileState) {
  // TODO: add freelist to avoid linear lookup time.
  for (__wasi_fd_t i = 0;; i++) {
    if (!getEntry(i)) {
      (void)setEntry(i, openFileState);
      return i;
    }
  }
  return -EBADF;
}

} // namespace wasmfs
