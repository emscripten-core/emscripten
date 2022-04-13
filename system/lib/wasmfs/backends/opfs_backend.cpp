// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdlib.h>
#include "backend.h"
#include "file.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"

namespace wasmfs {

using ProxyWorker = emscripten::ProxyWorker;

extern "C" {

// Ensure that the root OPFS directory is initialized with ID 0.
void _wasmfs_opfs_init_root_directory(em_proxying_ctx* ctx);

// Create a file under `parent` with `name` and store its ID in `child_id`.
void _wasmfs_opfs_insert_file(em_proxying_ctx* ctx,
                              int parent,
                              const char* name,
                              int* child_id);

// Create a directory under `parent` with `name` and store its ID in `child_id`.
void _wasmfs_opfs_insert_directory(em_proxying_ctx* ctx,
                                   int parent,
                                   const char* name,
                                   int* child_id);

// Look up the child under `parent` with `name`. Write 1 to `child_type` if it's
// a regular file or 2 if it's a directory. Write the child's file or directory
// ID to `child_id`, or -1 if the child does not exist, or -2 if the child
// exists but cannot be opened.
void _wasmfs_opfs_get_child(em_proxying_ctx* ctx,
                            int parent,
                            const char* name,
                            int* child_type,
                            int* child_id);
}

class OPFSFile : public DataFile {
public:
  ProxyWorker& proxy;

  // The ID of the corresponding access handle in the JS library.
  int accessID = 0;

  OPFSFile(mode_t mode, backend_t backend, int accessID, ProxyWorker& proxy)
    : DataFile(mode, backend), accessID(accessID), proxy(proxy) {}

private:
  size_t getSize() override { WASMFS_UNREACHABLE("TODO"); }

  void setSize(size_t size) override { WASMFS_UNREACHABLE("TODO"); }

  void open(oflags_t flags) override { WASMFS_UNREACHABLE("TODO"); }

  void close() override { WASMFS_UNREACHABLE("TODO"); }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    WASMFS_UNREACHABLE("TODO");
  }

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    WASMFS_UNREACHABLE("TODO");
  }

  void flush() override { WASMFS_UNREACHABLE("TODO"); }
};

class OPFSDirectory : public Directory {
public:
  ProxyWorker& proxy;

  // The ID of this directory in the JS library.
  int dirID = 0;

  OPFSDirectory(mode_t mode, backend_t backend, int dirID, ProxyWorker& proxy)
    : Directory(mode, backend), dirID(dirID), proxy(proxy) {}

private:
  std::shared_ptr<File> getChild(const std::string& name) override {
    int childType = 0, childID = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_get_child(
        ctx.ctx, dirID, name.c_str(), &childType, &childID);
    });
    if (childID < 0) {
      // TODO: More fine-grained error reporting.
      return NULL;
    }
    if (childType == 1) {
      return std::make_shared<OPFSFile>(0777, getBackend(), childID, proxy);
    } else if (childType == 2) {
      return std::make_shared<OPFSDirectory>(0777, getBackend(), childID, proxy);
    } else {
      WASMFS_UNREACHABLE("Unexpected child type");
    }
  }

  bool removeChild(const std::string& name) override {
    WASMFS_UNREACHABLE("TODO");
  }

  std::shared_ptr<File> insertChild(const std::string& name,
                                    std::shared_ptr<File> file) override {
    int childID = 0;
    if (file->is<DataFile>()) {
      proxy([&](auto ctx) {
        _wasmfs_opfs_insert_file(ctx.ctx, dirID, name.c_str(), &childID);
      });
      assert(childID > 0);
      std::static_pointer_cast<OPFSFile>(file)->accessID = childID;
      return file;
    } else if (file->is<Directory>()) {
      proxy([&](auto ctx) {
        _wasmfs_opfs_insert_directory(ctx.ctx, dirID, name.c_str(), &childID);
      });
      assert(childID > 0);
      std::static_pointer_cast<OPFSDirectory>(file)->dirID = childID;
      return file;
    } else {
      WASMFS_UNREACHABLE("TODO");
    }
  }

  std::string getName(std::shared_ptr<File> file) override {
    WASMFS_UNREACHABLE("TODO");
  }

  size_t getNumEntries() override { WASMFS_UNREACHABLE("TODO"); }

  std::vector<Directory::Entry> getEntries() override {
    WASMFS_UNREACHABLE("TODO");
  }
};

class OPFSBackend : public Backend {
public:
  ProxyWorker proxy;

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    // No way to support a raw file without a parent directory.
    abort();
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    proxy([](auto ctx) { _wasmfs_opfs_init_root_directory(ctx.ctx); });
    return std::make_shared<OPFSDirectory>(mode, this, 0, proxy);
  }
  // TODO: symlink?
};

extern "C" {

backend_t wasmfs_create_opfs_backend() {
  return wasmFS.addBackend(std::make_unique<OPFSBackend>());
}

} // extern "C"

} // namespace wasmfs
