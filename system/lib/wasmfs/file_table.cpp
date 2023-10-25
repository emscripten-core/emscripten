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

std::shared_ptr<DataFile>
FileTable::Handle::setEntry(__wasi_fd_t fd,
                            std::shared_ptr<OpenFileState> openFile) {
  assert(fd >= 0);
  if (fd >= fileTable.entries.size()) {
    fileTable.entries.resize(fd + 1);
  }
  if (openFile) {
    ++openFile->uses;
  }
  std::shared_ptr<DataFile> ret;
  if (fileTable.entries[fd] && --fileTable.entries[fd]->uses == 0) {
    ret = fileTable.entries[fd]->locked().getFile()->dynCast<DataFile>();
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

int OpenFileState::create(std::shared_ptr<File> file,
                          oflags_t flags,
                          std::shared_ptr<OpenFileState>& out) {
  assert(file);
  std::vector<Directory::Entry> dirents;
  if (auto f = file->dynCast<DataFile>()) {
    if (int err = f->locked().open(flags & O_ACCMODE)) {
      return err;
    }
  } else if (auto d = file->dynCast<Directory>()) {
    // We are opening a directory; cache its contents for subsequent reads.
    auto lockedDir = d->locked();
    dirents = {{".", File::DirectoryKind, d->getIno()},
               {"..", File::DirectoryKind, lockedDir.getParent()->getIno()}};
    auto entries = lockedDir.getEntries();
    if (int err = entries.getError()) {
      return err;
    }
    dirents.insert(dirents.end(), entries->begin(), entries->end());
  }

  out = std::make_shared<OpenFileState>(
    private_key{0}, flags, file, std::move(dirents));
  return 0;
}

} // namespace wasmfs
