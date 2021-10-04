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
#include <wasi/api.h>

class File {
  // TODO: Add other File properties later.

  // Mutex is used to lock File during multi-threaded access.
  std::mutex mutex;

public:
  virtual __wasi_errno_t read(
    const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) = 0;
  virtual __wasi_errno_t write(
    const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) = 0;

  virtual ~File() = default;
};

class StdinFile : public File {
public:
  // TODO: fill in write for stdin
  __wasi_errno_t write(
    const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) override;

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StdinFile::read() has not been implemented yet.");
    abort();
  };
};

class StdoutFile : public File {
public:
  __wasi_errno_t write(
    const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) override;

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StdoutFile::read() has not been implemented yet.");
    abort();
  };
};

class StderrFile : public File {
public:
  __wasi_errno_t write(
    const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) override;

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StderrFile::read() has not been implemented yet.");
    abort();
  };
};
