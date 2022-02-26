// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "paths.h"
#include "file.h"
#include "wasmfs.h"

namespace wasmfs {

#ifdef WASMFS_DEBUG
// Print the absolute path of a file.
std::string getAbsPath(std::shared_ptr<File> curr) {
  std::string result = "";

  while (curr != wasmFS.getRootDirectory()) {
    auto parent = curr->locked().getParent();
    // Check if the parent exists. The parent may not exist if the CWD or one
    // of its ancestors has been unlinked.
    if (!parent) {
      return "unlinked";
    }

    auto parentDir = parent->dynCast<Directory>();

    auto name = parentDir->locked().getName(curr);
    result = '/' + name + result;
    curr = parentDir;
  }

  // Check if the cwd is the root directory.
  if (result.empty()) {
    result = "/";
  }

  return result;
}
#endif

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
      auto lockedOpenDir =
        wasmFS.getFileTable().locked().getEntry(*baseFD)->locked();
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

  for (auto pathPart = begin; pathPart != pathParts.end(); ++pathPart) {
    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    curr->locked().printKeys();
#endif

    auto lockedCurr = curr->locked();

    // Find the relevant child for this path part.
    std::shared_ptr<File> child;
    std::shared_ptr<File> parent;
    if (*pathPart == ".") {
      child = curr;
      parent = lockedCurr.getParent();
    } else if (*pathPart == "..") {
      if (curr == wasmFS.getRootDirectory()) {
        child = curr;
      } else {
        child = lockedCurr.getParent()->cast<Directory>();
      }
      parent = child->locked().getParent();
    } else {
      child = lockedCurr.getChild(*pathPart);
      parent = curr;
    }

    bool atFinalPart = pathPart == (pathParts.end() - 1);
    if (atFinalPart) {
      // We found the child to return, now compute the parent. Normally that is
      // curr, but we must also handle the other cases of . and .. as well as
      // the possible corner case of the child being moved or unlinked
      // meanwhile (which can happen with . and .., as then we are not holding a
      // lock on the parent).
      if (!parent) {
        return ParsedPath{{}, child};
      }
      if (parent == curr) {
        // We already have a lock on curr; use that.
        return ParsedPath{std::move(lockedCurr), child};
      }
      // Take a new lock as this is something other than curr. However, we must
      // free our lock on curr first, to avoid a potential deadlock.
      {
        auto releaser = std::move(lockedCurr);
      }
      return ParsedPath{parent->cast<Directory>()->locked(), child};
    }

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

  // There was no path to process in the loop.
  return ParsedPath{std::move(curr->locked()), nullptr};
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
