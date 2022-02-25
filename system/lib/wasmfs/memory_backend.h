// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the memory file class of the new file system.
// This should be the only backend file type defined in a header since it is the
// default type. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include <emscripten/threading.h>

namespace wasmfs {
// This class describes a file that lives in Wasm Memory.
class MemoryFile : public DataFile {
  std::vector<uint8_t> buffer;

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override;
  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override;
  void flush() override {}
  size_t getSize() override { return buffer.size(); }
  void setSize(size_t size) override { return buffer.resize(size); }

public:
  MemoryFile(mode_t mode, backend_t backend) : DataFile(mode, backend) {}

  class Handle : public DataFile::Handle {

    std::shared_ptr<MemoryFile> getFile() { return file->cast<MemoryFile>(); }

  public:
    Handle(std::shared_ptr<File> dataFile) : DataFile::Handle(dataFile) {}
  };

  Handle locked() { return Handle(shared_from_this()); }
};

class MemoryDirectory : public Directory {
  // Use a vector instead of a map to save code size.
  struct ChildEntry {
    std::string name;
    std::shared_ptr<File> child;
  };

  std::vector<ChildEntry> entries;

  std::vector<ChildEntry>::iterator findEntry(const std::string& name);

  std::shared_ptr<File> getChild(const std::string& name) override;
  bool removeChild(const std::string& name) override;
  std::shared_ptr<File> insertChild(const std::string& name,
                                    std::shared_ptr<File> file) override;
  std::string getName(std::shared_ptr<File> file) override;
  size_t getNumEntries() override { return entries.size(); }
  std::vector<Directory::Entry> getEntries() override;

public:
  MemoryDirectory(mode_t mode, backend_t backend) : Directory(mode, backend) {}
};

} // namespace wasmfs
