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
    // Flush on either a null or a newline.
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
  // Node and worker threads issue in Emscripten:
  // https://github.com/emscripten-core/emscripten/issues/14804.
  // Issue filed in Node: https://github.com/nodejs/node/issues/40961
  // This is confirmed to occur when running with EXIT_RUNTIME and
  // PROXY_TO_PTHREAD. This results in only a single console.log statement being
  // outputted. The solution for now is to use out() and err() instead.
  return writeStdBuffer(buf, len, &_emscripten_out, writeBuffer);
}

void StdoutFile::flush() {
  // Write a null to flush the output (see comment above on "Flush on either a
  // null or a newline").
  const uint8_t nothing = '\0';
  write(&nothing, 1, 0);
}

std::shared_ptr<StdoutFile> StdoutFile::getSingleton() {
  static const std::shared_ptr<StdoutFile> stdoutFile =
    std::make_shared<StdoutFile>();
  return stdoutFile;
}

__wasi_errno_t StderrFile::write(const uint8_t* buf, size_t len, off_t offset) {
  // Similar issue with Node and worker threads as emscripten_out.
  // TODO: May not want to proxy stderr (fd == 2) to the main thread, as
  //       emscripten_err does.
  //       This will not show in HTML - a console.warn in a worker is
  //       sufficient. This would be a change from the current FS.
  return writeStdBuffer(buf, len, &_emscripten_err, writeBuffer);
}

void StderrFile::flush() {
  // Similar to StdoutFile.
  const uint8_t nothing = '\0';
  write(&nothing, 1, 0);
}

std::shared_ptr<StderrFile> StderrFile::getSingleton() {
  static const std::shared_ptr<StderrFile> stderrFile =
    std::make_shared<StderrFile>();
  return stderrFile;
}

} // namespace wasmfs
