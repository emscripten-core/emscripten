// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "async_callback.h"
#include "backend.h"
#include "thread_utils.h"
#include "wasmfs.h"

//
// Similar to JSImplBackend, but proxies to another thread where the JS can be
// async.
//

using js_index_t = uint32_t;

// A callback state that also adds the C++ component for resuming. This is only
// needed on the C++ side, but needs to be passed around as part of this
// struct when we go through C and JS.
struct CppCallbackState : public CallbackState {
  // The function to call to resume execution.
  emscripten::SyncToAsync::Callback resume;
};

extern "C" {
// JSImpl async API.

// An async callback with no extra parameters.
typedef void (*async_callback_t)(CppCallbackState* state);

void _wasmfs_jsimpl_async_alloc_file(js_index_t backend,
                                     js_index_t index,
                                     async_callback_t callback,
                                     CppCallbackState* state);
void _wasmfs_jsimpl_async_free_file(js_index_t backend,
                                    js_index_t index,
                                    async_callback_t callback,
                                    CppCallbackState* state);
void _wasmfs_jsimpl_async_write(js_index_t backend,
                                js_index_t index,
                                const uint8_t* buffer,
                                size_t length,
                                off_t offset,
                                async_callback_t callback,
                                CppCallbackState* state);
void _wasmfs_jsimpl_async_read(js_index_t backend,
                               js_index_t index,
                               const uint8_t* buffer,
                               size_t length,
                               off_t offset,
                               async_callback_t callback,
                               CppCallbackState* state);
void _wasmfs_jsimpl_async_get_size(js_index_t backend,
                                   js_index_t index,
                                   async_callback_t callback,
                                   CppCallbackState* state);
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
    CppCallbackState state;

    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      state.resume = resume;

      _wasmfs_jsimpl_async_write(
        getBackendIndex(),
        getFileIndex(),
        buf,
        len,
        offset,
        [](CppCallbackState* state) { (*state->resume)(); },
        &state);
    });

    return state.result;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    // The caller should have already checked that the offset + len does
    // not exceed the file's size.
    assert(offset + len <= getSize());

    CppCallbackState state;

    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      state.resume = resume;

      _wasmfs_jsimpl_async_read(
        getBackendIndex(),
        getFileIndex(),
        buf,
        len,
        offset,
        [](CppCallbackState* state) { (*state->resume)(); },
        &state);
    });

    return state.result;
  }

  void flush() override {}

  size_t getSize() override {
    CppCallbackState state;

    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      state.resume = resume;

      _wasmfs_jsimpl_async_get_size(
        getBackendIndex(),
        getFileIndex(),
        [](CppCallbackState* state) { (*state->resume)(); },
        &state);
    });

    return state.offset;
  }

public:
  ProxiedAsyncJSImplFile(mode_t mode,
                         backend_t backend,
                         emscripten::SyncToAsync& proxy)
    : DataFile(mode, backend), proxy(proxy) {
    CppCallbackState state;

    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      state.resume = resume;

      _wasmfs_jsimpl_async_alloc_file(
        getBackendIndex(),
        getFileIndex(),
        [](CppCallbackState* state) { (*state->resume)(); },
        &state);
    });
  }

  ~ProxiedAsyncJSImplFile() {
    CppCallbackState state;

    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      state.resume = resume;

      _wasmfs_jsimpl_async_free_file(
        getBackendIndex(),
        getFileIndex(),
        [](CppCallbackState* state) { (*state->resume)(); },
        &state);
    });
  }
};

class ProxiedAsyncJSBackend : public Backend {
  emscripten::SyncToAsync proxy;

public:
  // Receives as a parameter a function to call on the proxied thread, which is
  // useful for doing setup there.
  ProxiedAsyncJSBackend(std::function<void(backend_t)> setupOnThread) {
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      setupOnThread(this);
      (*resume)();
    });
  }

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<ProxiedAsyncJSImplFile>(mode, this, proxy);
  }
};

} // namespace wasmfs
