// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the standard streams of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include <emscripten/html5.h>
#include <vector>
#include <wasi/api.h>

namespace wasmfs {

class StdinFile : public DataFile {

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  size_t getSize() override { return 0; }

public:
  StdinFile(mode_t mode) : DataFile(mode, NullBackend, S_IFCHR) {}
  static std::shared_ptr<StdinFile> getSingleton();
};

class StdoutFile : public DataFile {
  std::vector<char> writeBuffer;

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override;

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  // /dev/stdout reports a size of 0 in the terminal.
  size_t getSize() override { return 0; }

public:
  StdoutFile(mode_t mode) : DataFile(mode, NullBackend, S_IFCHR) {}
  static std::shared_ptr<StdoutFile> getSingleton();
};

class StderrFile : public DataFile {
  std::vector<char> writeBuffer;

  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is sufficient.
  // This would be a change from the current FS.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override;

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  // /dev/stderr reports a size of 0 in the terminal.
  size_t getSize() override { return 0; }

public:
  StderrFile(mode_t mode) : DataFile(mode, NullBackend, S_IFCHR) {}
  static std::shared_ptr<StderrFile> getSingleton();
};
} // namespace wasmfs
