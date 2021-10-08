// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include <emscripten/html5.h>
#include <mutex>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

class File {
  // TODO: Add other File properties later.

  // A mutex is needed for multiple accesses to the same file.
  std::mutex mutex;

  virtual __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) = 0;
  virtual __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) = 0;

public:
  virtual ~File() = default;
  class Handle {
    File& file;
    std::unique_lock<std::mutex> lock;

  public:
    Handle(File& file) : file(file), lock(file.mutex) {}
    Handle(Handle&&) = default;

    __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) { return file.read(buf, len); }
    __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) { return file.write(buf, len); }
  };

  Handle get() { return Handle(*this); }
};

} // namespace wasmfs
