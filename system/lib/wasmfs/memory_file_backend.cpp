// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the memory file backend of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "memory_file.h"
#include "wasmfs.h"

namespace wasmfs {

__wasi_errno_t MemoryFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len > buffer.size()) {
    buffer.resize(offset + len);
  }
  std::memcpy(&buffer[offset], buf, len);

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
  // The caller should have already checked that the offset + len does
  // not exceed the file's size.
  assert(offset + len <= buffer.size());
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
}

class MemoryFileBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<MemoryFile>(mode, this);
  }
};

backend_t createMemoryFileBackend() {
  return wasmFS.addBackend(std::make_unique<MemoryFileBackend>());
}

} // namespace wasmfs
