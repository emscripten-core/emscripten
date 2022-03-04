// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string_view>

#include "paths.h"
#include "file.h"
#include "wasmfs.h"

namespace wasmfs::path {

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

static ParsedFile getChild(std::shared_ptr<Directory> dir,
                           std::string_view name) {
  auto child = dir->locked().getChild(std::string(name));
  if (!child) {
    return -ENOENT;
  }
  // TODO: Follow symlinks here.
  return child;
}

ParsedParent parseParent(std::string_view path,
                         std::optional<__wasi_fd_t> baseFD) {
  // Empty paths never exist.
  if (path.empty()) {
    return {-ENOENT};
  }

  // Initialize the starting directory.
  std::shared_ptr<Directory> curr;
  if (path.front() == '/') {
    curr = wasmFS.getRootDirectory();
    path.remove_prefix(1);
  } else if (baseFD && *baseFD != AT_FDCWD) {
    auto openFile = wasmFS.getFileTable().locked().getEntry(*baseFD);
    if (!openFile) {
      return -EBADF;
    }
    curr = openFile->locked().getFile()->dynCast<Directory>();
    if (!curr) {
      return {-ENOTDIR};
    }
  } else {
    curr = wasmFS.getCWD();
  }

  // Ignore trailing '/'.
  while (!path.empty() && path.back() == '/') {
    path.remove_suffix(1);
  }

  // An empty path here means that the path was equivalent to "/" and does not
  // contain a child segment for us to return. The root is its own parent, so we
  // can handle this by returning (root, ".").
  if (path.empty()) {
    return {std::make_pair(std::move(curr), std::string_view("."))};
  }

  while (true) {
    // Skip any leading '/' for each segment.
    while (!path.empty() && path.front() == '/') {
      path.remove_prefix(1);
    }

    // If this is the leaf segment, return.
    size_t segment_end = path.find_first_of('/');
    if (segment_end == std::string_view::npos) {
      return {std::make_pair(std::move(curr), path)};
    }

    // Try to descend into the child segment.
    // TODO: Check permissions on intermediate directories.
    auto segment = path.substr(0, segment_end);
    auto child = getChild(curr, segment);
    if (auto err = child.getError()) {
      return err;
    }
    curr = child.getFile()->dynCast<Directory>();
    if (!curr) {
      return -ENOTDIR;
    }
    path.remove_prefix(segment_end);
  }
}

ParsedFile parseFile(std::string_view path, std::optional<__wasi_fd_t> baseFD) {
  auto parsed = parseParent(path, baseFD);
  if (auto err = parsed.getError()) {
    return {err};
  }
  auto& [parent, child] = parsed.getParentChild();
  return getChild(parent, child);
}

} // namespace wasmfs::path
