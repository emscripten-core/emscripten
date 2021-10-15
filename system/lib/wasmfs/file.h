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
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

class Directory;
class DataFile;

class File : public std::enable_shared_from_this<File> {

public:
  enum FileKind { DataFileKind = 0, DirectoryKind, SymlinkKind };

  template<class T> bool is() const {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    return int(kind) == int(T::expectedKind);
  }

  template<class T> T* dynCast() {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    return int(kind) == int(T::expectedKind) ? (T*)this : nullptr;
  }

  template<class T> T* cast() {
    static_assert(std::is_base_of<File, T>::value,
                  "File is not a base of destination type T");
    assert(int(kind) == int(T::expectedKind));
    return (T*)this;
  }

  class Handle {

    std::unique_lock<std::mutex> lock;

  protected:
    std::shared_ptr<File> file;

  public:
    Handle(std::shared_ptr<File> file) : file(file), lock(file->mutex) {}
    size_t& size() { return file->size; }
    uint32_t& mode() { return file->mode; }
    time_t& ctime() { return file->ctime; }
    time_t& mtime() { return file->mtime; }
    time_t& atime() { return file->atime; }
  };

  Handle locked() { return Handle(shared_from_this()); }

protected:
  File(FileKind kind) : kind(kind) {}
  // A mutex is needed for multiple accesses to the same file.
  std::mutex mutex;

private:
  size_t size = 0;

  uint32_t mode = 0; // r/w/x modes

  time_t ctime = 0; // Time when the file node was last modified
  time_t mtime = 0; // Time when the file content was last modified
  time_t atime = 0; // Time when the content was last accessed

  FileKind kind;
};

class DataFile : public File {

  virtual __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) = 0;
  virtual __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) = 0;

public:
  static constexpr FileKind expectedKind = File::DataFileKind;
  DataFile() : File(File::DataFileKind) {}
  virtual ~DataFile() = default;
  class Handle : public File::Handle {

    DataFile& getFile() { return *file.get()->cast<DataFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : File::Handle(dataFile) {}
    Handle(Handle&&) = default;

    __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) {
      return getFile().read(buf, len);
    }
    __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) {
      return getFile().write(buf, len);
    }
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class Directory : public File {
protected:
  // TODO: maybe change to vector?
  std::map<std::string, std::shared_ptr<File>> entries;

public:
  static constexpr FileKind expectedKind = File::DirectoryKind;
  Directory() : File(File::DirectoryKind) {}
  class Handle : public File::Handle {
    Directory& getDir() { return *file.get()->cast<Directory>(); }

  public:
    Handle(std::shared_ptr<File> directory) : File::Handle(directory) {}

    std::shared_ptr<File> getEntry(std::string pathName) {
      auto it = getDir().entries.find(pathName);
      if (it == getDir().entries.end()) {
        return nullptr;
      } else {
        return it->second;
      }
    }
    void setEntry(std::string pathName, std::shared_ptr<File> inserted) {
      getDir().entries[pathName] = inserted;
    }

#ifdef WASMFS_DEBUG
    void printKeys() {
      for (auto keyPair : getFile().entries) {
        std::vector<char> temp(keyPair.first.begin(), keyPair.first.end());
        emscripten_console_log(&temp[0]);
      }
    }
#endif
  };

  Handle locked() { return Handle(shared_from_this()); }
};

} // namespace wasmfs
