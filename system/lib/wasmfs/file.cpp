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

std::shared_ptr<File> Directory::Handle::getEntry(std::string pathName) {
  auto found =
    std::find_if(getDir()->entries.begin(),
                 getDir()->entries.end(),
                 [&](const auto& entry) { return entry.name == pathName; });

  if (found != getDir()->entries.end()) {
    return found->file;
  }

  return nullptr;
}

void Directory::Handle::setEntry(std::string pathName,
                                 std::shared_ptr<File> inserted) {
  // Hold the lock over both functions to cover the case in which two
  // directories attempt to add the file.
  auto lockedInserted = inserted->locked();
  // Simultaneously, set the parent of the inserted node to be this Dir.
  // inserted must be locked because we have to go through Handle.
  // TODO: When rename is implemented, ensure that the source directory has
  // been removed as a parent.
  // https://github.com/emscripten-core/emscripten/pull/15410#discussion_r742171264
  assert(!lockedInserted.getParent());
  lockedInserted.setParent(file);

  // During testing, this will check that an existing file associated with
  // pathName does not exist. For rename, the existing file must be unlinked
  // first.
  assert(!getEntry(pathName));
  getDir()->entries.push_back({pathName, inserted});
}

void Directory::Handle::unlinkEntry(std::string pathName) {
  // The file lock must be held for both operations. Removing the child file
  // from the parent's entries and removing the parent pointer from the
  // child should be atomic. The state should not be mutated in between.
  auto unlinked = getEntry(pathName)->locked();
  unlinked.setParent({});

  getDir()->entries.erase(
    std::remove_if(getDir()->entries.begin(),
                   getDir()->entries.end(),
                   [&](const auto& entry) { return entry.name == pathName; }),
    getDir()->entries.end());
}

std::string Directory::Handle::getName(std::shared_ptr<File> target) {
  auto found =
    std::find_if(getDir()->entries.begin(),
                 getDir()->entries.end(),
                 [&](const auto& entry) { return entry.file == target; });

  if (found != getDir()->entries.end()) {
    return found->name;
  }

  return "";
}

//
// Symlink
//

size_t Symlink::getSize() {
  return target.size();
}

} // namespace wasmfs
