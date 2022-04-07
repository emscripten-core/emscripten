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
class Directory;

// This represents an opaque pointer to a Backend. A user may use this to
// specify a backend in file operations.
using backend_t = Backend*;
const backend_t NullBackend = nullptr;

// Access mode, file creation and file status flags for open.
using oflags_t = uint32_t;

class File : public std::enable_shared_from_this<File> {
public:
  enum FileKind { UnknownKind, DataFileKind, DirectoryKind, SymlinkKind };

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
    : kind(kind), mode(mode), backend(backend) {}
  // A mutex is needed for multiple accesses to the same file.
  std::recursive_mutex mutex;

  virtual size_t getSize() = 0;

  mode_t mode = 0; // User and group mode bits for access permission.

  time_t ctime = 0; // Time when the file node was last modified.
  time_t mtime = 0; // Time when the file content was last modified.
  time_t atime = 0; // Time when the content was last accessed.

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
  // Notify the backend when this file is opened or closed. The backend is
  // responsible for keeping files accessible as long as they are open, even if
  // they are unlinked.
  // TODO: Report errors.
  virtual void open(oflags_t flags) = 0;
  virtual void close() = 0;

  // TODO: Allow backends to override the version of read with multiple iovecs
  // to make it possible to implement pipes. See #16269.
  virtual __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) = 0;
  virtual __wasi_errno_t
  write(const uint8_t* buf, size_t len, off_t offset) = 0;

  // Sets the size of the file to a specific size. If new space is allocated, it
  // should be zero-initialized (often backends have an efficient way to do this
  // while doing the resizing).
  virtual void setSize(size_t size) = 0;

  // TODO: Design a proper API for flushing files.
  virtual void flush() = 0;

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

private:
  // Return the file with the given name or null if there is none.
  virtual std::shared_ptr<File> getChild(const std::string& name) = 0;
  // Remove the file with the given name, returning `true` on success or if the
  // child has already been removed.
  virtual bool removeChild(const std::string& name) = 0;
  // Insert the given file with the given name if there is not already an entry
  // with the same name. Returns the inserted file or the preexisting file or
  // null if the file could not be inserted and there was also no preexisting
  // file.
  virtual std::shared_ptr<File> insertChild(const std::string& name,
                                            std::shared_ptr<File> file) = 0;
  // Return the name of the file if it is contained within this directory or an
  // empty string if it is not.
  virtual std::string getName(std::shared_ptr<File> file) = 0;
  // The number of entries in this directory.
  virtual size_t getNumEntries() = 0;
  // The list of entries in this directory.
  virtual std::vector<Directory::Entry> getEntries() = 0;

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
  size_t getSize() override { return 4096; }
};

class Symlink : public File {
protected:
  // The target file that this symlink points to. This is constant as symlinks
  // cannot be modified to point to different things.
  const std::string target;

  size_t getSize() override;

public:
  static constexpr FileKind expectedKind = File::SymlinkKind;
  // Note that symlinks provide a mode of 0 to File. The mode of a symlink does
  // not matter, so that value will never be read (what matters is the mode of
  // the target).
  Symlink(std::string target, backend_t backend)
    : File(File::SymlinkKind, 0, backend), target(target) {}
  virtual ~Symlink() = default;

  // Constant, and therefore thread-safe, and can be done without locking.
  const std::string& getTarget() { return target; }
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
  Handle(std::shared_ptr<File> file) : file(file), lock(file->mutex) {}
  Handle(std::shared_ptr<File> file, std::defer_lock_t)
    : file(file), lock(file->mutex, std::defer_lock) {}
  size_t getSize() { return file->getSize(); }
  mode_t getMode() { return file->mode; }
  void setMode(mode_t mode) { file->mode = mode; }
  time_t getCTime() { return file->ctime; }
  void setCTime(time_t time) { file->ctime = time; }
  time_t getMTime() { return file->mtime; }
  void setMTime(time_t time) { file->mtime = time; }
  time_t getATime() { return file->atime; }
  void setATime(time_t time) { file->atime = time; }

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

  void open(oflags_t flags) { getFile()->open(flags); }
  void close() { getFile()->close(); }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) {
    return getFile()->read(buf, len, offset);
  }
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) {
    return getFile()->write(buf, len, offset);
  }

  void setSize(size_t size) { return getFile()->setSize(size); }

  // TODO: Design a proper API for flushing files.
  void flush() { getFile()->flush(); }

  // This function loads preloaded files from JS Memory into this DataFile.
  // TODO: Make this virtual so specific backends can specialize it for better
  // performance.
  void preloadFromJS(int index);
};

class Directory::Handle : public File::Handle {
  std::shared_ptr<Directory> getDir() { return file->cast<Directory>(); }

public:
  Handle(std::shared_ptr<File> directory) : File::Handle(directory) {}
  Handle(std::shared_ptr<File> directory, std::defer_lock_t)
    : File::Handle(directory, std::defer_lock) {}

  std::shared_ptr<File> getChild(const std::string& name) {
    // Unlinked directories must be empty, without even "." or ".."
    if (!getParent()) {
      return nullptr;
    }
    if (name == ".") {
      return file;
    }
    if (name == "..") {
      return getParent();
    }
    return getDir()->getChild(name);
  }
  bool removeChild(const std::string& name);
  std::shared_ptr<File> insertChild(const std::string& name,
                                    std::shared_ptr<File> file);
  std::string getName(std::shared_ptr<File> file) {
    return getDir()->getName(file);
  }
  size_t getNumEntries() { return getDir()->getNumEntries(); }
  std::vector<Directory::Entry> getEntries() { return getDir()->getEntries(); }
};

inline File::Handle File::locked() { return Handle(shared_from_this()); }

inline DataFile::Handle DataFile::locked() {
  return Handle(shared_from_this());
}

inline Directory::Handle Directory::locked() {
  return Handle(shared_from_this());
}

} // namespace wasmfs
