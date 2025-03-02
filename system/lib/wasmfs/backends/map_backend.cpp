// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the fetch backend.

#include "backend.h"
#include "wasmfs.h"
#include "virtual.h"
#include "file.h"
#include "paths.h"
#include "memory_backend.h"
#include <sstream>

namespace wasmfs {

typedef std::map<std::string,std::string> MapManifest;

class MapBackend : public wasmfs::Backend {
  MapManifest *manifest;
 public:
  // Takes ownership of manifest
  MapBackend(MapManifest *manifest): manifest(manifest) {
    assert(manifest && "Mapfs: null manifest not supported");
  }
  ~MapBackend() {
    if(manifest != NULL) {
      delete manifest;
    }
  }
  std::shared_ptr<DataFile> createFile(mode_t mode) override;
  std::shared_ptr<Directory> createDirectory(mode_t mode) override;
  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    fprintf(stderr, "mapfs doesn't support creating symlinks");
    abort();
    return NULL;
  }
  const std::string getTargetPath(const std::string& filePath);
  const MapManifest *getManifest() {
    return manifest;
  }
};


class MapDirectory : public MemoryDirectory {
  std::string dirPath;

public:
  MapDirectory(const std::string& path,
                 mode_t mode,
                 backend_t backend)
    : MemoryDirectory(mode, backend), dirPath(path) {
    auto manifest = dynamic_cast<MapBackend*>(getBackend())->getManifest();
    if (manifest && path == "") {
      for (const auto& pair : *manifest) {
        auto path = pair.first;
        assert(path[0] == '/');
        char delimiter = '/';
        std::string pathSoFar = "";
        std::string tmp = "";
        std::shared_ptr<MapDirectory> dir = NULL;
        std::istringstream iss(path);
        while(std::getline(iss, tmp, delimiter)) {
          pathSoFar += tmp;
          if (pathSoFar == path) {
            if(!dir) {
              assert(this->insertDataFile(tmp, 0777));
            } else {
              assert(dir->insertDataFile(tmp, 0777));
            }
          } else if (pathSoFar != "") {
            std::shared_ptr<MapDirectory> next = NULL;
            if(!dir) {
              next = std::dynamic_pointer_cast<MapDirectory>(this->getChild(tmp));
            } else {
              next = std::dynamic_pointer_cast<MapDirectory>(dir->getChild(tmp));
            }
            if (next) {
              dir = next;
              assert(dir);
            } else {
              if(!dir) {
                dir = std::dynamic_pointer_cast<MapDirectory>(this->insertDirectory(tmp, 0777));
              } else {
                dir = std::dynamic_pointer_cast<MapDirectory>(dir->insertDirectory(tmp, 0777));
              }
              assert(dir);
            }
          }
          pathSoFar += delimiter;
        }
      }
    }
  }

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                                  mode_t mode) override {
    auto backend = dynamic_cast<MapBackend*>(getBackend());
    auto childPath = getChildPath(name);
    auto targetPath = backend->getTargetPath(childPath);
    auto parsed = path::parseFile(targetPath);
    if (auto err = parsed.getError()) {
      fprintf(stderr, "error %ld\n", err);
      abort();
    }
    auto file = parsed.getFile();
    if (!file) {
      fprintf(stderr, "no datafile for %s\n", targetPath.c_str());
      return nullptr;
    }
    auto virtualFile = std::make_shared<VirtualDataFile>(file->cast<DataFile>(), file->getBackend());
    insertChild(name, virtualFile);
    return virtualFile->cast<DataFile>();
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    auto childPath = getChildPath(name);
    auto childDir =
      std::make_shared<MapDirectory>(childPath, mode, getBackend());
    insertChild(name, childDir);
    return childDir;
  }

  std::string getChildPath(const std::string& name) const {
    return dirPath + '/' + name;
  }

  std::shared_ptr<File> getChild(const std::string& name) override {
    return MemoryDirectory::getChild(name);
  }
};

std::shared_ptr<DataFile> MapBackend::createFile(mode_t mode) {
  assert(false && "Can't create freestanding file with mapfs");
  return NULL;
}

std::shared_ptr<Directory> MapBackend::createDirectory(mode_t mode) {
  return std::make_shared<MapDirectory>("", mode, this);
}

const std::string MapBackend::getTargetPath(const std::string& filePath) {
  if (auto search = manifest->find(filePath); search != manifest->end()) {
    return search->second;
  }
  fprintf(stderr, "File %s not found in manifest", filePath.c_str());
  abort();
}

extern "C" {

backend_t wasmfs_create_map_backend(MapManifest *manifest) {
  return wasmFS.addBackend(std::make_unique<MapBackend>(manifest));
}

void *EMSCRIPTEN_KEEPALIVE wasmfs_map_create_manifest() {
  return new MapManifest();
}

void EMSCRIPTEN_KEEPALIVE wasmfs_map_add_to_manifest(MapManifest *manifest, const char *virtualPath, const char *targetPath) {
  assert(manifest && "wasmfs_map_add_to_manifest: null manifest");
  auto virtualStr = std::string(virtualPath);
  auto targetStr = std::string(targetPath);
  manifest->insert(std::pair(virtualStr, targetStr));
}

}

} // namespace wasmfs
