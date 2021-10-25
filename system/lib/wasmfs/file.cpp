// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"

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
    size = buffer.size();
  }
  memcpy(&buffer[offset], buf, len);

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
  assert(offset + len - 1 < buffer.size());
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
};
} // namespace wasmfs
