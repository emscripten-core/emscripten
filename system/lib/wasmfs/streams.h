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

static __wasi_errno_t writeStdBuffer(const uint8_t* buf,
                                     size_t len,
                                     void (*console_write)(const char*),
                                     std::vector<char>& fd_write_buffer) {
  for (size_t j = 0; j < len; j++) {
    uint8_t current = buf[j];
    if (current == '\0' || current == '\n') {
      fd_write_buffer.push_back('\0'); // for null-terminated C strings
      console_write(fd_write_buffer.data());
      fd_write_buffer.clear();
    } else {
      fd_write_buffer.push_back(current);
    }
  }
  return __WASI_ERRNO_SUCCESS;
}

class StdinFile : public DataFile {

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  size_t getSize() override { return 0; }

public:
  StdinFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StdinFile> getSingleton() {
    static const std::shared_ptr<StdinFile> stdinFile =
      std::make_shared<StdinFile>(S_IRUGO);
    return stdinFile;
  }
};

class StdoutFile : public DataFile {
  std::vector<char> writeBuffer;

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  // /dev/stdout reports a size of 0 in the terminal.
  size_t getSize() override { return 0; }

public:
  StdoutFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StdoutFile> getSingleton() {
    static const std::shared_ptr<StdoutFile> stdoutFile =
      std::make_shared<StdoutFile>(S_IWUGO);
    return stdoutFile;
  }
};

class StderrFile : public DataFile {
  std::vector<char> writeBuffer;

  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is sufficient.
  // This would be a change from the current FS.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

  // /dev/stderr reports a size of 0 in the terminal.
  size_t getSize() override { return 0; }

public:
  StderrFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StderrFile> getSingleton() {
    static const std::shared_ptr<StderrFile> stderrFile =
      std::make_shared<StderrFile>(S_IWUGO);
    return stderrFile;
  }
};
} // namespace wasmfs
