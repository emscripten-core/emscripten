// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "support.h"
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <map>
#include <mutex>
#include <optional>
#include <sys/stat.h>
#include <variant>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

// Note: The general locking strategy for all Files is to only hold 1 lock at a
// time to prevent deadlock. This methodology can be seen in getDirs().

class Backend;
class Directory;
class Symlink;

// This represents an opaque pointer to a Backend. A user may use this to
// specify a backend in file operations.
using backend_t = Backend*;
const backend_t NullBackend = nullptr;

// Access mode, file creation and file status flags for open.
using oflags_t = uint32_t;

// An abstract representation of an underlying file. All `File` objects
// correspond to underlying (real or conceptual) files in a file system managed
// by some backend, but not all underlying files have a corresponding `File`
// object. For example, a persistent backend may contain some files that have
// not yet been discovered by WasmFS and that therefore do not yet have
// corresponding `File` objects. Backends override the `File` family of classes
// to implement the mapping from `File` objects to their underlying files.
class File : public std::enable_shared_from_this<File> {
public:
  enum FileKind {
    UnknownKind = 0,
    DataFileKind = 1,
    DirectoryKind = 2,
    SymlinkKind = 3
  };

  const FileKind kind;

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

  backend_t getBackend() const { return backend; }

  bool isSeekable() const { return seekable; }

  class Handle;
  Handle locked();

protected:
  File(FileKind kind, mode_t mode, backend_t backend)
    : kind(kind), mode(mode), backend(backend) {
    atime = mtime = ctime = emscripten_date_now();
  }

  // A mutex is needed for multiple accesses to the same file.
  std::recursive_mutex mutex;

  // The size in bytes of a file or return a negative error code. May be
  // called on files that have not been opened.
  virtual off_t getSize() = 0;

  mode_t mode = 0; // User and group mode bits for access permission.

  double atime; // Time when the content was last accessed, in ms.
  double mtime; // Time when the file content was last modified, in ms.
  double ctime; // Time when the file node was last modified, in ms.

  // Reference to parent of current file node. This can be used to
  // traverse up the directory tree. A weak_ptr ensures that the ref
  // count is not incremented. This also ensures that there are no cyclic
  // dependencies where the parent and child have shared_ptrs that reference
  // each other. This prevents the case in which an uncollectable cycle occurs.
  std::weak_ptr<Directory> parent;

  // This specifies which backend a file is associated with. It may be null
  // (NullBackend) if there is no particular backend associated with the file.
  backend_t backend;

  // By default files are seekable. The rare exceptions are things like pipes
  // and sockets.
  bool seekable = true;
};

class DataFile : public File {
protected:
  // Notify the backend when this file is opened or closed. The backend is
  // responsible for keeping files accessible as long as they are open, even if
  // they are unlinked. Returns 0 on success or a negative error code.
  virtual int open(oflags_t flags) = 0;
  virtual int close() = 0;

  // Return the accessed length or a negative error code. It is not an error to
  // access fewer bytes than requested. Will only be called on opened files.
  // TODO: Allow backends to override the version of read with
  // multiple iovecs to make it possible to implement pipes. See #16269.
  virtual ssize_t read(uint8_t* buf, size_t len, off_t offset) = 0;
  virtual ssize_t write(const uint8_t* buf, size_t len, off_t offset) = 0;

  // Sets the size of the file to a specific size. If new space is allocated, it
  // should be zero-initialized. May be called on files that have not been
  // opened. Returns 0 on success or a negative error code.
  virtual int setSize(off_t size) = 0;

  // Sync the file data to the underlying persistent storage, if any. Returns 0
  // on success or a negative error code.
  virtual int flush() = 0;

public:
  static constexpr FileKind expectedKind = File::DataFileKind;
  DataFile(mode_t mode, backend_t backend)
    : File(File::DataFileKind, mode | S_IFREG, backend) {}
  DataFile(mode_t mode, backend_t backend, mode_t fileType)
    : File(File::DataFileKind, mode | fileType, backend) {}
  virtual ~DataFile() = default;

  class Handle;
  Handle locked();
};

class Directory : public File {
public:
  struct Entry {
    std::string name;
    FileKind kind;
    ino_t ino;
  };

  struct MaybeEntries : std::variant<std::vector<Entry>, int> {
    int getError() {
      if (int* err = std::get_if<int>(this)) {
        assert(*err < 0);
        return *err;
      }
      return 0;
    }

    std::vector<Entry>& operator*() {
      return *std::get_if<std::vector<Entry>>(this);
    }

    std::vector<Entry>* operator->() {
      return std::get_if<std::vector<Entry>>(this);
    }
  };

private:
  // The directory cache, or `dcache`, stores `File` objects for the children of
  // each directory so that subsequent lookups do not need to query the backend.
  // It also supports cross-backend mount point children that are stored
  // exclusively in the cache and not reflected in any backend.
  enum class DCacheKind { Normal, Mount };
  struct DCacheEntry {
    DCacheKind kind;
    std::shared_ptr<File> file;
  };
  // TODO: Use a cache data structure with smaller code size.
  std::map<std::string, DCacheEntry> dcache;

protected:
  // Return the `File` object corresponding to the file with the given name or
  // null if there is none.
  virtual std::shared_ptr<File> getChild(const std::string& name) = 0;

  // Inserts a file with the given name, kind, and mode. Returns a `File` object
  // corresponding to the newly created file or nullptr if the new file could
  // not be created. Assumes a child with this name does not already exist.
  // If the operation failed, returns nullptr.
  virtual std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                                   mode_t mode) = 0;
  virtual std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                                     mode_t mode) = 0;
  virtual std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                                 const std::string& target) = 0;

  // Move the file represented by `file` from its current directory to this
  // directory with the new `name`, possibly overwriting another file that
  // already exists with that name. The old directory may be the same as this
  // directory. On success return 0 and otherwise return a negative error code
  // without changing any underlying state.
  virtual int insertMove(const std::string& name,
                         std::shared_ptr<File> file) = 0;

  // Remove the file with the given name. Returns zero on success or if the
  // child has already been removed and otherwise returns a negative error code
  // if the child cannot be removed.
  virtual int removeChild(const std::string& name) = 0;

  // The number of entries in this directory. Returns the number of entries or a
  // negative error code.
  virtual ssize_t getNumEntries() = 0;

  // The list of entries in this directory or a negative error code.
  virtual MaybeEntries getEntries() = 0;

  // Only backends that maintain file identity themselves (see below) need to
  // implement this.
  virtual std::string getName(std::shared_ptr<File> file) {
    WASMFS_UNREACHABLE("getName unimplemented");
  }

  // Whether this directory implementation always returns the same `File` object
  // for a given file. Most backends can be much simpler if they don't handle
  // this themselves. Instead, they rely on the directory cache (dcache) to
  // maintain file identity for them by ensuring each file is looked up in the
  // backend only once. Some backends, however, already track file identity, so
  // the dcache is not necessary (or would even introduce problems).
  //
  // When this is `true`, backends are responsible for:
  //
  //  1. Ensuring that all insert* and getChild calls returning a particular
  //     file return the same File object.
  //
  //  2. Clearing unlinked Files' parents in `removeChild` and `insertMove`.
  //
  //  3. Implementing `getName`, since it cannot be implemented in terms of the
  //     dcache.
  virtual bool maintainsFileIdentity() { return false; }

public:
  static constexpr FileKind expectedKind = File::DirectoryKind;
  Directory(mode_t mode, backend_t backend)
    : File(File::DirectoryKind, mode | S_IFDIR, backend) {}
  virtual ~Directory() = default;

  class Handle;
  Handle locked();

protected:
  // 4096 bytes is the size of a block in ext4.
  // This value was also copied from the JS file system.
  off_t getSize() override { return 4096; }
};

class Symlink : public File {
public:
  static constexpr FileKind expectedKind = File::SymlinkKind;
  // Note that symlinks provide a mode of 0 to File. The mode of a symlink does
  // not matter, so that value will never be read (what matters is the mode of
  // the target).
  Symlink(backend_t backend) : File(File::SymlinkKind, S_IFLNK, backend) {}
  virtual ~Symlink() = default;

  // Constant, and therefore thread-safe, and can be done without locking.
  virtual std::string getTarget() const = 0;

protected:
  off_t getSize() override { return getTarget().size(); }
};

class File::Handle {
protected:
  // This mutex is needed when one needs to access access a previously locked
  // file in the same thread. For example, rename will need to traverse
  // 2 paths and access the same locked directory twice.
  // TODO: During benchmarking, test recursive vs normal mutex performance.
  std::unique_lock<std::recursive_mutex> lock;
  std::shared_ptr<File> file;

public:
  Handle(std::shared_ptr<File> file) : lock(file->mutex), file(file) {}
  Handle(std::shared_ptr<File> file, std::defer_lock_t)
    : lock(file->mutex, std::defer_lock), file(file) {}
  off_t getSize() { return file->getSize(); }
  mode_t getMode() { return file->mode; }
  void setMode(mode_t mode) {
    // The type bits can never be changed (whether something is a file or a
    // directory, for example).
    file->mode = (file->mode & S_IFMT) | (mode & ~S_IFMT);
  }
  double getCTime() {
    return file->ctime;
  }
  void setCTime(double time) { file->ctime = time; }
  // updateCTime() updates the ctime to the current time.
  void updateCTime() {
    file->ctime = emscripten_date_now();
  }
  double getMTime() {
    return file->mtime;
  }
  void setMTime(double time) { file->mtime = time; }
  // updateMTime() updates the mtime to the current time.
  void updateMTime() {
    file->mtime = emscripten_date_now();
  }
  double getATime() {
    return file->atime;
  }
  void setATime(double time) { file->atime = time; }
  // updateATime() updates the atime to the current time.
  void updateATime() {
    file->atime = emscripten_date_now();
  }

  // Note: parent.lock() creates a new shared_ptr to the same Directory
  // specified by the parent weak_ptr.
  std::shared_ptr<Directory> getParent() { return file->parent.lock(); }
  void setParent(std::shared_ptr<Directory> parent) { file->parent = parent; }

  std::shared_ptr<File> unlocked() { return file; }
};

class DataFile::Handle : public File::Handle {
  std::shared_ptr<DataFile> getFile() { return file->cast<DataFile>(); }

public:
  Handle(std::shared_ptr<File> dataFile) : File::Handle(dataFile) {}
  Handle(Handle&&) = default;

  [[nodiscard]] int open(oflags_t flags) { return getFile()->open(flags); }
  [[nodiscard]] int close() { return getFile()->close(); }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) {
    return getFile()->read(buf, len, offset);
  }
  ssize_t write(const uint8_t* buf, size_t len, off_t offset) {
    return getFile()->write(buf, len, offset);
  }

  [[nodiscard]] int setSize(off_t size) { return getFile()->setSize(size); }

  // TODO: Design a proper API for flushing files.
  [[nodiscard]] int flush() { return getFile()->flush(); }

  // This function loads preloaded files from JS Memory into this DataFile.
  // TODO: Make this virtual so specific backends can specialize it for better
  // performance.
  void preloadFromJS(int index);
};

class Directory::Handle : public File::Handle {
  std::shared_ptr<Directory> getDir() { return file->cast<Directory>(); }
  void cacheChild(const std::string& name,
                  std::shared_ptr<File> child,
                  DCacheKind kind);

public:
  Handle(std::shared_ptr<File> directory) : File::Handle(directory) {}
  Handle(std::shared_ptr<File> directory, std::defer_lock_t)
    : File::Handle(directory, std::defer_lock) {}

  // Retrieve the child if it is in the dcache and otherwise forward the request
  // to the backend, caching any `File` object it returns.
  std::shared_ptr<File> getChild(const std::string& name);

  // Add a child to this directory's entry cache without actually inserting it
  // in the underlying backend. Assumes a child with this name does not already
  // exist. Return `true` on success and `false` otherwise.
  bool mountChild(const std::string& name, std::shared_ptr<File> file);

  // Insert a child of the given name, kind, and mode in the underlying backend,
  // which will allocate and return a corresponding `File` on success or return
  // nullptr otherwise. Assumes a child with this name does not already exist.
  // If the operation failed, returns nullptr.
  std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                           mode_t mode);
  std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                             mode_t mode);
  std::shared_ptr<Symlink> insertSymlink(const std::string& name,
                                         const std::string& target);

  // Move the file represented by `file` from its current directory to this
  // directory with the new `name`, possibly overwriting another file that
  // already exists with that name. The old directory may be the same as this
  // directory. On success return 0 and otherwise return a negative error code
  // without changing any underlying state. This should only be called from
  // renameat with the locks on the old and new parents already held.
  [[nodiscard]] int insertMove(const std::string& name,
                               std::shared_ptr<File> file);

  // Remove the file with the given name. Returns zero on success or if the
  // child has already been removed and otherwise returns a negative error code
  // if the child cannot be removed.
  [[nodiscard]] int removeChild(const std::string& name);

  std::string getName(std::shared_ptr<File> file);

  [[nodiscard]] ssize_t getNumEntries();
  [[nodiscard]] MaybeEntries getEntries();
};

inline File::Handle File::locked() { return Handle(shared_from_this()); }

inline DataFile::Handle DataFile::locked() {
  return Handle(shared_from_this());
}

inline Directory::Handle Directory::locked() {
  return Handle(shared_from_this());
}

} // namespace wasmfs
