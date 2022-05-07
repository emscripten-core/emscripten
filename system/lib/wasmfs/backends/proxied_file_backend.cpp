// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the Proxied File and Proxied Backend of the new file
// system. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "file.h"
#include "memory_backend.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"

namespace wasmfs {

// This class represents a file that forwards all file operations to a thread.
class ProxiedFile : public DataFile {

  emscripten::ProxyWorker& proxy;
  std::shared_ptr<DataFile> baseFile;

  void open(oflags_t flags) override {
    proxy([&]() { baseFile->locked().open(flags); });
  }

  void close() override {
    proxy([&]() { baseFile->locked().close(); });
  }

  // Read and write operations are forwarded via the proxying mechanism.
  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    ssize_t result;
    proxy([&]() { result = baseFile->locked().write(buf, len, offset); });
    return result;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    ssize_t result;
    proxy([&]() { result = baseFile->locked().read(buf, len, offset); });
    return result;
  }

  void flush() override {}

  // Querying the size of the Proxied File returns the size of the underlying
  // file given by the proxying mechanism.
  size_t getSize() override {
    size_t result;
    proxy([&]() { result = baseFile->locked().getSize(); });
    return result;
  }

  void setSize(size_t size) override {
    WASMFS_UNREACHABLE("TODO: ProxiedFS setSize");
  }

public:
  // A file with the chosen destination backend is created on a thread via
  // the ProxiedFile's proxy.
  ProxiedFile(mode_t mode,
              backend_t proxyBackend,
              backend_t underlyingBackend,
              emscripten::ProxyWorker& proxy)
    : DataFile(mode, proxyBackend), proxy(proxy) {
    proxy([&]() { baseFile = underlyingBackend->createFile(mode); });
  }

  // The destructor must use the proxy to forward notification that the Proxied
  // File resource has been destroyed. Proxying is necessary because the
  // underlying thread may need to free resources on the proxied thread.
  // Ex. A JSFile will need to proxy so that it can free its underlying JS array
  // on that thread.
  ~ProxiedFile() {
    proxy([&]() { baseFile = nullptr; });
  }
};

class ProxiedBackend : public Backend {
  backend_t backend;
  // ProxiedBackend uses the proxy member to create files on a thread.
  emscripten::ProxyWorker proxy;

public:
  ProxiedBackend(std::function<backend_t()> createBackend) {
    // Construct the sub-backend on the proper thread (it may set up some local
    // state there in JS, for example).
    proxy([&]() { backend = createBackend(); });
  }

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    // This creates a file on a thread specified by the proxy member.
    return std::make_shared<ProxiedFile>(mode, this, backend, proxy);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    // TODO: This is not generally correct, but happens to work for the JS
    // backend.
    return std::make_shared<MemoryDirectory>(mode, this);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    // TODO.
    abort();
  }
};

// Create a proxied backend by supplying another backend.
extern "C" backend_t
wasmfs_create_proxied_backend(backend_constructor_t create_backend, void* arg) {
  return wasmFS.addBackend(std::make_unique<ProxiedBackend>(
    [create_backend, arg]() { return create_backend(arg); }));
}

} // namespace wasmfs
