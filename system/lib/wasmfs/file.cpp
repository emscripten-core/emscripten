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

//
// Path Parsing utilities
//

ParsedPath getParsedPath(std::vector<std::string> pathParts,
                         long& err,
                         std::shared_ptr<File> forbiddenAncestor) {
  std::shared_ptr<Directory> curr;
  auto begin = pathParts.begin();

  if (pathParts.empty()) {
    err = -ENOENT;
    return ParsedPath{{}, nullptr};
  }

  // Check if the first path element is '/', indicating an absolute path.
  if (pathParts[0] == "/") {
    curr = wasmFS.getRootDirectory();
    begin++;
    // If the pathname is the root directory, return the root as the child.
    if (pathParts.size() == 1) {
      return ParsedPath{curr->locked(), curr};
    }
  } else {
    curr = wasmFS.getCWD();
  }

  for (auto pathPart = begin; pathPart != pathParts.end() - 1; ++pathPart) {
    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    curr->locked().printKeys();
#endif
    auto entry = curr->locked().getEntry(*pathPart);

    if (forbiddenAncestor) {
      if (entry == forbiddenAncestor) {
        err = -EINVAL;
        return ParsedPath{{}, nullptr};
      }
    }

    // An entry is defined in the current directory's entries vector.
    if (!entry) {
      err = -ENOENT;
      return ParsedPath{{}, nullptr};
    }

    curr = entry->dynCast<Directory>();

    // If file is nullptr, then the file was not a Directory.
    // TODO: Change this to accommodate symlinks
    if (!curr) {
      err = -ENOTDIR;
      return ParsedPath{{}, nullptr};
    }

#ifdef WASMFS_DEBUG
    emscripten_console_log(*pathPart->c_str());
#endif
  }

  // Lock the parent once.
  auto lockedCurr = curr->locked();
  auto child = lockedCurr.getEntry(*(pathParts.end() - 1));
  return ParsedPath{std::move(lockedCurr), child};
}

std::shared_ptr<Directory> getDir(std::vector<std::string>::iterator begin,
                                  std::vector<std::string>::iterator end,
                                  long& err,
                                  std::shared_ptr<File> forbiddenAncestor) {

  std::shared_ptr<File> curr;
  // Check if the first path element is '/', indicating an absolute path.
  if (*begin == "/") {
    curr = wasmFS.getRootDirectory();
    begin++;
  } else {
    curr = wasmFS.getCWD();
  }

  for (auto it = begin; it != end; ++it) {
    auto directory = curr->dynCast<Directory>();

    // If file is nullptr, then the file was not a Directory.
    // TODO: Change this to accommodate symlinks
    if (!directory) {
      err = -ENOTDIR;
      return nullptr;
    }

    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    directory->locked().printKeys();
#endif
    curr = directory->locked().getEntry(*it);

    if (forbiddenAncestor) {
      if (curr == forbiddenAncestor) {
        err = -EINVAL;
        return nullptr;
      }
    }

    // Requested entry (file or directory)
    if (!curr) {
      err = -ENOENT;
      return nullptr;
    }

#ifdef WASMFS_DEBUG
    emscripten_console_log(it->c_str());
#endif
  }

  auto currDirectory = curr->dynCast<Directory>();

  if (!currDirectory) {
    err = -ENOTDIR;
    return nullptr;
  }

  return currDirectory;
}

// TODO: Check for trailing slash, i.e. /foo/bar.txt/
// Currently any trailing slash is ignored.
std::vector<std::string> splitPath(char* pathname) {
  std::vector<std::string> pathParts;
  char newPathName[strlen(pathname) + 1];
  strcpy(newPathName, pathname);

  // TODO: Other path parsing edge cases.
  char* current;
  // Handle absolute path.
  if (newPathName[0] == '/') {
    pathParts.push_back("/");
  }

  current = strtok(newPathName, "/");
  while (current != NULL) {
    pathParts.push_back(current);
    current = strtok(NULL, "/");
  }

  return pathParts;
}

} // namespace wasmfs
