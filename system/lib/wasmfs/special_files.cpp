// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the standard streams of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include <emscripten/html5.h>
#include <unistd.h>
#include <vector>
#include <wasi/api.h>

#include "special_files.h"
#include "wasmfs_internal.h"

namespace wasmfs::SpecialFiles {

namespace {

// No-op reads and writes: /dev/null
class NullFile : public DataFile {
  int open(oflags_t) override { return 0; }
  int close() override { return 0; }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return len;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override { return 0; }

  int flush() override { return 0; }
  off_t getSize() override { return 0; }
  int setSize(off_t size) override { return -EPERM; }

public:
  NullFile() : DataFile(S_IRUGO | S_IWUGO, NullBackend, S_IFCHR) {}
};

class StdinFile : public DataFile {
  int open(oflags_t) override { return 0; }
  int close() override { return 0; }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return -__WASI_ERRNO_INVAL;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    for (size_t i = 0; i < len; i++) {
      auto c = _wasmfs_stdin_get_char();
      if (c < 0) {
        // No more input can be read, return what we did read.
        return i;
      }
      buf[i] = c;
    }
    return len;
  };

  int flush() override { return 0; }
  off_t getSize() override { return 0; }
  int setSize(off_t size) override { return -EPERM; }

public:
  StdinFile() : DataFile(S_IRUGO, NullBackend, S_IFCHR) { seekable = false; }
};

// A standard stream that writes: stdout or stderr.
class WritingStdFile : public DataFile {
protected:
  std::vector<char> writeBuffer;

  int open(oflags_t) override { return 0; }
  int close() override { return 0; }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    return -__WASI_ERRNO_INVAL;
  };

  int flush() override {
    // Write a null to flush the output if we have content.
    if (!writeBuffer.empty()) {
      const uint8_t nothing = '\0';
      write(&nothing, 1, 0);
    }
    return 0;
  }

  off_t getSize() override { return 0; }
  int setSize(off_t size) override { return -EPERM; }

  ssize_t writeToJS(const uint8_t* buf,
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
    return len;
  }

public:
  WritingStdFile() : DataFile(S_IWUGO, NullBackend, S_IFCHR) {
    seekable = false;
  }
};

class StdoutFile : public WritingStdFile {
  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    // Node and worker threads issue in Emscripten:
    // https://github.com/emscripten-core/emscripten/issues/14804.
    // Issue filed in Node: https://github.com/nodejs/node/issues/40961
    // This is confirmed to occur when running with EXIT_RUNTIME and
    // PROXY_TO_PTHREAD. This results in only a single console.log statement
    // being outputted. The solution for now is to use out() and err() instead.
    return writeToJS(buf, len, &emscripten_out, writeBuffer);
  }

public:
  StdoutFile() {}
};

class StderrFile : public WritingStdFile {
  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    // Similar issue with Node and worker threads as emscripten_out.
    // TODO: May not want to proxy stderr (fd == 2) to the main thread, as
    //       emscripten_err does.
    //       This will not show in HTML - a console.warn in a worker is
    //       sufficient. This would be a change from the current FS.
    return writeToJS(buf, len, &emscripten_err, writeBuffer);
  }

public:
  StderrFile() {}
};

class RandomFile : public DataFile {
  int open(oflags_t) override { return 0; }
  int close() override { return 0; }

  ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return -__WASI_ERRNO_INVAL;
  }

  ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    uint8_t* end = buf + len;
    for (; buf < end; buf += 256) {
      [[maybe_unused]] int err = getentropy(buf, std::min(end - buf, 256l));
      assert(err == 0);
    }
    return len;
  };

  int flush() override { return 0; }
  off_t getSize() override { return 0; }
  int setSize(off_t size) override { return -EPERM; }

public:
  RandomFile() : DataFile(S_IRUGO, NullBackend, S_IFCHR) { seekable = false; }
};

} // anonymous namespace

std::shared_ptr<DataFile> getNull() {
  static auto null = std::make_shared<NullFile>();
  return null;
}

std::shared_ptr<DataFile> getStdin() {
  static auto stdin = std::make_shared<StdinFile>();
  return stdin;
}

std::shared_ptr<DataFile> getStdout() {
  static auto stdout = std::make_shared<StdoutFile>();
  return stdout;
}

std::shared_ptr<DataFile> getStderr() {
  static auto stderr = std::make_shared<StderrFile>();
  return stderr;
}

std::shared_ptr<DataFile> getRandom() {
  static auto random = std::make_shared<RandomFile>();
  return random;
}

std::shared_ptr<DataFile> getURandom() {
  static auto urandom = std::make_shared<RandomFile>();
  return urandom;
}

} // namespace wasmfs::SpecialFiles
