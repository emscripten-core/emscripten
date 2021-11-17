// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the JS file backend of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"

namespace wasmfs {

using jsIndex_t = uint32_t;

// This class describes a file that lives in JS Memory
class JSFile : public DataFile {
  // This index indicates the location of the JS File in $wasmFS$JSMemoryFiles.
  jsIndex_t index;

  // JSFiles will write from a Wasm Memory buffer into a JS array defined in
  // $wasmFS$JSMemoryFiles.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    EM_ASM({ $wasmFS$JSMemoryFiles[$0].set(HEAPU8.subarray($1, $2), $3); },
           index,
           (int)buf,
           (int)(buf + len),
           (int)offset);

    return __WASI_ERRNO_SUCCESS;
  }

  // JSFiles will read from JS Memory into a Wasm Memory buffer.
  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    EM_ASM({ HEAPU8.set($wasmFS$JSMemoryFiles[$0].subarray($1, $2), $3); },
           index,
           (int)offset,
           (int)offset + len,
           buf);

    return __WASI_ERRNO_SUCCESS;
  }

  // The size of the JS File is defined as the length of the JS array in
  // $wasmFS$JSMemoryFiles.
  size_t getSize() override {
    return (size_t)EM_ASM_INT({ return $wasmFS$JSMemoryFiles[$0].length; },
                              index);
  }

public:
  JSFile(mode_t mode) : DataFile(mode) {}

  class Handle : public DataFile::Handle {

    std::shared_ptr<JSFile> getFile() { return file->cast<JSFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : DataFile::Handle(dataFile) {}
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class JSFileBackend : public Backend {

public:
  JSFileBackend(backend_t backendID) : Backend(backendID) {}

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<JSFile>(mode);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<Directory>(mode, backendID);
  }
};

std::unique_ptr<Backend> createJSFileBackend(backend_t backendID) {
  return std::make_unique<JSFileBackend>(backendID);
}

} // namespace wasmfs