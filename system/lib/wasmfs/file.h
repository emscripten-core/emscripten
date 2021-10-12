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
#include <unordered_map>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

class File : public std::enable_shared_from_this<File> {

public:
  enum FileKind { DataFileKind = 0, DirectoryKind, SymlinkKind };

  // curr->is<Directory>()
  template <class T> bool is() const {
    static_assert(std::is_base_of<File, T>::value, "File is not a base of destination type T");
    return int(kind) == int(T::SpecificFileKind);
  }

  template <class T> T* dynCast() {
    static_assert(std::is_base_of<File, T>::value, "File is not a base of destination type T");
    return int(kind) == int(T::SpecificFileKind) ? (T*)this : nullptr;
  }

  template <class T> const T* dynCast() const {
    static_assert(std::is_base_of<File, T>::value, "File is not a base of destination type T");
    return int(kind) == int(T::SpecificFileKind) ? (const T*)this : nullptr;
  }

  template <class T> T* cast() {
    static_assert(std::is_base_of<File, T>::value, "File is not a base of destination type T");
    assert(int(kind) == int(T::SpecificFileKind));
    return (T*)this;
  }

  template <class T> const T* cast() const {
    static_assert(std::is_base_of<File, T>::value, "File is not a base of destination type T");
    assert(int(kind) == int(T::SpecificFileKind));
    return (const T*)this;
  }

  class Handle {

    std::unique_lock<std::mutex> lock;

  protected:
    std::shared_ptr<File> file;

  public:
    Handle(std::shared_ptr<File> file) : file(file), lock(file->mutex) {}
  };

  Handle get() { return Handle(shared_from_this()); }

protected:
  File(FileKind kind) : kind(kind) {}
  // A mutex is needed for multiple accesses to the same file.
  std::mutex mutex;

private:
  // TODO: Add other File properties later.

  FileKind kind;
};

class DataFile : public File {

  virtual __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) = 0;
  virtual __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) = 0;

public:
  enum { SpecificFileKind = File::DataFileKind };
  DataFile() : File(File::DataFileKind) {}
  virtual ~DataFile() = default;
  class Handle : public File::Handle {

    DataFile* getFile() { return file.get()->dynCast<DataFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : File::Handle(dataFile) {}
    Handle(Handle&&) = default;

    __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) { return getFile()->read(buf, len); }
    __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) {
      return getFile()->write(buf, len);
    }
  };

  Handle get() { return Handle(shared_from_this()); }
};

class Directory : public File {
protected:
  std::unordered_map<std::string, std::shared_ptr<File>> entries;

public:
  enum { SpecificFileKind = File::DirectoryKind };
  Directory() : File(File::DirectoryKind) {}
  class Handle : public File::Handle {
    Directory* getFile() { return file.get()->dynCast<Directory>(); }

  public:
    Handle(std::shared_ptr<File> directory) : File::Handle(directory) {}

    std::shared_ptr<File> getEntry(std::string pathName) {
      if (getFile()->entries.find(pathName) == getFile()->entries.end()) {
        return nullptr;
      } else {
        return getFile()->entries[pathName];
      }
    }

    void setEntry(std::string pathName, std::shared_ptr<File> inserted) {
      getFile()->entries[pathName] = inserted;
    }

    // For debugging, TODO: delete later.
    void printKeys() {
      for (auto keyPair : getFile()->entries) {
        std::vector<char> temp(keyPair.first.begin(), keyPair.first.end());
        emscripten_console_log(&temp[0]);
      }
    }
  };

  Handle get() { return Handle(shared_from_this()); }
};

} // namespace wasmfs
