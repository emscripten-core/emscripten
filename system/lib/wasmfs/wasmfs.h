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
  // Files will be preloaded in this constructor.
  WasmFS() {}

  // Since the constructor is private, the only way to access the global file
  // state is through this get method.
  static WasmFS get() {
    static WasmFS globalState;
    return globalState;
  }

public:
  // Initialize default directories including dev/stdin, dev/stdout, dev/stderr.
  // Refers to same std streams in the open file table.
  std::shared_ptr<Directory> getRootDir();

  // This get method is responsible for lazily initializing the FileTable.
  // There is only ever one FileTable in the system.
  FileTable::Handle getFileTable() {
    static FileTable fileTable;
    return FileTable::Handle(fileTable);
  }

  // Returns lazily initialized file table defined on WasmFS singleton.
  static FileTable::Handle getLockedFileTable() { return get().getFileTable(); }

  // Returns root directory defined on WasmFS singleton.
  static std::shared_ptr<Directory> getRootDirectory() {
    return get().getRootDir();
  }
};

} // namespace wasmfs
