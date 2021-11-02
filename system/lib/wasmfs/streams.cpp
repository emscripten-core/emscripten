// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the standard streams of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "streams.h"

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

std::shared_ptr<StdinFile> StdinFile::getSingleton() {
  static const std::shared_ptr<StdinFile> stdinFile =
    std::make_shared<StdinFile>(S_IRUGO);
  return stdinFile;
}

__wasi_errno_t StdoutFile::write(const uint8_t* buf, size_t len, off_t offset) {
  return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
}

std::shared_ptr<StdoutFile> StdoutFile::getSingleton() {
  static const std::shared_ptr<StdoutFile> stdoutFile =
    std::make_shared<StdoutFile>(S_IWUGO);
  return stdoutFile;
}

__wasi_errno_t StderrFile::write(const uint8_t* buf, size_t len, off_t offset) {
  return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
}

std::shared_ptr<StderrFile> StderrFile::getSingleton() {
  static const std::shared_ptr<StderrFile> stderrFile =
    std::make_shared<StderrFile>(S_IWUGO);
  return stderrFile;
}

} // namespace wasmfs
