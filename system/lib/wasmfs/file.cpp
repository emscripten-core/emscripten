// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"
#include "wasmfs.h"
#include <emscripten/threading.h>

extern "C" {
size_t _wasmfs_get_preloaded_file_size(uint32_t index);

size_t _wasmfs_copy_preloaded_file_data(uint32_t index, uint8_t* data);
}

namespace wasmfs {

//
// DataFile
//

void DataFile::Handle::preloadFromJS(int index) {
  // TODO: Each Datafile type could have its own impl of file preloading.
  // Create a buffer with the required file size.
  std::vector<uint8_t> buffer(_wasmfs_get_preloaded_file_size(index));

  // Ensure that files are preloaded from the main thread.
  assert(emscripten_is_main_runtime_thread());

  // Load data into the in-memory buffer.
  _wasmfs_copy_preloaded_file_data(index, buffer.data());

  write((const uint8_t*)buffer.data(), buffer.size(), 0);
}

//
// Directory
//

void Directory::Handle::cacheChild(const std::string& name,
                                   std::shared_ptr<File> child,
                                   DCacheKind kind) {
  // Update the dcache and set the child's parent.
  auto& dcache = getDir()->dcache;
  auto [_, inserted] = dcache.insert({name, {kind, child}});
  assert(inserted && "inserted child already existed!");
  assert(child->locked().getParent() == nullptr);
  child->locked().setParent(getDir());
}

std::shared_ptr<File> Directory::Handle::getChild(const std::string& name) {
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
  // Check whether the cache already contains this child.
  auto& dcache = getDir()->dcache;
  if (auto it = dcache.find(name); it != dcache.end()) {
    return it->second.file;
  }
  // Otherwise check whether the backend contains an underlying file we don't
  // know about.
  auto child = getDir()->getChild(name);
  if (!child) {
    return nullptr;
  }
  cacheChild(name, child, DCacheKind::Normal);
  return child;
}

bool Directory::Handle::mountChild(const std::string& name,
                                   std::shared_ptr<File> child) {
  assert(child);
  // Cannot insert into an unlinked directory.
  if (!getParent()) {
    return false;
  }
  cacheChild(name, child, DCacheKind::Mount);
  return true;
}

std::shared_ptr<DataFile>
Directory::Handle::insertDataFile(const std::string& name, mode_t mode) {
  // Cannot insert into an unlinked directory.
  if (!getParent()) {
    return nullptr;
  }
  auto child = getDir()->insertDataFile(name, mode);
  if (!child) {
    return nullptr;
  }
  cacheChild(name, child, DCacheKind::Normal);
  return child;
}

std::shared_ptr<Directory>
Directory::Handle::insertDirectory(const std::string& name, mode_t mode) {
  // Cannot insert into an unlinked directory.
  if (!getParent()) {
    return nullptr;
  }
  auto child = getDir()->insertDirectory(name, mode);
  if (!child) {
    return nullptr;
  }
  cacheChild(name, child, DCacheKind::Normal);
  return child;
}

std::shared_ptr<Symlink>
Directory::Handle::insertSymlink(const std::string& name,
                                 const std::string& target) {
  // Cannot insert into an unlinked directory.
  if (!getParent()) {
    return nullptr;
  }
  auto child = getDir()->insertSymlink(name, target);
  if (!child) {
    return nullptr;
  }
  cacheChild(name, child, DCacheKind::Normal);
  return child;
}

// TODO: consider moving this to be `Backend::move` to avoid asymmetry between
// the source and destination directories and/or taking `Directory::Handle`
// arguments to prove that the directories have already been locked.
bool Directory::Handle::insertMove(const std::string& name,
                                   std::shared_ptr<File> file) {
  // Cannot insert into an unlinked directory.
  if (!getParent()) {
    return false;
  }
  // Look up the file in its old parent's cache.
  auto& oldCache = file->locked().getParent()->dcache;
  auto oldIt = std::find_if(oldCache.begin(), oldCache.end(), [&](auto& kv) {
    return kv.second.file == file;
  });
  assert(oldIt != oldCache.end());
  auto [oldName, entry] = *oldIt;
  assert(oldName.size());
  // Attempt the move.
  if (!getDir()->insertMove(name, file)) {
    return false;
  }
  // Update parent pointers and caches to reflect the successful move.
  oldCache.erase(oldIt);
  auto& newCache = getDir()->dcache;
  auto [it, inserted] = newCache.insert({name, entry});
  if (!inserted) {
    // Update and overwrite the overwritten file.
    it->second.file->locked().setParent(nullptr);
    it->second = entry;
  }
  file->locked().setParent(getDir());
  return true;
}

bool Directory::Handle::removeChild(const std::string& name) {
  auto& dcache = getDir()->dcache;
  auto entry = dcache.find(name);
  // If this is a mount, we don't need to call into the backend.
  if (entry != dcache.end() && entry->second.kind == DCacheKind::Mount) {
    dcache.erase(entry);
    return true;
  }
  if (!getDir()->removeChild(name)) {
    return false;
  }
  if (entry != dcache.end()) {
    entry->second.file->locked().setParent(nullptr);
    dcache.erase(entry);
  }
  return true;
}

std::string Directory::Handle::getName(std::shared_ptr<File> file) {
  auto& dcache = getDir()->dcache;
  for (auto it = dcache.begin(); it != dcache.end(); ++it) {
    if (it->second.file == file) {
      return it->first;
    }
  }
  return "";
}

size_t Directory::Handle::getNumEntries() {
  size_t mounts = 0;
  auto& dcache = getDir()->dcache;
  for (auto it = dcache.begin(); it != dcache.end(); ++it) {
    if (it->second.kind == DCacheKind::Mount) {
      ++mounts;
    }
  }
  return getDir()->getNumEntries() + mounts;
}

std::vector<Directory::Entry> Directory::Handle::getEntries() {
  auto entries = getDir()->getEntries();
  auto& dcache = getDir()->dcache;
  for (auto it = dcache.begin(); it != dcache.end(); ++it) {
    auto& [name, entry] = *it;
    if (entry.kind == DCacheKind::Mount) {
      entries.push_back({name, entry.file->kind, entry.file->getIno()});
    }
  }
  return entries;
}

} // namespace wasmfs
