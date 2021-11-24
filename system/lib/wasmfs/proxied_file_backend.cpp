// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the Proxied File and Proxied Backend of the new file
// system. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "file.h"
#include "thread_utils.h"
#include "wasmfs.h"

namespace wasmfs {

// This class represents a file that is resident on a background pthread. All
// file operations are proxied to this dedicated thread.
class ProxiedFile : public DataFile {

  emscripten::SyncToAsync& proxy;
  std::shared_ptr<DataFile> backendFile;

  // Read and write operations are forwarded to the file residing on the
  // dedicated thread.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    __wasi_errno_t result;
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      result = backendFile->locked().write(buf, len, offset);
      (*resume)();
    });
    return result;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    __wasi_errno_t result;
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      result = backendFile->locked().read(buf, len, offset);
      (*resume)();
    });
    return result;
  }

  // Querying the size of the Proxied File returns the size of the underlying
  // file on the dedicated thread.
  size_t getSize() override {
    size_t result;
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      result = backendFile->locked().getSize();
      (*resume)();
    });
    return result;
  }

public:
  // A file with the chosen destination backend is created on the Proxied File's
  // dedicated thread.
  ProxiedFile(mode_t mode,
              backend_t srcBackend,
              backend_t destBackend,
              emscripten::SyncToAsync& proxy)
    : DataFile(mode, srcBackend), proxy(proxy) {
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      backendFile = destBackend->createFile(mode);
      (*resume)();
    });
  }

  // The destructor must proxy to the pthread and release the file stored on
  // that pthread.
  ~ProxiedFile() {
    proxy.invoke([&](emscripten::SyncToAsync::Callback resume) {
      backendFile = nullptr;
      (*resume)();
    });
  }
};
class ProxiedBackend : public Backend {
  backend_t backend;
  // The proxy member manages a dedicated thread that the Proxied Backend uses
  // to create files and directories under.
  emscripten::SyncToAsync proxy;

public:
  ProxiedBackend(backend_t backend) : backend(backend) {}

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<ProxiedFile>(mode, this, backend, proxy);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<Directory>(mode, this);
  }
};

// Create a proxied backend by supplying another backend.
extern "C" backend_t wasmfs_create_proxied_file_backend(backend_t backend) {
  return wasmFS.addBackend(std::make_unique<ProxiedBackend>(backend));
}

} // namespace wasmfs
