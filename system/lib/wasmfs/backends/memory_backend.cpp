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

ssize_t MemoryFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len > buffer.size()) {
    buffer.resize(offset + len);
  }
  std::memcpy(&buffer[offset], buf, len);
  return len;
}

ssize_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
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

bool MemoryDirectory::removeChild(const std::string& name) {
  auto entry = findEntry(name);
  if (entry != entries.end()) {
    entries.erase(entry);
  }
  return true;
}

std::vector<Directory::Entry> MemoryDirectory::getEntries() {
  std::vector<Directory::Entry> result;
  result.reserve(entries.size());
  for (auto& [name, child] : entries) {
    result.push_back({name, child->kind, child->getIno()});
  }
  return result;
}

bool MemoryDirectory::insertMove(const std::string& name,
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
  removeChild(name);
  insertChild(name, file);
  return true;
}

class MemoryFileBackend : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<MemoryFile>(mode, this);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<MemoryDirectory>(mode, this);
  }
  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return std::make_shared<MemorySymlink>(target, this);
  }
};

backend_t createMemoryFileBackend() {
  return wasmFS.addBackend(std::make_unique<MemoryFileBackend>());
}

} // namespace wasmfs
