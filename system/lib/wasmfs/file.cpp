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
// File
//

File::Handle File::locked() { return Handle(shared_from_this()); }
std::optional<File::Handle> File::maybeLocked() {
  auto handle = Handle(shared_from_this(), std::defer_lock);
  if (handle.trylock()) {
    return Handle(shared_from_this());
  } else {
    return {};
  }
}

//
// DataFile
//

DataFile::Handle DataFile::locked() { return Handle(shared_from_this()); }

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

Directory::Handle Directory::locked() { return Handle(shared_from_this()); }

std::optional<Directory::Handle> Directory::maybeLocked() {
  auto handle = Handle(shared_from_this(), std::defer_lock);
  if (handle.trylock()) {
    return Handle(shared_from_this());
  } else {
    return {};
  }
}

std::shared_ptr<File> Directory::Handle::removeEntry(const std::string& name) {
  auto entry = getEntry(name);
  if (entry == nullptr) {
    return nullptr;
  }
  // Atomically remove the entry and clear its parent.
  auto lockedEntry = entry->locked();
  if (!getDir()->removeEntry(name)) {
    return nullptr;
  }
  assert(lockedEntry.getParent() == getDir());
  lockedEntry.setParent(nullptr);
  return entry;
}

std::shared_ptr<File>
Directory::Handle::insertEntry(const std::string& name,
                               std::shared_ptr<File> file) {
  // Atomically add the entry and set its parent.
  auto lockedFile = file->locked();
  assert(lockedFile.getParent() == nullptr);
  auto entry = getDir()->insertEntry(name, file);
  if (file == entry) {
    // The insertion succeeded; set the parent.
    lockedFile.setParent(getDir());
  }
  return entry;
}

//
// Symlink
//

size_t Symlink::getSize() {
  return target.size();
}

} // namespace wasmfs
