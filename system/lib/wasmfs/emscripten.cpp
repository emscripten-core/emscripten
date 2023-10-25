// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file contains implementations of emscripten APIs that are compatible
// with WasmFS. These replace APIs in src/library*js, and basically do things
// in a simpler manner for the situation where the FS is in wasm and not JS
// (in particular, these implemenations avoid calling from JS to wasm, and
// dependency issues that arise from that).
//

#include <emscripten.h>
#include <stdio.h>

#include <string>

#include "file.h"
#include "file_table.h"
#include "wasmfs.h"

namespace wasmfs {

// Given an fd, returns the string path that the fd refers to.
// TODO: full error handling
// TODO: maybe make this a public API, as it is useful for debugging
std::string getPath(int fd) {
  auto fileTable = wasmfs::wasmFS.getFileTable().locked();

  auto openFile = fileTable.getEntry(fd);
  if (!openFile) {
    return "!";
  }

  auto curr = openFile->locked().getFile();
  std::string result = "";
  while (curr != wasmfs::wasmFS.getRootDirectory()) {
    auto parent = curr->locked().getParent();
    // Check if the parent exists. The parent may not exist if curr was
    // unlinked.
    if (!parent) {
      return "?/" + result;
    }

    auto parentDir = parent->dynCast<wasmfs::Directory>();
    auto name = parentDir->locked().getName(curr);
    result = '/' + name + result;
    curr = parentDir;
  }
  return result;
}

} // namespace wasmfs

extern "C" {

char* emscripten_get_preloaded_image_data_from_FILE(FILE* file,
                                                    int* w,
                                                    int* h) {
  auto fd = fileno(file);
  if (fd < 0) {
    return 0;
  }

  auto path = wasmfs::getPath(fd);
  return emscripten_get_preloaded_image_data(path.c_str(), w, h);
}

} // extern "C"
