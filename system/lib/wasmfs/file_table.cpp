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

// Initialize files specified by --preload-file option.
// Set up directories and files from preloadedDirs and preloadedFiles
// from JS. This function will be called before any file operation to ensure any
// preloaded files are eagerly available for use.
static void preloadFiles() {
  static bool init = []() {
    int numFiles = EM_ASM_INT({return FS.preloadedFiles.length});
    int numDirs = EM_ASM_INT({return FS.preloadedDirs.length});

    // If there are no preloaded files, exit early.
    if (numDirs == 0 && numFiles == 0) {
      return true;
    }

    // Iterate through FS.preloadedDirs to obtain parent and child pair.
    // Ex. Module['FS_createPath']("/foo/parent", "child", true, true);
    for (int i = 0; i < numDirs; i++) {

      char parentPath[PATH_MAX] = {};
      EM_ASM(
        {
          var s = FS.preloadedDirs[$0].parentPath;
          var len = lengthBytesUTF8(s) + 1;
          var numBytesWritten = stringToUTF8(s, $1, len);
        },
        i,
        parentPath);

      auto pathParts = splitPath(parentPath);

      // TODO: Improvement - cache parent path names instead of looking up the
      // Directory every iteration.
      long err;
      auto parentDir = getDir(pathParts.begin(), pathParts.end(), err);

      if (!parentDir) {
        emscripten_console_log(
          "Fatal error during directory creation in file preloading.");
        abort();
      }

      char childName[PATH_MAX] = {};
      EM_ASM(
        {
          var s = FS.preloadedDirs[$0].childName;
          var len = lengthBytesUTF8(s) + 1;
          var numBytesWritten = stringToUTF8(s, $1, len);
        },
        i,
        childName);

      auto created = std::make_shared<Directory>(S_IRUGO | S_IXUGO);

      parentDir->locked().setEntry(childName, created);
    }

    for (int i = 0; i < numFiles; i++) {
      char fileName[PATH_MAX] = {};
      EM_ASM(
        {
          var s = FS.preloadedFiles[$0].pathName;
          var len = lengthBytesUTF8(s) + 1;
          stringToUTF8(s, $1, len);
        },
        i,
        fileName);

      auto mode = (mode_t)EM_ASM_INT({ return FS.preloadedFiles[$0].mode; }, i);

      auto size = EM_ASM_INT({return FS.preloadedFiles[$0].fileData.length}, i);

      auto pathParts = splitPath(fileName);

      auto base = pathParts[pathParts.size() - 1];

      auto created = std::make_shared<MemoryFile>((mode_t)mode);

      long err;
      auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

      if (!parentDir) {
        emscripten_console_log("Fatal error during file preloading");
        abort();
      }

      parentDir->locked().setEntry(base, created);

      created->locked().writeFromJS(i, size);
    }

    return true;
  }();
}

std::vector<std::shared_ptr<OpenFileState>> FileTable::entries;

FileTable::FileTable() {
  preloadFiles();
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_RDONLY, StdinFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StdoutFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StderrFile::getSingleton()));
}

// Initialize default directories including dev/stdin, dev/stdout, dev/stderr.
// Refers to same std streams in the open file table.
std::shared_ptr<Directory> getRootDirectory() {
  static std::atomic<bool> entered(false);

  static const std::shared_ptr<Directory> rootDirectory = [] {
    std::shared_ptr<Directory> rootDirectory =
      std::make_shared<Directory>(S_IRUGO | S_IXUGO);
    auto devDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
    rootDirectory->locked().setEntry("dev", devDirectory);

    auto dir = devDirectory->locked();

    dir.setEntry("stdin", StdinFile::getSingleton());
    dir.setEntry("stdout", StdoutFile::getSingleton());
    dir.setEntry("stderr", StderrFile::getSingleton());

    return rootDirectory;
  }();

  // Exit recursion early as preloadFiles calls getRootDirectory.
  if (entered) {
    return rootDirectory;
  }

  entered = true;

  preloadFiles();

  return rootDirectory;
}

FileTable::Handle FileTable::get() {
  static FileTable fileTable;
  return FileTable::Handle(fileTable);
}

FileTable::Handle::Entry&
FileTable::Handle::Entry::operator=(std::shared_ptr<OpenFileState> ptr) {
  assert(fd >= 0);

  if (fd >= fileTableHandle.fileTable.entries.size()) {
    fileTableHandle.fileTable.entries.resize(fd + 1);
  }
  fileTableHandle.fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<OpenFileState> FileTable::Handle::Entry::unlocked() {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return nullptr;
  }

  return fileTableHandle.fileTable.entries[fd];
}

FileTable::Handle::Entry::operator bool() const {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return false;
  }

  return fileTableHandle.fileTable.entries[fd] != nullptr;
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
