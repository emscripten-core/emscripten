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
// JSImpl API (see below for overview).
int _wasmfs_jsimpl_write(backend_t backend,
                         js_index_t index,
                         const uint8_t* buffer,
                         size_t length,
                         off_t offset);
int _wasmfs_jsimpl_read(backend_t backend,
                        js_index_t index,
                        const uint8_t* buffer,
                        size_t length,
                        off_t offset);
int _wasmfs_jsimpl_get_size(backend_t backend, js_index_t index);
int _wasmfs_jsimpl_create(backend_t backend);
void _wasmfs_jsimpl_remove(backend_t backend, js_index_t index);

// Backends.
void _wasmfs_init_jsfile_backend_js();
}


namespace wasmfs {

// A "JSImplFile" is a file that is implemented by JS code. Which particular
// JS code handles it is indicated by a pointer to the backend. The JS code on
// the other side connects that pointer to the actual JS code, basically adding
// a layer of indirection that way. This allows a single C++ class here to have
// multiple JS implementations, which makes it easy to write new JS backends
// without C++ boilerplate.
//
// Each file operation in the _wasmfs_jsimpl_* APIs that we call from here take
// the backend and a pointer to this file itself. Those allow the JS to identify
// both the backend and the particular file. TODO: We could use dense indexes
// instead of pointers, which cause JS to use a map and not an array.
class JSImplFile : public DataFile {
  js_index_t getIndex() {
    static_assert(sizeof(this) == sizeof(js_index_t), "TODO: wasm64");
    return this;
  }

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return _wasmfs_jsimpl_write(getBackend(), getIndex(), buf, len, offset);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    // The caller should have already checked that the offset + len does
    // not exceed the file's size.
    assert(offset + len <= getSize());
    return _wasmfs_jsimpl_read(getBackend(), getIndex(), buf, len, offset);
  }

  void flush() override {}

  size_t getSize() override { return _wasmfs_jsimpl_get_size(getBackend(), getIndex()); }

public:
  JSImplFile(mode_t mode, backend_t backend) : DataFile(mode, backend) {
    _wasmfs_jsimpl_constructor(getBackend(), getIndex());
  }

  ~JSImplFile() { _wasmfs_jsimpl_destructor(getBackend(), getIndex()); }
};

// This backend's files reside in JS Memory.
class JSFileBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<JSFile>(mode, this);
  }
};

// This function is exposed to users to instantiate a new JSBackend.
extern "C" backend_t wasmfs_create_js_file_backend() {
  backend_t backend = wasmFS.addBackend(std::make_unique<JSFileBackend>());
  _wasmfs_backend_add_jsfile(backend);
  return backend;;
}

} // namespace wasmfs
