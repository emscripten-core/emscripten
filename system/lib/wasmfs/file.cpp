// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the file object of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"
#include <emscripten/html5.h>
#include <vector>

namespace wasmfs {

static __wasi_errno_t writeStdBuffer(const uint8_t* buf, __wasi_size_t len,
  void (*console_write)(const char*), std::vector<char>& fd_write_buffer) {
  for (__wasi_size_t j = 0; j < len; j++) {
    uint8_t current = buf[j];
    if (current == '\0' || current == '\n') {
      fd_write_buffer.push_back('\0'); // for null-terminated C strings
      console_write(&fd_write_buffer[0]);
      fd_write_buffer.clear();
    } else {
      fd_write_buffer.push_back(current);
    }
  }
  return 0;
}

__wasi_errno_t StdinFile::write(const uint8_t* buf, __wasi_size_t len) { return 0; }

__wasi_errno_t StdoutFile::write(const uint8_t* buf, __wasi_size_t len) {
  return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
}

// TODO: May not want to proxy stderr (fd == 2) to the main thread.
// This will not show in HTML - a console.warn in a worker is sufficient.
// This would be a change from the current FS.
__wasi_errno_t StderrFile::write(const uint8_t* buf, __wasi_size_t len) {
  return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
}
} // namespace wasmfs
