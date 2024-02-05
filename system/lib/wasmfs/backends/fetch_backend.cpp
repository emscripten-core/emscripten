// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "fetch_backend.h"
#include "backend.h"
#include "proxied_async_js_impl_backend.h"
#include "wasmfs.h"

namespace wasmfs {

class FetchFile : public ProxiedAsyncJSImplFile {
  std::string filePath;

public:
  FetchFile(const std::string& path,
            mode_t mode,
            backend_t backend,
            emscripten::ProxyWorker& proxy)
    : ProxiedAsyncJSImplFile(mode, backend, proxy), filePath(path) {}

  const std::string& getPath() const { return filePath; }
};

class FetchDirectory : public MemoryDirectory {
  std::string dirPath;
  emscripten::ProxyWorker& proxy;

public:
  FetchDirectory(const std::string& path,
                 mode_t mode,
                 backend_t backend,
                 emscripten::ProxyWorker& proxy)
    : MemoryDirectory(mode, backend), dirPath(path), proxy(proxy) {}

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override {
    auto childPath = getChildPath(name);
    auto child =
      std::make_shared<FetchFile>(childPath, mode, getBackend(), proxy);
    insertChild(name, child);
    return child;
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    auto childPath = getChildPath(name);
    auto childDir =
      std::make_shared<FetchDirectory>(childPath, mode, getBackend(), proxy);
    insertChild(name, childDir);
    return childDir;
  }

  std::string getChildPath(const std::string& name) const {
    return dirPath + '/' + name;
  }
};

class FetchBackend : public ProxiedAsyncJSBackend {
  std::string baseUrl;

public:
  FetchBackend(const std::string& baseUrl,
               std::function<void(backend_t)> setupOnThread)
    : ProxiedAsyncJSBackend(setupOnThread), baseUrl(baseUrl) {}

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<FetchFile>(baseUrl, mode, this, proxy);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<FetchDirectory>(baseUrl, mode, this, proxy);
  }
};

extern "C" {
backend_t wasmfs_create_fetch_backend(const char* base_url) {
  // ProxyWorker cannot safely be synchronously spawned from the main browser
  // thread. See comment in thread_utils.h for more details.
  assert(!emscripten_is_main_browser_thread() &&
         "Cannot safely create fetch backend on main browser thread");
  return wasmFS.addBackend(std::make_unique<FetchBackend>(
    base_url ? base_url : "",
    [](backend_t backend) { _wasmfs_create_fetch_backend_js(backend); }));
}

const char* EMSCRIPTEN_KEEPALIVE _wasmfs_fetch_get_file_path(void* ptr) {
  auto* file = reinterpret_cast<wasmfs::FetchFile*>(ptr);
  return file ? file->getPath().data() : nullptr;
}
}

} // namespace wasmfs
