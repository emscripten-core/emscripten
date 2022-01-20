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
