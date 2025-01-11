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
#include <emscripten/fetch.h>
#include <regex>

namespace wasmfs {

class FetchFile : public ProxiedAsyncJSImplFile {
  std::string filePath;

public:
  FetchFile(const std::string& path,
            mode_t mode,
            backend_t backend,
            emscripten::ProxyWorker& proxy)
    : ProxiedAsyncJSImplFile(mode, backend, proxy),  filePath(path) {}

  const std::string& getPath() const { return filePath; }

  void fetchInit() {
    proxy([&](auto ctx) {
      _wasmfs_jsimpl_async_fetch_init(
        ctx.ctx, getBackendIndex(), getFileIndex());
    });
  }
};


class FetchDirectory : public MemoryDirectory {

  std::string dirPath;
  emscripten::ProxyWorker& proxy;

  const char* fileKindToString(FileKind kind) {
    switch (kind) {
      case DataFileKind: return "file";
      case DirectoryKind: return "directory";
      case SymlinkKind: return "symlink";
      default: return "unknown";
    }
  }

  void createDirectoryStructure() {
    if (fetched) return;
    // Prepare fetch request attributes
    emscripten_fetch_attr_t fetchAttributes;
    emscripten_fetch_attr_init(&fetchAttributes);

    fetchAttributes.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    fetchAttributes.timeoutMSecs = 10;
    strcpy(fetchAttributes.requestMethod, "GET");


    emscripten_fetch_t* fetchData = emscripten_fetch(&fetchAttributes, dirPath.c_str());

    if (!fetchData)
      return;;

    if (fetchData->status == 404) {
        return;
    }

    std::string responseText(fetchData->data, fetchData->numBytes);
    std::regex linkRegex(R"(<a\s+href="([^"]+)\">([^<]+)</a>)");
    std::smatch match;

    std::string::const_iterator searchStart(responseText.cbegin());
    while (std::regex_search(searchStart, responseText.cend(), match, linkRegex)) {
        std::string href = match[1];
        std::string linkName = match[2];
        FileKind kind = href.back() == '/' ? DirectoryKind : DataFileKind;

        if (!linkName.empty() && linkName.back() == '/') {
            linkName.erase(linkName.size() - 1);
        }

        searchStart = match.suffix().first;

        mode_t regularMode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        if (kind == DataFileKind)
          insertDataFile(linkName,  regularMode);
        else
          insertDirectory(linkName, mode | S_IFDIR);
    }
    fetched = true;
    emscripten_fetch_close(fetchData);
  }

  bool fetched = false;




public:
  FetchDirectory(const std::string& path,
                 mode_t mode,
                 backend_t backend,
                 emscripten::ProxyWorker& proxy)
    : MemoryDirectory(mode, backend), dirPath(path), proxy(proxy) {

    createDirectoryStructure();


  }

  FetchDirectory(const std::string& path,
                 mode_t mode,
                 backend_t backend,
                 emscripten::ProxyWorker& proxy, bool willNotfetch)
    : MemoryDirectory(mode, backend), dirPath(path), proxy(proxy) {
  }

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override {
    auto childPath = getChildPath(name);
    auto child =
      std::make_shared<FetchFile>(childPath, mode, getBackend(), proxy);

    insertChild(name, child);
    child->fetchInit();
    return child;
  }


  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    auto childPath = getChildPath(name);
    auto childDir =
      std::make_shared<FetchDirectory>(childPath, mode, getBackend(), proxy, true);
    insertChild(name, childDir);
    return childDir;
  }

  std::string getChildPath(const std::string& name) const {
    return dirPath + '/' + name;
  }


  std::shared_ptr<File> getChild(const std::string& name) override {
    auto child = MemoryDirectory::getChild(name);
    if (!child)
      return nullptr;

    if (child->kind == DirectoryKind){
      auto dir = std::static_pointer_cast<FetchDirectory>(child);
      dir->createDirectoryStructure();
      return dir;
    }

    //This hack calls await in the js fetch implementation
    child->locked().getSize();
    return child;
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
