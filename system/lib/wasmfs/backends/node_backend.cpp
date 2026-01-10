// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory>

#include "backend.h"
#include "file.h"
#include "node_backend.h"
#include "support.h"
#include "wasmfs.h"

namespace wasmfs {

class NodeBackend;

// The state of a file on the underlying Node file system.
class NodeState {
  // Map all separate WasmFS opens of a file to a single underlying fd.
  size_t openCount = 0;
  oflags_t openFlags = 0;
  int fd = -1;

public:
  std::string path;

  NodeState(std::string path) : path(path) {}

  int getFD() {
    assert(openCount > 0);
    return fd;
  }

  bool isOpen() { return openCount > 0; }

  // Attempt to open the file with the given flags, returning 0 on success or an
  // error code.
  int open(oflags_t flags) {
    int result = 0;
    if (openCount == 0) {
      // No existing fd, so open a fresh one.
      switch (flags) {
        case O_RDONLY:
          result = _wasmfs_node_open(path.c_str(), "r");
          break;
        case O_WRONLY:
          // TODO(sbc): Specific handling of O_WRONLY.
          // There is no simple way to map O_WRONLY to an fopen-style
          // mode string since the only two modes that are write only
          // are `w` and `a`.  The problem with the former is that it
          // truncates to file.  The problem with the latter is that it
          // opens for appending.  For now simply opening in O_RDWR
          // mode is enough to pass all our tests.
        case O_RDWR:
          result = _wasmfs_node_open(path.c_str(), "r+");
          break;
        default:
          WASMFS_UNREACHABLE("Unexpected open access mode");
      }
      if (result < 0) {
        return result;
      }
      // Fall through to update our state with the new result.
    } else if ((openFlags == O_RDONLY &&
                (flags == O_WRONLY || flags == O_RDWR)) ||
               (openFlags == O_WRONLY &&
                (flags == O_RDONLY || flags == O_RDWR))) {
      // We already have a file descriptor, but we need to replace it with a new
      // fd with more access.
      result = _wasmfs_node_open(path.c_str(), "r+");
      if (result < 0) {
        return result;
      }
      // Success! Close the old fd before updating it.
      (void)_wasmfs_node_close(fd);
      // Fall through to update our state with the new result.
    } else {
      // Reuse the existing file descriptor.
      ++openCount;
      return 0;
    }
    // Update our state for the new fd.
    fd = result;
    openFlags = flags;
    ++openCount;
    return 0;
  }

  int close() {
    int ret = 0;
    if (--openCount == 0) {
      ret = _wasmfs_node_close(fd);
      *this = NodeState(path);
    }
    return ret;
  }
};

class NodeFile : public DataFile {
public:
  NodeState state;

  NodeFile(mode_t mode, backend_t backend, std::string path)
    : DataFile(mode, backend), state(path) {}

private:
  off_t getSize() override {
    // TODO: This should really be using a 64-bit file size type.
    uint32_t size;
    if (state.isOpen()) {
      if (_wasmfs_node_fstat_size(state.getFD(), &size)) {
        // TODO: Make this fallible.
        return 0;
      }
    } else {
      if (_wasmfs_node_stat_size(state.path.c_str(), &size)) {
        // TODO: Make this fallible.
        return 0;
      }
    }
    return off_t(size);
  }

  int setSize(off_t size) override {
    if (state.isOpen()) {
      return _wasmfs_node_ftruncate(state.getFD(), size);
    }
    return _wasmfs_node_truncate(state.path.c_str(), size);
  }

  int open(oflags_t flags) override { return state.open(flags); }

  int close() override { return state.close(); }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nread;
    if (auto err = _wasmfs_node_read(state.getFD(), buf, len, offset, &nread)) {
      return -err;
    }
    return nread;
  }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nwritten;
    if (auto err =
          _wasmfs_node_write(state.getFD(), buf, len, offset, &nwritten)) {
      return -err;
    }
    return nwritten;
  }

  int flush() override {
    WASMFS_UNREACHABLE("TODO: implement NodeFile::flush");
  }
};

class NodeSymlink : public Symlink {
public:
  std::string path;

  NodeSymlink(backend_t backend, std::string path)
    : Symlink(backend), path(path) {}

  virtual std::string getTarget() const {
    char buf[PATH_MAX];
    if (_wasmfs_node_readlink(path.c_str(), buf, PATH_MAX) < 0) {
      WASMFS_UNREACHABLE("getTarget cannot fail");
    }
    return std::string(buf);
  }
};

class NodeDirectory : public Directory {
public:
  NodeState state;

  NodeDirectory(mode_t mode, backend_t backend, std::string path)
    : Directory(mode, backend), state(path) {}

private:
  std::string getChildPath(const std::string& name) {
    return state.path + '/' + name;
  }

  std::shared_ptr<File> getChild(const std::string& name) override {
    static_assert(std::is_same_v<mode_t, unsigned int>);
    // TODO: also retrieve and set ctime, atime, ino, etc.
    auto childPath = getChildPath(name);
    mode_t mode;
    if (_wasmfs_node_get_mode(childPath.c_str(), &mode)) {
      return nullptr;
    }
    if (S_ISREG(mode)) {
      return std::make_shared<NodeFile>(mode, getBackend(), childPath);
    } else if (S_ISDIR(mode)) {
      return std::make_shared<NodeDirectory>(mode, getBackend(), childPath);
    } else if (S_ISLNK(mode)) {
      return std::make_shared<NodeSymlink>(getBackend(), childPath);
    } else {
      // Unrecognized file kind not made visible to WasmFS.
      return nullptr;
    }
  }

  int removeChild(const std::string& name) override {
    auto childPath = getChildPath(name);
    // Try both `unlink` and `rmdir`.
    if (auto err = _wasmfs_node_unlink(childPath.c_str())) {
      if (err == EISDIR) {
        err = _wasmfs_node_rmdir(childPath.c_str());
      }
      return -err;
    }
    return 0;
  }

  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode) override {
    auto childPath = getChildPath(name);
    if (_wasmfs_node_insert_file(childPath.c_str(), mode)) {
      return nullptr;
    }
    return std::make_shared<NodeFile>(mode, getBackend(), childPath);
  }

  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode) override {
    auto childPath = getChildPath(name);
    if (_wasmfs_node_insert_directory(childPath.c_str(), mode)) {
      return nullptr;
    }
    return std::make_shared<NodeDirectory>(mode, getBackend(), childPath);
  }

  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target) override {
    auto childPath = getChildPath(name);
    if (_wasmfs_node_symlink(target.c_str(), childPath.c_str())) {
      return nullptr;
    }
    return std::make_shared<NodeSymlink>(getBackend(), childPath);
  }

  int insertMove(const std::string& name, std::shared_ptr<File> file) override {
    std::string fromPath;

    if (file->is<DataFile>()) {
      auto nodeFile = std::static_pointer_cast<NodeFile>(file);
      fromPath = nodeFile->state.path;
    } else {
      auto nodeDir = std::static_pointer_cast<NodeDirectory>(file);
      fromPath = nodeDir->state.path;
    }

    auto childPath = getChildPath(name);
    return _wasmfs_node_rename(fromPath.c_str(), childPath.c_str());
  }

  ssize_t getNumEntries() override {
    // TODO: optimize this?
    auto entries = getEntries();
    if (int err = entries.getError()) {
      return err;
    }
    return entries->size();
  }

  Directory::MaybeEntries getEntries() override {
    std::vector<Directory::Entry> entries;
    int err = _wasmfs_node_readdir(state.path.c_str(), &entries);
    if (err) {
      return {-err};
    }
    return {entries};
  }
};

class NodeBackend : public Backend {
  // The underlying Node FS path of this backend's mount points.
  std::string mountPath;

public:
  NodeBackend(const std::string& mountPath) : mountPath(mountPath) {}

  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<NodeFile>(mode, this, mountPath);
  }

  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<NodeDirectory>(mode, this, mountPath);
  }

  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    WASMFS_UNREACHABLE("TODO: implement NodeBackend::createSymlink");
  }
};

// TODO: symlink

extern "C" {

backend_t wasmfs_create_node_backend(const char* root) {
  return wasmFS.addBackend(std::make_unique<NodeBackend>(root));
}

void EMSCRIPTEN_KEEPALIVE _wasmfs_node_record_dirent(
  std::vector<Directory::Entry>* entries, const char* name, int type) {
  entries->push_back({name, File::FileKind(type), 0});
}

} // extern "C"

} // namespace wasmfs
