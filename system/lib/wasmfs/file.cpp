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

extern "C" {

static std::vector<char> fdWriteStdoutBuffer;
static std::vector<char> fdWriteStderrBuffer;

static __wasi_errno_t writeStdBuffer(const __wasi_ciovec_t* iovs, size_t iovs_len,
  __wasi_size_t* nwritten, void (*console_write)(const char*), std::vector<char>& fd_write_buffer) {
  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;
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
    num += len;
  }
  *nwritten = num;
  return 0;
}

__wasi_errno_t StdinFile::write(
  const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  return 0;
}

__wasi_errno_t StdoutFile::write(
  const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  return writeStdBuffer(iovs, iovs_len, nwritten, &emscripten_console_log, fdWriteStdoutBuffer);
}

// TODO: May not want to proxy stderr (fd == 2) to the main thread.
// This will not show in HTML - a console.warn in a worker is suffficient.
// This would be a change from the current FS.
__wasi_errno_t StderrFile::write(
  const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  return writeStdBuffer(iovs, iovs_len, nwritten, &emscripten_console_error, fdWriteStderrBuffer);
}
}
