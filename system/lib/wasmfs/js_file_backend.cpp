// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "wasmfs.h"

using js_index_t = uint32_t;

extern "C" {
int _wasmfs_write_js_file(js_index_t index,
                          const uint8_t* buffer,
                          size_t length,
                          off_t offset);
int _wasmfs_read_js_file(js_index_t index,
                         const uint8_t* buffer,
                         size_t length,
                         off_t offset);
int _wasmfs_get_js_file_size(js_index_t index);
int _wasmfs_create_js_file();
void _wasmfs_remove_js_file(js_index_t index);
}

namespace wasmfs {

// This class describes a file that lives in JS Memory
class JSFile : public DataFile {
  // This index indicates the location of the JS File in the backing JS array.
  js_index_t index;

  // JSFiles will write from a Wasm Memory buffer into the backing JS array.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return _wasmfs_write_js_file(index, buf, len, offset);
  }

  // JSFiles will read from the backing JS array into a Wasm Memory buffer.
  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    // The caller should have already checked that the offset + len does
    // not exceed the file's size.
    assert(offset + len <= getSize());
    return _wasmfs_read_js_file(index, buf, len, offset);
  }

  // The size of the JSFile is defined as the length of the backing JS array.
  size_t getSize() override { return _wasmfs_get_js_file_size(index); }

public:
  JSFile(mode_t mode, backend_t backend) : DataFile(mode, backend) {
    // Create a new file in the backing JS array and store its index.
    index = _wasmfs_create_js_file();
  }

  // Remove the typed array file contents in the backing JS array.
  ~JSFile() { _wasmfs_remove_js_file(index); }
};

class JSFileBackend : public Backend {

public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<JSFile>(mode, this);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<Directory>(mode, this);
  }
};

// This function is exposed to users to instantiate a new JSBackend.
extern "C" backend_t wasmfs_create_js_file_backend() {
  return wasmFS.addBackend(std::make_unique<JSFileBackend>());
}

} // namespace wasmfs
