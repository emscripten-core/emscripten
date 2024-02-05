// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the memory file backend of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "memory_backend.h"
#include "backend.h"
#include "wasmfs.h"

namespace wasmfs {

ssize_t MemoryDataFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len > buffer.size()) {
    if (offset + len > buffer.max_size()) {
      // Overflow: the necessary size fits in an off_t, but cannot fit in the
      // container.
      return -EIO;
    }
    buffer.resize(offset + len);
  }
  std::memcpy(&buffer[offset], buf, len);
  return len;
}

ssize_t MemoryDataFile::read(uint8_t* buf, size_t len, off_t offset) {
  if (offset >= buffer.size()) {
    len = 0;
  } else if (offset + len >= buffer.size()) {
    len = buffer.size() - offset;
  }
  std::memcpy(buf, &buffer[offset], len);
  return len;
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

int MemoryDirectory::removeChild(const std::string& name) {
  auto entry = findEntry(name);
  if (entry != entries.end()) {
    entry->child->locked().setParent(nullptr);
    entries.erase(entry);
  }
  return 0;
}

Directory::MaybeEntries MemoryDirectory::getEntries() {
  std::vector<Directory::Entry> result;
  result.reserve(entries.size());
  for (auto& [name, child] : entries) {
    result.push_back({name, child->kind, child->getIno()});
  }
  return {result};
}

int MemoryDirectory::insertMove(const std::string& name,
                                std::shared_ptr<File> file) {
  auto& oldEntries =
    std::static_pointer_cast<MemoryDirectory>(file->locked().getParent())
      ->entries;
  for (auto it = oldEntries.begin(); it != oldEntries.end(); ++it) {
    if (it->child == file) {
      oldEntries.erase(it);
      break;
    }
  }
  (void)removeChild(name);
  insertChild(name, file);
  return 0;
}

std::string MemoryDirectory::getName(std::shared_ptr<File> file) {
  auto it =
    std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
      return entry.child == file;
    });
  if (it != entries.end()) {
    return it->name;
  }
  return "";
}

class MemoryBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<MemoryDataFile>(mode, this);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<MemoryDirectory>(mode, this);
  }
  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return std::make_shared<MemorySymlink>(target, this);
  }
};

backend_t createMemoryBackend() {
  return wasmFS.addBackend(std::make_unique<MemoryBackend>());
}

extern "C" {

backend_t wasmfs_create_memory_backend() { return createMemoryBackend(); }

} // extern "C"

} // namespace wasmfs
