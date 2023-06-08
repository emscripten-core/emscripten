// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <string>

#include "file.h"
#include "file_table.h"
#include "wasmfs.h"

// Given an fd, returns the string path that the fd refers to.
// TODO: full error handling
static std::string getPath(int fd) {
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

extern "C"
char *emscripten_get_preloaded_image_data_from_FILE(FILE *file,
                                                    int *w,
                                                    int *h) {
  auto fd = fileno(file);
  if (fd < 0) {
    return 0;
  }

  auto path = getPath(fd);
  return emscripten_get_preloaded_image_data(path.c_str(), w, h);
}
