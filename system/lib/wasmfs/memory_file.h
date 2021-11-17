// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the memory file class of the new file system.
// This should be the only backend file type defined in a header since it is the
// default type. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.
#include "file.h"
#include <emscripten/threading.h>

#pragma once

namespace wasmfs {
// This class describes a file that lives in Wasm Memory.
class MemoryFile : public DataFile {
  std::vector<uint8_t> buffer;
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    if (offset + len >= buffer.size()) {
      buffer.resize(offset + len);
    }
    memcpy(&buffer[offset], buf, len);

    return __WASI_ERRNO_SUCCESS;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    assert(offset + len - 1 < buffer.size());
    std::memcpy(buf, &buffer[offset], len);

    return __WASI_ERRNO_SUCCESS;
  }

  size_t getSize() override { return buffer.size(); }

public:
  MemoryFile(mode_t mode) : DataFile(mode) {}

  class Handle : public DataFile::Handle {

    std::shared_ptr<MemoryFile> getFile() { return file->cast<MemoryFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : DataFile::Handle(dataFile) {}
    // This function copies preloaded files from JS Memory to Wasm Memory.
    void preloadFromJS(int index) {
      getFile()->buffer.resize(
        EM_ASM_INT({return wasmFS$preloadedFiles[$0].fileData.length}, index));
      // Ensure that files are preloaded from the main thread.
      assert(emscripten_is_main_browser_thread());
      // TODO: Replace every EM_ASM with EM_JS.
      EM_ASM({ HEAPU8.set(wasmFS$preloadedFiles[$1].fileData, $0); },
             getFile()->buffer.data(),
             index);
    }
  };

  Handle locked() { return Handle(shared_from_this()); }
};

} // namespace wasmfs
