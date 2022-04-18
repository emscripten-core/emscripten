// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string_view>

#include "paths.h"
#include "file.h"
#include "wasmfs.h"

namespace wasmfs::path {

static ParsedFile getChild(std::shared_ptr<Directory> dir,
                           std::string_view name) {
  auto child = dir->locked().getChild(std::string(name));
  if (!child) {
    return -ENOENT;
  }
  // TODO: Follow symlinks here.
  return child;
}

ParsedParent parseParent(std::string_view path, __wasi_fd_t basefd) {
  // Empty paths never exist.
  if (path.empty()) {
    return {-ENOENT};
  }

  // Initialize the starting directory.
  std::shared_ptr<Directory> curr;
  if (path.front() == '/') {
    curr = wasmFS.getRootDirectory();
    path.remove_prefix(1);
  } else if (basefd == AT_FDCWD) {
    curr = wasmFS.getCWD();
  } else {
    auto openFile = wasmFS.getFileTable().locked().getEntry(basefd);
    if (!openFile) {
      return -EBADF;
    }
    curr = openFile->locked().getFile()->dynCast<Directory>();
    if (!curr) {
      return {-ENOTDIR};
    }
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

ParsedFile parseFile(std::string_view path, __wasi_fd_t basefd) {
  auto parsed = parseParent(path, basefd);
  if (auto err = parsed.getError()) {
    return {err};
  }
  auto& [parent, child] = parsed.getParentChild();
  return getChild(parent, child);
}

ParsedFile getFileAt(__wasi_fd_t fd, std::string_view path, int flags) {
  if ((flags & AT_EMPTY_PATH) && path.size() == 0) {
    // Don't parse a path, just use `dirfd` directly.
    if (fd == AT_FDCWD) {
      return {wasmFS.getCWD()};
    }
    auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
    if (!openFile) {
      return {-EBADF};
    }
    return {openFile->locked().getFile()};
  }
  return path::parseFile(path, fd);
}

} // namespace wasmfs::path
