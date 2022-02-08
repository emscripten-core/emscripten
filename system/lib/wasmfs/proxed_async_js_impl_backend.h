// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "backend.h"
#include "thread_utils.h"
#include "wasmfs.h"

//
// Similar to JSImplBackend, but proxies to another thread where the JS can be
// async.
//

using js_index_t = uint32_t;

extern "C" {
// JSImpl async API.
void _wasmfs_jsimpl_async_alloc_file(js_index_t backend, js_index_t index);
void _wasmfs_jsimpl_async_free_file(js_index_t backend, js_index_t index);
int _wasmfs_jsimpl_async_write(js_index_t backend,
                         js_index_t index,
                         const uint8_t* buffer,
                         size_t length,
                         off_t offset);
int _wasmfs_jsimpl_async_read(js_index_t backend,
                        js_index_t index,
                        const uint8_t* buffer,
                        size_t length,
                        off_t offset);
int _wasmfs_jsimpl_async_get_size(js_index_t backend, js_index_t index);
}

namespace wasmfs {

class ProxiedAsyncJSImplFile : public DataFile {
  emscripten::SyncToAsync& proxy;

  js_index_t getBackendIndex() {
    static_assert(sizeof(backend_t) == sizeof(js_index_t), "TODO: wasm64");
    return js_index_t(getBackend());
  }

  js_index_t getFileIndex() {
    static_assert(sizeof(this) == sizeof(js_index_t), "TODO: wasm64");
    return js_index_t(this);
  }

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return _wasmfs_jsimpl_async_write(
      getBackendIndex(), getFileIndex(), buf, len, offset);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    // The caller should have already checked that the offset + len does
    // not exceed the file's size.
    assert(offset + len <= getSize());
    return _wasmfs_jsimpl_async_read(
      getBackendIndex(), getFileIndex(), buf, len, offset);
  }

  void flush() override {}

  size_t getSize() override {
    return _wasmfs_jsimpl_async_get_size(getBackendIndex(), getFileIndex());
  }

public:
  ProxiedAsyncJSImplFile(mode_t mode, backend_t backend, emscripten::SyncToAsync& proxy) : DataFile(mode, backend), proxy(proxy) {
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      _wasmfs_jsimpl_async_alloc_file(getBackendIndex(), getFileIndex());
      (*resume)();
    });
  }

  ~ProxiedAsyncJSImplFile() {
    _wasmfs_jsimpl_async_free_file(getBackendIndex(), getFileIndex());
  }
};

class ProxiedAsyncJSImplBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<ProxiedAsyncJSImplFile>(mode, this);
  }
};

} // namespace wasmfs
