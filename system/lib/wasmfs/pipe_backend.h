// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include <queue>

#include "file.h"
#include "support.h"

namespace wasmfs {

// The data shared between the two sides of a pipe.
// TODO: Consider switching to a ring buffer for performance and code size if we
//       don't need unbounded pipes.
using PipeData = std::queue<uint8_t>;

// A PipeFile is a simple file that has a reference to a PipeData that it
// either reads from or writes to. A pair of PipeFiles comprise the two ends of
// a pipe.
class PipeFile : public DataFile {
  std::shared_ptr<PipeData> data;

  int open(oflags_t) override { return 0; }
  int close() override { return 0; }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    for (size_t i = 0; i < len; i++) {
      data->push(buf[i]);
    }
    return len;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    for (size_t i = 0; i < len; i++) {
      if (data->empty()) {
        return i;
      }
      buf[i] = data->front();
      data->pop();
    }
    return len;
  }

  int flush() override { return 0; }

  off_t getSize() override { return data->size(); }

  // TODO: Should this return an error?
  int setSize(off_t size) override { return 0; }

public:
  // PipeFiles do not have or need a backend. Pass NullBackend to the parent for
  // that.
  PipeFile(mode_t mode, std::shared_ptr<PipeData> data)
    : DataFile(mode, NullBackend), data(data) {
    // Reads are always from the front; writes always to the end.
    seekable = false;
  }
};
} // namespace wasmfs
