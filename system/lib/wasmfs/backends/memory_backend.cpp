// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the memory file backend of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "memory_backend.h"
#include "wasmfs.h"

namespace wasmfs {

__wasi_errno_t MemoryFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len > buffer.size()) {
    buffer.resize(offset + len);
  }
  std::memcpy(&buffer[offset], buf, len);

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
  // The caller should have already checked that the offset + len does
  // not exceed the file's size.
  assert(offset + len <= buffer.size());
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
}

std::vector<MemoryDirectory::ChildEntry>::iterator
MemoryDirectory::findEntry(const std::string& name) {
  return std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
    return entry.name == name;
  });
}

std::shared_ptr<File> MemoryDirectory::getChild(const std::string& name) {
  if (auto entry = findEntry(name); entry != entries.end()) {
    return entry->child;
  }
  return nullptr;
}

bool MemoryDirectory::removeChild(const std::string& name) {
  auto entry = findEntry(name);
  if (entry == entries.end()) {
    return false;
  }
  entries.erase(entry);
  return true;
}

std::shared_ptr<File> MemoryDirectory::insertChild(const std::string& name,
                                                   std::shared_ptr<File> file) {
  if (auto entry = findEntry(name); entry != entries.end()) {
    return entry->child;
  }
  entries.push_back({name, file});
  return file;
}

std::string MemoryDirectory::getName(std::shared_ptr<File> file) {
  auto entry =
    std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
      return entry.child == file;
    });
  if (entry != entries.end()) {
    return entry->name;
  }
  return {};
}

std::vector<Directory::Entry> MemoryDirectory::getEntries() {
  std::vector<Directory::Entry> result;
  result.reserve(entries.size());
  for (auto& [name, child] : entries) {
    result.push_back({name, child->kind, child->getIno()});
  }
  return result;
}

class MemoryFileBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<MemoryFile>(mode, this);
  }
};

backend_t createMemoryFileBackend() {
  return wasmFS.addBackend(std::make_unique<MemoryFileBackend>());
}

} // namespace wasmfs
