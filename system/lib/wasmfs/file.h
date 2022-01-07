// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include <assert.h>
#include <emscripten/html5.h>
#include <mutex>
#include <optional>
#include <sys/stat.h>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

// Note: The general locking strategy for all Files is to only hold 1 lock at a
// time to prevent deadlock. This methodology can be seen in getDirs().

class Backend;
// This represents an opaque pointer to a Backend. A user may use this to
// specify a backend in file operations.
using backend_t = Backend*;
const backend_t NullBackend = nullptr;

class File : public std::enable_shared_from_this<File> {

public:
  enum FileKind { DataFileKind = 0, DirectoryKind, SymlinkKind };

  template<class T> bool is() const {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    return int(kind) == int(T::expectedKind);
  }

  template<class T> std::shared_ptr<T> dynCast() {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    if (int(kind) == int(T::expectedKind)) {
      return std::static_pointer_cast<T>(shared_from_this());
    } else {
      return nullptr;
    }
  }

  template<class T> std::shared_ptr<T> cast() {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    assert(int(kind) == int(T::expectedKind));
    return std::static_pointer_cast<T>(shared_from_this());
  }

  ino_t getIno() {
    // Set inode number to the file pointer. This gives a unique inode number.
    // TODO: For security it would be better to use an indirect mapping.
    // Ensure that the pointer will not overflow an ino_t.
    static_assert(sizeof(this) <= sizeof(ino_t));
    return (ino_t)this;
  }

  backend_t getBackend() { return backend; }

  class Handle {

  protected:
    // This mutex is needed when one needs to access access a previously locked
    // file in the same thread. For example, rename will need to traverse
    // 2 paths and access the same locked directory twice.
    // TODO: During benchmarking, test recursive vs normal mutex performance.
    std::unique_lock<std::recursive_mutex> lock;
    std::shared_ptr<File> file;

  public:
    Handle(std::shared_ptr<File> file) : file(file), lock(file->mutex) {}
    Handle(std::shared_ptr<File> file, std::defer_lock_t)
      : file(file), lock(file->mutex, std::defer_lock) {}
    bool trylock() { return lock.try_lock(); }
    size_t getSize() { return file->getSize(); }
    mode_t& mode() { return file->mode; }
    time_t& ctime() { return file->ctime; }
    time_t& mtime() { return file->mtime; }
    time_t& atime() { return file->atime; }

    // Note: parent.lock() creates a new shared_ptr to the same Directory
    // specified by the parent weak_ptr.
    std::shared_ptr<File> getParent() { return file->parent.lock(); }
    void setParent(std::shared_ptr<File> parent) { file->parent = parent; }

    std::shared_ptr<File> unlocked() { return file; }
  };

  Handle locked() { return Handle(shared_from_this()); }

  std::optional<Handle> maybeLocked() {
    auto handle = Handle(shared_from_this(), std::defer_lock);
    if (handle.trylock()) {
      return Handle(shared_from_this());
    } else {
      return {};
    }
  }

protected:
  File(FileKind kind, mode_t mode, backend_t backend)
    : kind(kind), mode(mode), backend(backend) {}
  // A mutex is needed for multiple accesses to the same file.
  std::recursive_mutex mutex;

  virtual size_t getSize() = 0;

  mode_t mode = 0; // User and group mode bits for access permission.

  time_t ctime = 0; // Time when the file node was last modified.
  time_t mtime = 0; // Time when the file content was last modified.
  time_t atime = 0; // Time when the content was last accessed.

  FileKind kind;

  // Reference to parent of current file node. This can be used to
  // traverse up the directory tree. A weak_ptr ensures that the ref
  // count is not incremented. This also ensures that there are no cyclic
  // dependencies where the parent and child have shared_ptrs that reference
  // each other. This prevents the case in which an uncollectable cycle occurs.
  std::weak_ptr<File> parent;

  // This specifies which backend a file is associated with.
  backend_t backend;
};

class DataFile : public File {

  virtual __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) = 0;
  virtual __wasi_errno_t
  write(const uint8_t* buf, size_t len, off_t offset) = 0;

public:
  static constexpr FileKind expectedKind = File::DataFileKind;
  DataFile(mode_t mode, backend_t backend)
    : File(File::DataFileKind, mode | S_IFREG, backend) {}
  DataFile(mode_t mode, backend_t backend, mode_t fileType)
    : File(File::DataFileKind, mode | fileType, backend) {}
  virtual ~DataFile() = default;

  class Handle : public File::Handle {

    std::shared_ptr<DataFile> getFile() { return file->cast<DataFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : File::Handle(dataFile) {}
    Handle(Handle&&) = default;

    __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) {
      return getFile()->read(buf, len, offset);
    }
    __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) {
      return getFile()->write(buf, len, offset);
    }

    // This function loads preloaded files from JS Memory into this DataFile.
    // TODO: Make this virtual so specific backends can specialize it for better
    // performance.
    void preloadFromJS(int index);
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class Directory : public File {
public:
  static constexpr FileKind expectedKind = File::DirectoryKind;
  Directory(mode_t mode, backend_t backend)
    : File(File::DirectoryKind, mode | S_IFDIR, backend) {}

  struct Entry {
    std::string name;
    std::shared_ptr<File> file;
  };

  class Handle : public File::Handle {
    std::shared_ptr<Directory> getDir() { return file->cast<Directory>(); }

  public:
    Handle(std::shared_ptr<File> directory) : File::Handle(directory) {}
    Handle(std::shared_ptr<File> directory, std::defer_lock_t)
      : File::Handle(directory, std::defer_lock) {}

    std::shared_ptr<File> getEntry(std::string pathName);

    void setEntry(std::string pathName, std::shared_ptr<File> inserted);

    void unlinkEntry(std::string pathName);

    // Used to obtain the name of a child File in the directory entries vector.
    std::string getName(std::shared_ptr<File> target);

    int getNumEntries() { return getDir()->entries.size(); }

    std::vector<Directory::Entry> getEntries() { return getDir()->entries; };

#ifdef WASMFS_DEBUG
    void printKeys() {
      for (const auto& entry : getDir()->entries) {
        emscripten_console_log(entry.name.c_str());
      }
    }
#endif
  };

  Handle locked() { return Handle(shared_from_this()); }

  std::optional<Handle> maybeLocked() {
    auto handle = Handle(shared_from_this(), std::defer_lock);
    if (handle.trylock()) {
      return Handle(shared_from_this());
    } else {
      return {};
    }
  }

protected:
  // A vector (instead of a map) saves on code size, but will lead to linear
  // search time in certain operations, ex. getEntry().
  std::vector<Entry> entries;
  // 4096 bytes is the size of a block in ext4.
  // This value was also copied from the JS file system.
  size_t getSize() override { return 4096; }
};

struct ParsedPath {
  std::optional<Directory::Handle> parent;
  std::shared_ptr<File> child;
};

// Call getParsedPath if one needs a locked handle to a parent dir and a
// shared_ptr to its child file, given a file path.
// TODO: When locking the directory structure is refactored, parent should be
// returned as a pointer, similar to child.
// Will return an empty handle if the parent is not a directory.
// Will error if the forbiddenAncestor is encountered while processing.
// If the forbiddenAncestor is encountered, err will be set to EINVAL and
// an empty parent handle will be returned.
ParsedPath getParsedPath(std::vector<std::string> pathParts,
                         long& err,
                         std::shared_ptr<File> forbiddenAncestor = nullptr);

// Call getDir if one needs a parent directory of a file path.
// TODO: Remove this when directory structure locking is refactored and use
// getParsedPath instead. Obtains parent directory of a given pathname.
// Will return a nullptr if the parent is not a directory. Will error if the
// forbiddenAncestor is encountered while processing. If the forbiddenAncestor
// is encountered, err will be set to EINVAL and nullptr will be returned.
std::shared_ptr<Directory>
getDir(std::vector<std::string>::iterator begin,
       std::vector<std::string>::iterator end,
       long& err,
       std::shared_ptr<File> forbiddenAncestor = nullptr);

// Return a vector of the '/'-delimited components of a path. The first
// element will be "/" iff the path is an absolute path.
std::vector<std::string> splitPath(char* pathname);

} // namespace wasmfs
