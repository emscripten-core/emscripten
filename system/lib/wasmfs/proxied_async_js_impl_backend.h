// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "backend.h"
#include "memory_backend.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"

//
// Similar to JSImplBackend, but proxies to another thread where the JS can be
// async.
//
// To write a new async backend in JS, you basically do the following:
//
//  1. Add a declaration of the C function to create the backend in the
//     "backend creation" section of emscripten/wasmfs.h. (One line.)
//  2. Add a cpp file for the new backend, and implement the C function from 1,
//     which should create a ProxiedAsyncJSBackend while passing it code to
//     set up the JS side. (A few lines.)
//  3. Write a new JS library, and add the implementation of the JS method just
//     mentioned, which should set up the mapping from the C++ backend object's
//     address to the JS code containing the hooks to read and write etc. The
//     hooks should each return a JS Promise. (99% of the work happens here.)
//
// This is basically the same as JSImplBackend, except that the code in step #3
// is async (also, step #2 looks slightly different, but is similarly very
// short).
//
// For a simple example, see fetch_backend.cpp and library_wasmfs_fetch.js.
//

using js_index_t = uint32_t;

extern "C" {

// JSImpl async API.
void _wasmfs_jsimpl_async_alloc_file(em_proxying_ctx* ctx,
                                     js_index_t backend,
                                     js_index_t index);
void _wasmfs_jsimpl_async_free_file(em_proxying_ctx* ctx,
                                    js_index_t backend,
                                    js_index_t index);
void _wasmfs_jsimpl_async_write(em_proxying_ctx* ctx,
                                js_index_t backend,
                                js_index_t index,
                                const uint8_t* buffer,
                                size_t length,
                                off_t offset,
                                ssize_t* result);
void _wasmfs_jsimpl_async_read(em_proxying_ctx* ctx,
                               js_index_t backend,
                               js_index_t index,
                               const uint8_t* buffer,
                               size_t length,
                               off_t offset,
                               ssize_t* result);
void _wasmfs_jsimpl_async_get_size(em_proxying_ctx* ctx,
                                   js_index_t backend,
                                   js_index_t index,
                                   size_t* result);
}

namespace wasmfs {

class ProxiedAsyncJSImplFile : public DataFile {
  emscripten::ProxyWorker& proxy;

  js_index_t getBackendIndex() {
    static_assert(sizeof(backend_t) == sizeof(js_index_t), "TODO: wasm64");
    return js_index_t(getBackend());
  }

  js_index_t getFileIndex() {
    static_assert(sizeof(this) == sizeof(js_index_t), "TODO: wasm64");
    return js_index_t(this);
  }

  // TODO: Notify the JS about open and close events?
  void open(oflags_t) override {}
  void close() override {}

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    ssize_t result;
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_write(
        ctx.ctx, getBackendIndex(), getFileIndex(), buf, len, offset, &result);
    });
    return result;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    ssize_t result;
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_read(
        ctx.ctx, getBackendIndex(), getFileIndex(), buf, len, offset, &result);
    });
    return result;
  }

  void flush() override {}

  size_t getSize() override {
    size_t result;
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_get_size(
        ctx.ctx, getBackendIndex(), getFileIndex(), &result);
    });
    return result;
  }

  void setSize(size_t size) override {
    WASMFS_UNREACHABLE("TODO: ProxiedAsyncJSImplFile setSize");
  }

public:
  ProxiedAsyncJSImplFile(mode_t mode,
                         backend_t backend,
                         emscripten::ProxyWorker& proxy)
    : DataFile(mode, backend), proxy(proxy) {
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_alloc_file(
        ctx.ctx, getBackendIndex(), getFileIndex());
    });
  }

  ~ProxiedAsyncJSImplFile() {
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_free_file(
        ctx.ctx, getBackendIndex(), getFileIndex());
    });
  }
};

class ProxiedAsyncJSBackend : public Backend {
  emscripten::ProxyWorker proxy;

public:
  // Receives as a parameter a function to call on the proxied thread, which is
  // useful for doing setup there.
  ProxiedAsyncJSBackend(std::function<void(backend_t)> setupOnThread) {
    proxy([&](auto ctx) {
      setupOnThread(this);
      ctx.finish();
    });
  }

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<ProxiedAsyncJSImplFile>(mode, this, proxy);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<MemoryDirectory>(mode, this);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return std::make_shared<MemorySymlink>(target, this);
  }
};

} // namespace wasmfs
