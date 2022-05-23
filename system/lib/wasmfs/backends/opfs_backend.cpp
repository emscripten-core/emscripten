// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "backend.h"
#include "file.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"
#include <stdlib.h>

using namespace wasmfs;

extern "C" {

// Ensure that the root OPFS directory is initialized with ID 0.
void _wasmfs_opfs_init_root_directory(em_proxying_ctx* ctx);

// Look up the child under `parent` with `name`. Write 1 to `child_type` if it's
// a regular file or 2 if it's a directory. Write the child's file or directory
// ID to `child_id`, or -1 if the child does not exist, or -2 if the child
// exists but cannot be opened.
void _wasmfs_opfs_get_child(em_proxying_ctx* ctx,
                            int parent,
                            const char* name,
                            int* child_type,
                            int* child_id);

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

void _wasmfs_opfs_move(em_proxying_ctx* ctx,
                       int file_id,
                       int new_dir_id,
                       const char* name);

void _wasmfs_opfs_remove_child(em_proxying_ctx* ctx,
                               int dir_id,
                               const char* name);

void _wasmfs_opfs_get_entries(em_proxying_ctx* ctx,
                              int dirID,
                              std::vector<Directory::Entry>* entries);

void _wasmfs_opfs_open(em_proxying_ctx* ctx, int file_id, int* access_id);

void _wasmfs_opfs_close(em_proxying_ctx* ctx, int access_id);

void _wasmfs_opfs_free_file(int file_id);

void _wasmfs_opfs_free_directory(int dir_id);

// Synchronous read. Return the number of bytes read.
int _wasmfs_opfs_read(int access_id, uint8_t* buf, uint32_t len, uint32_t pos);

// Synchronous write. Return the number of bytes written.
int _wasmfs_opfs_write(int access_id,
                       const uint8_t* buf,
                       uint32_t len,
                       uint32_t pos);

// Get the size via an AccessHandle.
void _wasmfs_opfs_get_size_access(em_proxying_ctx* ctx,
                                  int access_id,
                                  uint32_t* size);

// Get the size via a File Blob.
void _wasmfs_opfs_get_size_blob(em_proxying_ctx* ctx,
                                int access_id,
                                uint32_t* size);

void _wasmfs_opfs_set_size(em_proxying_ctx* ctx, int access_id, uint32_t size);

void _wasmfs_opfs_flush(em_proxying_ctx* ctx, int access_id);

} // extern "C"

namespace {

using ProxyWorker = emscripten::ProxyWorker;

class OPFSFile : public DataFile {
public:
  ProxyWorker& proxy;

  // The IDs of the corresponding file handle and, if the file is open, the
  // corresponding access handle.
  int fileID;
  int accessID = -1;

  // The number of times this file has been opened. We only close its
  // AccessHandle when this falls to zero.
  size_t openCount = 0;

  OPFSFile(mode_t mode, backend_t backend, int fileID, ProxyWorker& proxy)
    : DataFile(mode, backend), fileID(fileID), proxy(proxy) {}

  ~OPFSFile() override {
    assert(openCount == 0);
    assert(accessID == -1);
    proxy([&]() { _wasmfs_opfs_free_file(fileID); });
  }

private:
  size_t getSize() override {
    uint32_t size;
    if (accessID == -1) {
      proxy(
        [&](auto ctx) { _wasmfs_opfs_get_size_blob(ctx.ctx, fileID, &size); });
    } else {
      proxy([&](auto ctx) {
        _wasmfs_opfs_get_size_access(ctx.ctx, accessID, &size);
      });
    }
    return size_t(size);
  }

  void setSize(size_t size) override {
    proxy([&](auto ctx) { _wasmfs_opfs_set_size(ctx.ctx, accessID, size); });
  }

  void open(oflags_t flags) override {
    if (openCount == 0) {
      proxy([&](auto ctx) { _wasmfs_opfs_open(ctx.ctx, fileID, &accessID); });
      ++openCount;
    }
    // TODO: proper error handling.
    assert(accessID >= 0);
  }

  void close() override {
    assert(openCount >= 1);
    if (--openCount == 0) {
      proxy([&](auto ctx) { _wasmfs_opfs_close(ctx.ctx, accessID); });
      accessID = -1;
    }
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nread;
    proxy([&]() { nread = _wasmfs_opfs_read(accessID, buf, len, offset); });
    return nread;
  }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nwritten;
    proxy([&]() { nwritten = _wasmfs_opfs_write(accessID, buf, len, offset); });
    return nwritten;
  }

  void flush() override {
    proxy([&](auto ctx) { _wasmfs_opfs_flush(ctx.ctx, accessID); });
  }
};

class OPFSDirectory : public Directory {
public:
  ProxyWorker& proxy;

  // The ID of this directory in the JS library.
  int dirID = 0;

  OPFSDirectory(mode_t mode, backend_t backend, int dirID, ProxyWorker& proxy)
    : Directory(mode, backend), dirID(dirID), proxy(proxy) {}

  ~OPFSDirectory() override {
    // Never free the root directory ID.
    if (dirID != 0) {
      proxy([&]() { _wasmfs_opfs_free_directory(dirID); });
    }
  }

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
      return std::make_shared<OPFSDirectory>(
        0777, getBackend(), childID, proxy);
    } else {
      WASMFS_UNREACHABLE("Unexpected child type");
    }
  }

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override {
    int childID = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_insert_file(ctx.ctx, dirID, name.c_str(), &childID);
    });
    // TODO: Handle errors gracefully.
    assert(childID >= 0);
    return std::make_shared<OPFSFile>(mode, getBackend(), childID, proxy);
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    int childID = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_insert_directory(ctx.ctx, dirID, name.c_str(), &childID);
    });
    // TODO: Handle errors gracefully.
    assert(childID >= 0);
    return std::make_shared<OPFSDirectory>(mode, getBackend(), childID, proxy);
  }

  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target) override {
    // Symlinks not supported.
    return nullptr;
  }

  bool insertMove(const std::string& name,
                  std::shared_ptr<File> file) override {
    auto old_file = std::static_pointer_cast<OPFSFile>(file);
    proxy([&](auto ctx) {
      _wasmfs_opfs_move(ctx.ctx, old_file->fileID, dirID, name.c_str());
    });
    // TODO: Handle errors.
    return true;
  }

  bool removeChild(const std::string& name) override {
    proxy([&](auto ctx) {
      _wasmfs_opfs_remove_child(ctx.ctx, dirID, name.c_str());
    });
    return true;
  }

  size_t getNumEntries() override { return getEntries().size(); }

  std::vector<Directory::Entry> getEntries() override {
    std::vector<Directory::Entry> entries;
    proxy(
      [&](auto ctx) { _wasmfs_opfs_get_entries(ctx.ctx, dirID, &entries); });
    return entries;
  }
};

class OPFSBackend : public Backend {
public:
  ProxyWorker proxy;

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    // No way to support a raw file without a parent directory.
    // TODO: update the core system to document this as a possible result of
    // `createFile` and to handle it gracefully.
    return nullptr;
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    proxy([](auto ctx) { _wasmfs_opfs_init_root_directory(ctx.ctx); });
    return std::make_shared<OPFSDirectory>(mode, this, 0, proxy);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    // Symlinks not supported.
    return nullptr;
  }
};

} // anonymous namespace

extern "C" {

backend_t wasmfs_create_opfs_backend() {
  return wasmFS.addBackend(std::make_unique<OPFSBackend>());
}

void EMSCRIPTEN_KEEPALIVE _wasmfs_opfs_record_entry(
  std::vector<Directory::Entry>* entries, const char* name, int type) {
  entries->push_back({name, File::FileKind(type), 0});
}

} // extern "C"
