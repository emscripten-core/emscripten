// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#pragma once

#include <limits.h>
#include <mutex>
#include <queue>

#include "file.h"
#include "support.h"

namespace wasmfs {

// The data shared between the two sides of a pipe.
// TODO: Consider switching to a ring buffer for performance and code size if we
//       don't need unbounded pipes.
struct PipeData {
  // Each end is a separate PipeFile with its own lock, so the shared data needs
  // its own.
  std::mutex mutex;
  std::queue<uint8_t> bytes;
  bool readClosed = false;
  bool writeClosed = false;
};

// A PipeFile is a simple file that has a reference to a PipeData that it
// either reads from or writes to. A pair of PipeFiles comprise the two ends of
// a pipe.
class PipeFile : public DataFile {
  static constexpr off_t capacity = 1024 * 64; // as on Linux

  std::shared_ptr<PipeData> data;
  bool writeEnd;

  int open(oflags_t) override { return 0; }
  int close() override {
    std::lock_guard lock(data->mutex);
    (writeEnd ? data->writeClosed : data->readClosed) = true;
    return 0;
  }

  ssize_t write(const __wasi_ciovec_t* iovs,
                size_t iovs_len,
                off_t offset,
                oflags_t flags) override {
    std::lock_guard lock(data->mutex);
    if (data->readClosed) {
      return -EPIPE;
    }
    ssize_t total = 0;
    for (size_t i = 0; i < iovs_len; i++) {
      if (iovs[i].buf_len > std::numeric_limits<ssize_t>::max() ||
          addWillOverFlow(total, (ssize_t)iovs[i].buf_len)) {
        return -EINVAL;
      }
      total += iovs[i].buf_len;
    }
    if (total == 0) {
      return 0;
    }
    if (flags & O_NONBLOCK) {
      // PIPE_BUF rule: a small write is all-or-nothing, a larger one takes what
      // fits.
      off_t room = capacity - (off_t)data->bytes.size();
      if (room <= 0 || (total <= PIPE_BUF && total > room)) {
        return -EAGAIN;
      }
      total = std::min<ssize_t>(total, room);
    }
    size_t written = 0;
    for (size_t i = 0; i < iovs_len && written < (size_t)total; i++) {
      size_t len = std::min(iovs[i].buf_len, (size_t)total - written);
      for (size_t j = 0; j < len; j++) {
        data->bytes.push(iovs[i].buf[j]);
      }
      written += len;
    }
    return written;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    for (size_t i = 0; i < len; i++) {
      if (data->bytes.empty()) {
        return i;
      }
      buf[i] = data->bytes.front();
      data->bytes.pop();
    }
    return len;
  }

  ssize_t read(const __wasi_iovec_t* iovs,
               size_t iovs_len,
               off_t offset,
               oflags_t flags) override {
    std::lock_guard lock(data->mutex);
    auto nread = DataFile::read(iovs, iovs_len, offset, flags);
    if (nread == 0 && !data->writeClosed) {
      for (size_t i = 0; i < iovs_len; i++) {
        if (iovs[i].buf_len > 0) {
          return -EAGAIN;
        }
      }
    }
    return nread;
  }

  int flush() override { return 0; }

  off_t getSize() override {
    std::lock_guard lock(data->mutex);
    return data->bytes.size();
  }

  // TODO: Should this return an error?
  int setSize(off_t size) override { return 0; }

  short poll(oflags_t flags) override {
    std::lock_guard lock(data->mutex);
    if ((flags & O_ACCMODE) == O_WRONLY) {
      // Linux keeps the write end writable (the write itself fails with EPIPE)
      // while also signalling POLLERR once the read end is closed.
      if (data->readClosed) {
        return POLLOUT | POLLERR;
      }
      // Writable while a PIPE_BUF write fits, so a small write after POLLOUT
      // never fails.
      return capacity - (off_t)data->bytes.size() >= PIPE_BUF ? POLLOUT : 0;
    }
    short mask = data->bytes.empty() ? 0 : POLLIN;
    // With the write end closed the read end is at EOF: readable (read
    // returns 0) and hung up.
    if (data->writeClosed) {
      mask |= POLLIN | POLLHUP;
    }
    return mask;
  }

public:
  // PipeFiles do not have or need a backend. Pass NullBackend to the parent for
  // that.
  PipeFile(mode_t mode, std::shared_ptr<PipeData> data)
    : DataFile(mode, NullBackend, S_IFIFO), data(data),
      writeEnd(mode & S_IWUGO) {
    // Reads are always from the front; writes always to the end.
    seekable = false;
  }
};
} // namespace wasmfs
