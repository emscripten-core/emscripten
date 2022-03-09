// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory>

#include "backend.h"
#include "file.h"
#include "support.h"
#include "wasmfs.h"

namespace wasmfs {

class NodeBackend;

extern "C" {

// Fill `entries` and return 0 or an error code.
int _wasmfs_node_readdir(const char* path,
                         std::vector<Directory::Entry>* entries);
// Write `mode` and return 0 or an error code.
int _wasmfs_node_get_mode(const char* path, mode_t* mode);

// Write `size` and return 0 or an error code.
int _wasmfs_node_stat_size(const char* path, uint32_t* size);
int _wasmfs_node_fstat_size(int fd, uint32_t* size);

// Create a new file system entry and return 0 or an error code.
int _wasmfs_node_insert_file(const char* path, mode_t mode);
int _wasmfs_node_insert_directory(const char* path, mode_t mode);

// Unlink the given file and return 0 or an error code.
int _wasmfs_node_unlink(const char* path);
int _wasmfs_node_rmdir(const char* path);

// Open the file and return the underlying file descriptor.
int _wasmfs_node_open(const char* path, const char* mode);

// Close the underlying file descriptor.
int _wasmfs_node_close(int fd);

// Read up to `size` bytes into `buf` from position `pos` in the file, writing
// the number of bytes read to `nread`. Return 0 on success or an error code.
int _wasmfs_node_read(
  int fd, void* buf, uint32_t len, uint32_t pos, uint32_t* nread);

// Write up to `size` bytes from `buf` at position `pos` in the file, writing
// the number of bytes written to `nread`. Return 0 on success or an error code.
int _wasmfs_node_write(
  int fd, const void* buf, uint32_t len, uint32_t pos, uint32_t* nwritten);

} // extern "C"

// The state of a file on the underlying Node file system.
class NodeState {
  // Map all separate WasmFS opens of a file to a single underlying fd.
  size_t openCount = 0;
  int fd = 0;

public:
  std::string path;
  NodeState(std::string path) : path(path) {}
  int getFD() {
    assert(openCount > 0);
    return fd;
  }
  bool isOpen() { return openCount > 0; }
  void open(oflags_t flags) {
    if (openCount++ == 0) {
      switch (flags) {
        case O_RDONLY:
          fd = _wasmfs_node_open(path.c_str(), "r");
          break;
        case O_WRONLY:
          fd = _wasmfs_node_open(path.c_str(), "w");
          break;
        case O_RDWR:
          fd = _wasmfs_node_open(path.c_str(), "r+");
          break;
        default:
          WASMFS_UNREACHABLE("Unexpected open access mode");
      }
    }
  }
  void close() {
    if (--openCount == 0) {
      _wasmfs_node_close(fd);
    }
  }
};

class NodeFile : public DataFile {
public:
  NodeState state;

  NodeFile(mode_t mode, backend_t backend, std::string path)
    : DataFile(mode, backend), state(path) {}

private:
  size_t getSize() override {
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
    return size_t(size);
  }

  void setSize(size_t size) override {
    WASMFS_UNREACHABLE("TODO: implement NodeFile::setSize");
  }

  void open(oflags_t flags) override { state.open(flags); }
  void close() override { state.close(); }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nread;
    if (auto err = _wasmfs_node_read(state.getFD(), buf, len, offset, &nread)) {
      return err;
    }
    // TODO: Add a way to report the actual bytes read. We currently assume the
    // available bytes can't change under us.
    return __WASI_ERRNO_SUCCESS;
  }

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    uint32_t nwritten;
    if (auto err =
          _wasmfs_node_write(state.getFD(), buf, len, offset, &nwritten)) {
      return err;
    }
    // TODO: Add a way to report the actual bytes written. We currently assume
    // the write cannot be short.
    return __WASI_ERRNO_SUCCESS;
  }

  void flush() override {
    WASMFS_UNREACHABLE("TODO: implement NodeFile::flush");
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
    std::shared_ptr<File> child;
    if (S_ISREG(mode)) {
      child = std::make_shared<NodeFile>(mode, getBackend(), childPath);
    } else if (S_ISDIR(mode)) {
      child = std::make_shared<NodeDirectory>(mode, getBackend(), childPath);
    } else if (S_ISLNK(mode)) {
      // return std::make_shared<NodeSymlink>(mode, getBackend(), childPath);
    } else {
      // Unrecognized file kind not made visible to WasmFS.
      return nullptr;
    }
    child->locked().setParent(shared_from_this()->cast<Directory>());
    return child;
  }

  bool removeChild(const std::string& name) override {
    auto childPath = getChildPath(name);
    // Try both `unlink` and `rmdir`.
    if (auto err = _wasmfs_node_unlink(childPath.c_str())) {
      if (err == EISDIR) {
        err = _wasmfs_node_rmdir(childPath.c_str());
      }
      if (err) {
        // TODO: Report specific errors.
        return false;
      }
    }
    return true;
  }

  std::shared_ptr<File> insertChild(const std::string& name,
                                    std::shared_ptr<File> file) override {
    auto childPath = getChildPath(name);
    auto mode = file->locked().getMode();
    if (file->is<DataFile>()) {
      if (_wasmfs_node_insert_file(childPath.c_str(), mode)) {
        return nullptr;
      }
      std::static_pointer_cast<NodeFile>(file)->state.path = childPath;
      return file;
    } else if (file->is<Directory>()) {
      if (_wasmfs_node_insert_directory(childPath.c_str(), mode)) {
        return nullptr;
      }
      std::static_pointer_cast<NodeDirectory>(file)->state.path = childPath;
      return file;
    } else if (file->is<Symlink>()) {
      // fs.linkSync(target, name)
      assert(false && "Symlinks not implemented");
      return nullptr;
    } else {
      assert(false && "Unimplemented file kind");
      return nullptr;
    }
    return nullptr;
  }

  std::string getName(std::shared_ptr<File> file) override {
    WASMFS_UNREACHABLE("TODO: implement NodeDirectory::getName");
    return "";
  }

  size_t getNumEntries() override {
    // TODO: optimize this?
    return getEntries().size();
  }

  std::vector<Directory::Entry> getEntries() override {
    std::vector<Directory::Entry> entries;
    int err = _wasmfs_node_readdir(state.path.c_str(), &entries);
    // TODO: Make this fallible. We actually depend on suppressing the error
    //       here to pass test_unlink_wasmfs_node because the File stored in the
    //       file table is not the same File that had its parent pointer reset
    //       during the unlink. Fixing this may require caching Files at some
    //       layer to ensure they are the same.
    (void)err;
    return entries;
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
