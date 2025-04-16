// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/threading.h>
#include <stdlib.h>

#include "backend.h"
#include "file.h"
#include "opfs_backend.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"

using namespace wasmfs;

namespace {

using ProxyWorker = emscripten::ProxyWorker;
using ProxyingQueue = emscripten::ProxyingQueue;

class Worker {
public:
#ifdef __EMSCRIPTEN_PTHREADS__
  ProxyWorker proxy;

  template<typename T> void operator()(T func) { proxy(func); }
#else
  // When used with JSPI on the main thread the various wasmfs_opfs_* functions
  // can be directly executed since they are all async.
  template<typename T> void operator()(T func) {
    if constexpr (std::is_invocable_v<T&, ProxyingQueue::ProxyingCtx>) {
      // TODO: Find a way to remove this, since it's unused.
      ProxyingQueue::ProxyingCtx p;
      func(p);
    } else {
      func();
    }
  }
#endif
};

class OpenState {
public:
  enum Kind { None, Access, Blob };

private:
  Kind kind = None;
  int id = -1;
  size_t openCount = 0;

public:
  Kind getKind() { return kind; }

  int open(Worker& proxy, int fileID, oflags_t flags) {
    if (kind == None) {
      assert(openCount == 0);
      switch (flags) {
        case O_RDWR:
        case O_WRONLY:
          // If we need write access, try to open an AccessHandle.
          proxy(
            [&](auto ctx) { _wasmfs_opfs_open_access(ctx.ctx, fileID, &id); });
          // TODO: Fall back to open as a blob instead.
          if (id < 0) {
            return id;
          }
          kind = Access;
          break;
        case O_RDONLY:
          // We only need read access, so open as a Blob
          proxy(
            [&](auto ctx) { _wasmfs_opfs_open_blob(ctx.ctx, fileID, &id); });
          if (id < 0) {
            return id;
          }
          kind = Blob;
          break;
        default:
          WASMFS_UNREACHABLE("Unexpected open access mode");
      }
    } else if (kind == Blob && (flags == O_WRONLY || flags == O_RDWR)) {
      // Try to upgrade to an AccessHandle.
      int newID;
      proxy(
        [&](auto ctx) { _wasmfs_opfs_open_access(ctx.ctx, fileID, &newID); });
      if (newID < 0) {
        return newID;
      }
      // We have an AccessHandle, so close the blob.
      proxy([&]() { _wasmfs_opfs_close_blob(getBlobID()); });
      id = newID;
      kind = Access;
    }
    ++openCount;
    return 0;
  }

  int close(Worker& proxy) {
    // TODO: Downgrade to Blob access once the last writable file descriptor has
    // been closed.
    int err = 0;
    if (--openCount == 0) {
      switch (kind) {
        case Access:
          proxy(
            [&](auto ctx) { _wasmfs_opfs_close_access(ctx.ctx, id, &err); });
          break;
        case Blob:
          proxy([&]() { _wasmfs_opfs_close_blob(id); });
          break;
        case None:
          WASMFS_UNREACHABLE("Open file should have kind");
      }
      kind = None;
      id = -1;
    }
    return err;
  }

  int getAccessID() {
    assert(openCount > 0);
    assert(id >= 0);
    assert(kind == Access);
    return id;
  }

  int getBlobID() {
    assert(openCount > 0);
    assert(id >= 0);
    assert(kind == Blob);
    return id;
  }
};

class OPFSFile : public DataFile {
public:
  Worker& proxy;
  int fileID;
  OpenState state;

  OPFSFile(mode_t mode, backend_t backend, int fileID, Worker& proxy)
    : DataFile(mode, backend), proxy(proxy), fileID(fileID) {}

  ~OPFSFile() override {
    assert(state.getKind() == OpenState::None);
    proxy([&]() { _wasmfs_opfs_free_file(fileID); });
  }

private:
  off_t getSize() override {
    off_t size;
    switch (state.getKind()) {
      case OpenState::None:
        proxy([&](auto ctx) {
          _wasmfs_opfs_get_size_file(ctx.ctx, fileID, &size);
        });
        break;
      case OpenState::Access:
        proxy([&](auto ctx) {
          _wasmfs_opfs_get_size_access(ctx.ctx, state.getAccessID(), &size);
        });
        break;
      case OpenState::Blob:
        proxy([&]() { size = _wasmfs_opfs_get_size_blob(state.getBlobID()); });
        break;
      default:
        WASMFS_UNREACHABLE("Unexpected open state");
    }
    return size;
  }

  int setSize(off_t size) override {
    int err = 0;
    switch (state.getKind()) {
      case OpenState::Access:
        proxy([&](auto ctx) {
          _wasmfs_opfs_set_size_access(
            ctx.ctx, state.getAccessID(), size, &err);
        });
        break;
      case OpenState::Blob:
        // We don't support `truncate` in blob mode because the blob would
        // become invalidated and refreshing it while ensuring other in-flight
        // operations on the same file do not observe the invalidated blob would
        // be extremely complicated.
        // TODO: Can we assume there are no other in-flight operations on this
        // file and do something better here?
        return -EIO;
      case OpenState::None: {
        proxy([&](auto ctx) {
          _wasmfs_opfs_set_size_file(ctx.ctx, fileID, size, &err);
        });
        break;
      }
      default:
        WASMFS_UNREACHABLE("Unexpected open state");
    }
    return err;
  }

  int open(oflags_t flags) override { return state.open(proxy, fileID, flags); }

  int close() override { return state.close(proxy); }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    // TODO: use an i64 here.
    int32_t nread;
    switch (state.getKind()) {
      case OpenState::Access:
        proxy([&]() {
          nread =
            _wasmfs_opfs_read_access(state.getAccessID(), buf, len, offset);
        });
        break;
      case OpenState::Blob:
        proxy([&](auto ctx) {
          _wasmfs_opfs_read_blob(
            ctx.ctx, state.getBlobID(), buf, len, offset, &nread);
        });
        break;
      case OpenState::None:
      default:
        WASMFS_UNREACHABLE("Unexpected open state");
    }
    return nread;
  }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    assert(state.getKind() == OpenState::Access);
    // TODO: use an i64 here.
    int32_t nwritten;
    proxy([&]() {
      nwritten =
        _wasmfs_opfs_write_access(state.getAccessID(), buf, len, offset);
    });
    return nwritten;
  }

  int flush() override {
    int err = 0;
    switch (state.getKind()) {
      case OpenState::Access:
        proxy([&](auto ctx) {
          _wasmfs_opfs_flush_access(ctx.ctx, state.getAccessID(), &err);
        });
        break;
      case OpenState::Blob:
      case OpenState::None:
      default:
        break;
    }
    return err;
  }
};

class OPFSDirectory : public Directory {
public:
  Worker& proxy;

  // The ID of this directory in the JS library.
  int dirID = 0;

  OPFSDirectory(mode_t mode, backend_t backend, int dirID, Worker& proxy)
    : Directory(mode, backend), proxy(proxy), dirID(dirID) {}

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
    if (childID < 0) {
      // TODO: Propagate specific errors.
      return nullptr;
    }
    return std::make_shared<OPFSFile>(mode, getBackend(), childID, proxy);
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    int childID = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_insert_directory(ctx.ctx, dirID, name.c_str(), &childID);
    });
    if (childID < 0) {
      // TODO: Propagate specific errors.
      return nullptr;
    }
    return std::make_shared<OPFSDirectory>(mode, getBackend(), childID, proxy);
  }

  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target) override {
    // Symlinks not supported.
    // TODO: Propagate EPERM specifically.
    return nullptr;
  }

  int insertMove(const std::string& name, std::shared_ptr<File> file) override {
    int err = 0;
    if (file->is<DataFile>()) {
      auto opfsFile = std::static_pointer_cast<OPFSFile>(file);
      proxy([&](auto ctx) {
        _wasmfs_opfs_move_file(
          ctx.ctx, opfsFile->fileID, dirID, name.c_str(), &err);
      });
    } else {
      // TODO: Support moving directories once OPFS supports that.
      // EBUSY can be returned when the directory is "in use by the system,"
      // which can mean whatever we want.
      err = -EBUSY;
    }
    return err;
  }

  int removeChild(const std::string& name) override {
    int err = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_remove_child(ctx.ctx, dirID, name.c_str(), &err);
    });
    return err;
  }

  ssize_t getNumEntries() override {
    auto entries = getEntries();
    if (int err = entries.getError()) {
      return err;
    }
    return entries->size();
  }

  Directory::MaybeEntries getEntries() override {
    std::vector<Directory::Entry> entries;
    int err = 0;
    proxy([&](auto ctx) {
      _wasmfs_opfs_get_entries(ctx.ctx, dirID, &entries, &err);
    });
    if (err) {
      assert(err < 0);
      return {err};
    }
    return {entries};
  }
};

class OPFSBackend : public Backend {
public:
  Worker proxy;

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    // No way to support a raw file without a parent directory.
    // TODO: update the core system to document this as a possible result of
    // `createFile` and to handle it gracefully.
    return nullptr;
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    proxy([](auto ctx) { _wasmfs_opfs_init_root_directory(ctx.ctx); });
    return std::make_shared<OPFSDirectory>(mode, this, 1, proxy);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    // Symlinks not supported.
    return nullptr;
  }
};

} // anonymous namespace

extern "C" {

backend_t wasmfs_create_opfs_backend() {
  // ProxyWorker cannot safely be synchronously spawned from the main browser
  // thread. See comment in thread_utils.h for more details.
  assert(
    !emscripten_is_main_browser_thread() ||
    emscripten_has_asyncify() == 2 &&
      "Cannot safely create OPFS backend on main browser thread without JSPI");

  return wasmFS.addBackend(std::make_unique<OPFSBackend>());
}

void EMSCRIPTEN_KEEPALIVE _wasmfs_opfs_record_entry(
  std::vector<Directory::Entry>* entries, const char* name, int type) {
  entries->push_back({name, File::FileKind(type), 0});
}

} // extern "C"
