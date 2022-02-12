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
