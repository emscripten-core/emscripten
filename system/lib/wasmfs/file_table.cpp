// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"

namespace wasmfs {

std::vector<std::shared_ptr<OpenFileState>> FileTable::entries;

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
  return __WASI_ERRNO_SUCCESS;
}

class StdinFile : public File {

  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override {
    return __WASI_ERRNO_INVAL;
  }

  __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) override {
    return __WASI_ERRNO_INVAL;
  };
};

class StdoutFile : public File {
  std::vector<char> writeBuffer;

  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override {
    return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
  }

  __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) override {
    return __WASI_ERRNO_INVAL;
  };
};

class StderrFile : public File {
  std::vector<char> writeBuffer;

  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is sufficient.
  // This would be a change from the current FS.
  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override {
    return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
  }

  __wasi_errno_t read(const uint8_t* buf, __wasi_size_t len) override {
    return __WASI_ERRNO_INVAL;
  };
};

FileTable::FileTable() {
  entries.push_back(std::make_shared<OpenFileState>(0, std::make_shared<StdinFile>()));
  entries.push_back(std::make_shared<OpenFileState>(0, std::make_shared<StdoutFile>()));
  entries.push_back(std::make_shared<OpenFileState>(0, std::make_shared<StderrFile>()));
}

FileTable::Handle FileTable::get() {
  static FileTable fileTable;
  return FileTable::Handle(fileTable);
}

// Operator Overloading for FileTable::Handle::Entry
FileTable::Handle::Entry::operator std::shared_ptr<OpenFileState>() const {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return nullptr;
  }

  return fileTableHandle.fileTable.entries[fd];
}

FileTable::Handle::Entry& FileTable::Handle::Entry::operator=(std::shared_ptr<OpenFileState> ptr) {
  assert(fd >= 0);

  if (fd >= fileTableHandle.fileTable.entries.size()) {
    fileTableHandle.fileTable.entries.resize(fd + 1);
  }
  fileTableHandle.fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<OpenFileState>& FileTable::Handle::Entry::operator->() {
  assert(fd < fileTableHandle.fileTable.entries.size() && fd >= 0);

  return fileTableHandle.fileTable.entries[fd];
}

FileTable::Handle::Entry::operator bool() const {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return false;
  }

  return fileTableHandle.fileTable.entries[fd] != nullptr;
}
} // namespace wasmfs
