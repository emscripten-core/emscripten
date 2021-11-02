// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the global state of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include "file_table.h"
#include <assert.h>
#include <emscripten/html5.h>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

class WasmFS {

  FileTable fileTable;
  std::shared_ptr<Directory> rootDirectory;

  // Private method to initialize root directory once.
  // Initializes default directories including dev/stdin, dev/stdout,
  // dev/stderr. Refers to the same std streams in the open file table.
  std::shared_ptr<Directory> initRootDirectory();

public:
  // Files will be preloaded in this constructor.
  // This global constructor has init_priority 100. Please see wasmfs.cpp.
  WasmFS() : rootDirectory(initRootDirectory()) {}

  // This get method returns a locked file table.
  // There is only ever one FileTable in the system.
  FileTable::Handle getLockedFileTable() {
    return FileTable::Handle(fileTable);
  }

  // Returns root directory defined on WasmFS singleton.
  std::shared_ptr<Directory> getRootDirectory() { return rootDirectory; };
};

// Global state instance.
extern WasmFS wasmFS;

} // namespace wasmfs
