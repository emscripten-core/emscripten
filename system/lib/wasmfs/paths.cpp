// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string_view>

#include "paths.h"
#include "file.h"
#include "wasmfs.h"

namespace wasmfs::path {

namespace {

static inline constexpr size_t MAX_RECURSIONS = 40;

ParsedFile doParseFile(std::string_view path,
                       std::shared_ptr<Directory> base,
                       LinkBehavior links,
                       size_t& recursions,
                       bool topLevel = false);

ParsedFile getBase(__wasi_fd_t basefd) {
  if (basefd == AT_FDCWD) {
    return {wasmFS.getCWD()};
  }
  auto openFile = wasmFS.getFileTable().locked().getEntry(basefd);
  if (!openFile) {
    return -EBADF;
  }
  if (auto baseDir = openFile->locked().getFile()->dynCast<Directory>()) {
    return {baseDir};
  }
  return -ENOTDIR;
}

ParsedFile getChild(std::shared_ptr<Directory> dir,
                    std::string_view name,
                    LinkBehavior links,
                    size_t& recursions) {
  auto child = dir->locked().getChild(std::string(name));
  if (!child) {
    return -ENOENT;
  }
  if (links != NoFollowLinks) {
    while (auto link = child->dynCast<Symlink>()) {
      if (++recursions > MAX_RECURSIONS) {
        return -ELOOP;
      }
      auto target = link->getTarget();
      if (target.empty()) {
        return -ENOENT;
      }
      auto parsed = doParseFile(target, dir, links, recursions);
      if (auto err = parsed.getError()) {
        return err;
      }
      child = parsed.getFile();
    }
  }
  return child;
}

ParsedParent doParseParent(std::string_view path,
                           std::shared_ptr<Directory> curr,
                           LinkBehavior links,
                           size_t& recursions) {
  // Empty paths never exist.
  if (path.empty()) {
    return {-ENOENT};
  }

  // Handle absolute paths.
  if (path.front() == '/') {
    curr = wasmFS.getRootDirectory();
    path.remove_prefix(1);
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
    auto child = getChild(curr, segment, links, recursions);
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

ParsedFile doParseFile(std::string_view path,
                       std::shared_ptr<Directory> base,
                       LinkBehavior links,
                       size_t& recursions,
                       bool topLevel) {
  auto parsed = doParseParent(path, base, links, recursions);
  if (auto err = parsed.getError()) {
    return {err};
  }
  auto& [parent, child] = parsed.getParentChild();
  if (topLevel && links == FollowParentLinks) {
    links = NoFollowLinks;
  }
  return getChild(parent, child, links, recursions);
}

} // anonymous namespace

ParsedParent
parseParent(std::string_view path, __wasi_fd_t basefd, LinkBehavior links) {
  auto base = getBase(basefd);
  if (auto err = base.getError()) {
    return err;
  }
  size_t recursions = 0;
  auto baseDir = base.getFile()->cast<Directory>();
  return doParseParent(path, baseDir, links, recursions);
}

ParsedFile
parseFile(std::string_view path, __wasi_fd_t basefd, LinkBehavior links) {
  auto base = getBase(basefd);
  if (auto err = base.getError()) {
    return err;
  }
  size_t recursions = 0;
  auto baseDir = base.getFile()->cast<Directory>();
  return doParseFile(path, baseDir, links, recursions, true);
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
  auto links = (flags & AT_SYMLINK_NOFOLLOW) ? FollowParentLinks : FollowLinks;
  return path::parseFile(path, fd, links);
}

} // namespace wasmfs::path
