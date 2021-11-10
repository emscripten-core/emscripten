// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"
#include "wasmfs.h"
#include <emscripten/threading.h>

namespace wasmfs {
//
// Directory
//
std::shared_ptr<File> Directory::Handle::getEntry(std::string pathName) {
  auto it = getDir()->entries.find(pathName);
  if (it == getDir()->entries.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}
//
// MemoryFile
//
__wasi_errno_t MemoryFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len >= buffer.size()) {
    buffer.resize(offset + len);
  }
  memcpy(&buffer[offset], buf, len);

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
  assert(offset + len - 1 < buffer.size());
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
}

void MemoryFile::Handle::preloadFromJS(int index) {
  getFile()->buffer.resize(
    EM_ASM_INT({return wasmFS$preloadedFiles[$0].fileData.length}, index));
  // Ensure that files are preloaded from the main thread.
  assert(emscripten_is_main_browser_thread());
  // TODO: Replace every EM_ASM with EM_JS.
  EM_ASM({ HEAPU8.set(wasmFS$preloadedFiles[$1].fileData, $0); },
         getFile()->buffer.data(),
         index);
}
//
// Path Parsing utilities
//
std::shared_ptr<Directory> getDir(std::vector<std::string>::iterator begin,
                                  std::vector<std::string>::iterator end,
                                  long& err) {

  std::shared_ptr<File> curr;
  // Check if the first path element is '/', indicating an absolute path.
  if (*begin == "/") {
    curr = wasmFS.getRootDirectory();
    begin++;
  } else {
    curr = wasmFS.getCWD();
  }

  for (auto it = begin; it != end; ++it) {
    auto directory = curr->dynCast<Directory>();

    // If file is nullptr, then the file was not a Directory.
    // TODO: Change this to accommodate symlinks
    if (!directory) {
      err = -ENOTDIR;
      return nullptr;
    }

    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    directory->locked().printKeys();
#endif
    curr = directory->locked().getEntry(*it);

    // Requested entry (file or directory)
    if (!curr) {
      err = -ENOENT;
      return nullptr;
    }

#ifdef WASMFS_DEBUG
    emscripten_console_log(it->c_str());
#endif
  }

  auto currDirectory = curr->dynCast<Directory>();

  if (!currDirectory) {
    err = -ENOTDIR;
    return nullptr;
  }

  return currDirectory;
}

// TODO: Check for trailing slash, i.e. /foo/bar.txt/
// Currently any trailing slash is ignored.
std::vector<std::string> splitPath(char* pathname) {
  std::vector<std::string> pathParts;
  char newPathName[strlen(pathname) + 1];
  strcpy(newPathName, pathname);

  // TODO: Other path parsing edge cases.
  char* current;
  // Handle absolute path.
  if (newPathName[0] == '/') {
    pathParts.push_back("/");
  }

  current = strtok(newPathName, "/");
  while (current != NULL) {
    pathParts.push_back(current);
    current = strtok(NULL, "/");
  }

  return pathParts;
}
} // namespace wasmfs
