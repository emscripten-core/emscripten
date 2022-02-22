// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "paths.h"
#include "file.h"
#include "wasmfs.h"

namespace wasmfs {

ParsedPath getParsedPath(std::vector<std::string> pathParts,
                         long& err,
                         std::shared_ptr<File> forbiddenAncestor,
                         std::optional<__wasi_fd_t> baseFD) {
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
    // This is a relative path. It is either relative to the current working
    // directory if no base FD is given, or if the base FD is the special value
    // indicating the CWD.
    if (baseFD && *baseFD != AT_FDCWD) {
      auto lockedOpenDir = wasmFS.getLockedFileTable()[*baseFD].locked();
      auto openDir = lockedOpenDir.getFile();
      if (!openDir->is<Directory>()) {
        err = -EBADF;
        return ParsedPath{{}, nullptr};
      }
      curr = openDir->dynCast<Directory>();
    } else {
      curr = wasmFS.getCWD();
    }
  }

  for (auto pathPart = begin; pathPart != pathParts.end() - 1; ++pathPart) {
    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    curr->locked().printKeys();
#endif
    auto child = curr->locked().getChild(*pathPart);

    if (!child) {
      err = -ENOENT;
      return ParsedPath{{}, nullptr};
    }

    if (child == forbiddenAncestor) {
      err = -EINVAL;
      return ParsedPath{{}, nullptr};
    }

    curr = child->dynCast<Directory>();

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
  auto child = lockedCurr.getChild(*(pathParts.end() - 1));
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
    curr = directory->locked().getChild(*it);

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
