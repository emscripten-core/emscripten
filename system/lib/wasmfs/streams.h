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
  void open(oflags_t) override {}
  void close() override {}

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  void flush() override {}

  size_t getSize() override { return 0; }

  void setSize(size_t size) override {
    // no-op
  }

public:
  StdinFile(mode_t mode) : DataFile(mode, NullBackend, S_IFCHR) {
    seekable = false;
  }

  static std::shared_ptr<StdinFile> getSingleton();
};

// A standard stream that writes: stdout or stderr.
class WritingStdFile : public DataFile {
protected:
  std::vector<char> writeBuffer;

  void open(oflags_t) override {}
  void close() override {}

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  // /dev/stdout|stderr reports a size of 0 in the terminal.
  size_t getSize() override { return 0; }

  void setSize(size_t size) override {
    // no-op
  }

public:
  WritingStdFile() : DataFile(S_IWUGO, NullBackend, S_IFCHR) {
    seekable = false;
  }
};

class StdoutFile : public WritingStdFile {
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override;
  void flush() override;

public:
  StdoutFile() {}

  static std::shared_ptr<StdoutFile> getSingleton();
};

class StderrFile : public WritingStdFile {
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override;
  void flush() override;

public:
  StderrFile() {}

  static std::shared_ptr<StderrFile> getSingleton();
};

} // namespace wasmfs
