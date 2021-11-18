// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the JS file backend of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "wasmfs.h"

namespace wasmfs {

using js_index_t = uint32_t;

// This class describes a file that lives in JS Memory
class JSFile : public DataFile {
  // This index indicates the location of the JS File in $wasmFS$JSMemoryFiles.
  js_index_t index;

  // JSFiles will write from a Wasm Memory buffer into a JS array defined in
  // $wasmFS$JSMemoryFiles.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    EM_ASM(
      {
        var size = wasmFS$JSMemoryFiles[$0].length;
        if ($2 >= size) {
          // Resize the current arraybuffer.
          var oldContents = wasmFS$JSMemoryFiles[$0];
          wasmFS$JSMemoryFiles[$0] = new Uint8Array($2);
          wasmFS$JSMemoryFiles[$0].set(oldContents.subarray(0, $2));
        }
        wasmFS$JSMemoryFiles[$0].set(HEAPU8.subarray($1, $1 + $2), $3);
      },
      index,
      buf,
      len,
      (size_t)offset);

    return __WASI_ERRNO_SUCCESS;
  }

  // JSFiles will read from JS Memory into a Wasm Memory buffer.
  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    assert(offset + len - 1 < getSize());
    EM_ASM({ HEAPU8.set(wasmFS$JSMemoryFiles[$0].subarray($1, $2), $3); },
           index,
           (size_t)offset,
           (size_t)offset + len,
           buf);

    return __WASI_ERRNO_SUCCESS;
  }

  // The size of the JS File is defined as the length of the JS array in
  // $wasmFS$JSMemoryFiles.
  size_t getSize() override {
    return (size_t)EM_ASM_INT({ return wasmFS$JSMemoryFiles[$0].length; },
                              index);
  }

public:
  JSFile(mode_t mode) : DataFile(mode) {
    // Add the new JS File to the $wasmFS$JSMemoryFiles array and assign the
    // array index.
    index = (js_index_t)EM_ASM_INT({
      wasmFS$JSMemoryFiles.push(new Uint8Array(0));
      return wasmFS$JSMemoryFiles.length - 1;
    });
  }

  class Handle : public DataFile::Handle {

    std::shared_ptr<JSFile> getFile() { return file->cast<JSFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : DataFile::Handle(dataFile) {}
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class JSFileBackend : public Backend {

public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<JSFile>(mode);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<Directory>(mode, this);
  }
};

extern "C" backend_t createJSFileBackend() {
  wasmFS.backendTable.push_back(std::make_unique<JSFileBackend>());
  return wasmFS.backendTable.back().get();
}

} // namespace wasmfs
