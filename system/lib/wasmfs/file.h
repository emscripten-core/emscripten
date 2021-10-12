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
  enum FileType { DataFileType = 0, DirectoryType, SymlinkType };

  // curr->is<Directory>()
  template <class T> bool is() const { return int(fileType) == int(T::fileType); }

  class Handle {

    std::unique_lock<std::mutex> lock;

  protected:
    // File& file;
    std::shared_ptr<File> file;

  public:
    Handle(std::shared_ptr<File> file) : file(file), lock(file->mutex) {}
  };

  Handle get() { return Handle(shared_from_this()); }

protected:
  File(FileType fileType) : fileType(fileType) {}
  // A mutex is needed for multiple accesses to the same file.
  std::mutex mutex;

private:
  // TODO: Add other File properties later.

  FileType fileType;
};

class DataFile : public File {

  virtual __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) = 0;
  virtual __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) = 0;

public:
  enum { FileType = DataFileType };
  DataFile() : File(File::DataFileType) {}
  virtual ~DataFile() = default;
  class Handle : public File::Handle {

    DataFile* getFile() { return static_cast<DataFile*>(file.get()); }

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
  Directory() : File(File::DirectoryType) {}
  class Handle : public File::Handle {
    Directory* getFile() { return static_cast<Directory*>(file.get()); }

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
