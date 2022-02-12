// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the memory file class of the new file system.
// This should be the only backend file type defined in a header since it is the
// default type. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "memory_file.h"

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

std::vector<Directory::Entry>::iterator
MemoryDirectory::findEntry(const std::string& name) {
  return std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
    return entry.name == name;
  });
}

std::shared_ptr<File> MemoryDirectory::getEntry(const std::string& name) {
  if (auto entry = findEntry(name); entry != entries.end()) {
    return entry->file;
  }
  return nullptr;
}

bool MemoryDirectory::removeEntry(const std::string& name) {
  auto entry = findEntry(name);
  if (entry == entries.end()) {
    return false;
  }
  entries.erase(entry);
  return true;
}

std::shared_ptr<File> MemoryDirectory::insertEntry(const std::string& name,
                                                   std::shared_ptr<File> file) {
  if (auto entry = findEntry(name); entry != entries.end()) {
    return entry->file;
  }
  entries.push_back({name, file});
  return file;
}

std::string MemoryDirectory::getName(std::shared_ptr<File> file) {
  auto entry =
    std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
      return entry.file == file;
    });
  if (entry != entries.end()) {
    return entry->name;
  }
  return {};
}

} // namespace wasmfs
