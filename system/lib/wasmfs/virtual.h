// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Utilities for creating virtual backends that forward operations to underlying
// "real" files. Virtual backends must wrap all underlying files, even if the
// wrapper is a no-op, so that no part of the system observes mixed backends. To
// the backend-independent code, the root directory of the virtual backend and
// all of its descendants must appear to have the same virtual backend. To the
// underlying "real" backend, all of its files must appear to have parents of
// the same real backend. Without these invariants, renames would not work
// correctly and without wrapping all files, these invariants would not be
// upheld.

#pragma once

#include "file.h"

namespace wasmfs {

static inline std::shared_ptr<File> devirtualize(std::shared_ptr<File>);

class VirtualDataFile : public DataFile {
protected:
  std::shared_ptr<DataFile> real;

public:
  VirtualDataFile(std::shared_ptr<DataFile> real, backend_t backend)
    : DataFile(real->locked().getMode(), backend), real(real) {}

protected:
  virtual off_t getSize() override { return real->locked().getSize(); }
  virtual int open(oflags_t flags) override {
    return real->locked().open(flags);
  }
  virtual int close() override { return real->locked().close(); }
  virtual ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    return real->locked().read(buf, len, offset);
  }
  virtual ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return real->locked().write(buf, len, offset);
  }
  virtual int setSize(off_t size) override {
    return real->locked().setSize(size);
  }
  virtual int flush() override { return real->locked().flush(); }

  friend std::shared_ptr<File> devirtualize(std::shared_ptr<File>);
};

// Forward operations through Directory::Handle to get the proper dcache logic.
class VirtualDirectory : public Directory {
protected:
  std::shared_ptr<Directory> real;

public:
  VirtualDirectory(std::shared_ptr<Directory> real, backend_t backend)
    : Directory(real->locked().getMode(), backend), real(real) {}

protected:
  virtual std::shared_ptr<File> getChild(const std::string& name) override {
    return real->locked().getChild(name);
  }
  virtual std::shared_ptr<DataFile> insertDataFile(const std::string& name,
                                                   mode_t mode) override {
    return real->locked().insertDataFile(name, mode);
  }
  virtual std::shared_ptr<Directory> insertDirectory(const std::string& name,
                                                     mode_t mode) override {
    return real->locked().insertDirectory(name, mode);
  }
  virtual std::shared_ptr<Symlink>
  insertSymlink(const std::string& name, const std::string& target) override {
    return real->locked().insertSymlink(name, target);
  }
  virtual int insertMove(const std::string& name,
                         std::shared_ptr<File> file) override {
    return real->locked().insertMove(name, file);
  }
  virtual int removeChild(const std::string& name) override {
    return real->locked().removeChild(name);
  }
  virtual ssize_t getNumEntries() override {
    return real->locked().getNumEntries();
  }
  virtual MaybeEntries getEntries() override {
    return real->locked().getEntries();
  }
  virtual std::string getName(std::shared_ptr<File> file) override {
    return real->locked().getName(file);
  }

  friend std::shared_ptr<File> devirtualize(std::shared_ptr<File>);
};

class VirtualSymlink : public Symlink {
protected:
  std::shared_ptr<Symlink> real;

public:
  VirtualSymlink(std::shared_ptr<Symlink> real, backend_t backend)
    : Symlink(backend), real(real) {}

protected:
  virtual std::string getTarget() const override { return real->getTarget(); }

  friend std::shared_ptr<File> devirtualize(std::shared_ptr<File>);
};

// Unwrap a virtual file and get the underlying real file of the same kind.
inline std::shared_ptr<File> devirtualize(std::shared_ptr<File> file) {
  if (file->is<DataFile>()) {
    return std::static_pointer_cast<VirtualDataFile>(file)->real;
  } else if (file->is<Directory>()) {
    return std::static_pointer_cast<VirtualDirectory>(file)->real;
  } else if (file->is<Symlink>()) {
    return std::static_pointer_cast<VirtualDirectory>(file)->real;
  }
  WASMFS_UNREACHABLE("unexpected file kind");
}

} // namespace wasmfs
