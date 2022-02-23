// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "file.h"

namespace wasmfs {

struct ParsedPath {
  std::optional<Directory::Handle> parent;
  std::shared_ptr<File> child;
};

// Call getParsedPath if one needs a locked handle to a parent dir and a
// shared_ptr to its child file, given a file path.
// TODO: When locking the directory structure is refactored, parent should be
// returned as a pointer, similar to child.
// Will return an empty handle if the parent is not a directory.
// Will error if the forbiddenAncestor is encountered while processing.
// If the forbiddenAncestor is encountered, err will be set to EINVAL and
// an empty parent handle will be returned.
ParsedPath getParsedPath(std::vector<std::string> pathParts,
                         long& err,
                         std::shared_ptr<File> forbiddenAncestor = nullptr,
                         std::optional<__wasi_fd_t> baseFD = {});

// Call getDir if one needs a parent directory of a file path.
// TODO: Remove this when directory structure locking is refactored and use
// getParsedPath instead. Obtains parent directory of a given pathname.
// Will return a nullptr if the parent is not a directory. Will error if the
// forbiddenAncestor is encountered while processing. If the forbiddenAncestor
// is encountered, err will be set to EINVAL and nullptr will be returned.
std::shared_ptr<Directory>
getDir(std::vector<std::string>::iterator begin,
       std::vector<std::string>::iterator end,
       long& err,
       std::shared_ptr<File> forbiddenAncestor = nullptr);

// Return a vector of the '/'-delimited components of a path. The first
// element will be "/" iff the path is an absolute path.
std::vector<std::string> splitPath(char* pathname);

} // namespace wasmfs
