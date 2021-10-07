// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the open file table of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file_table.h"

namespace wasmfs {

std::vector<std::shared_ptr<OpenFileInfo>> FileTable::entries;

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

class StdinFile : public File {

  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override { return 0; }

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StdinFile::read() has not been implemented yet.");
    abort();
  };

public:
  File::Handle get() { return Handle(*this); }
};

class StdoutFile : public File {
  static std::vector<char> writeBuffer;

  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override {
    return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
  }

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StdoutFile::read() has not been implemented yet.");
    abort();
  };

public:
  File::Handle get() { return Handle(*this); }
};

class StderrFile : public File {
  static std::vector<char> writeBuffer;

  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is sufficient.
  // This would be a change from the current FS.
  __wasi_errno_t write(const uint8_t* buf, __wasi_size_t len) override {
    return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
  }

  __wasi_errno_t read(const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) override {
    emscripten_console_log("StderrFile::read() has not been implemented yet.");
    abort();
  };

public:
  File::Handle get() { return Handle(*this); }
};

std::vector<char> StdoutFile::writeBuffer;
std::vector<char> StderrFile::writeBuffer;

std::shared_ptr<File>& OpenFileInfo::Handle::getFile() { return openFileInfo.file; }

FileTable::Handle FileTable::get() {
  static FileTable fileTable;
  return FileTable::Handle(fileTable);
}

FileTable::FileTable() {
  entries.push_back(std::make_shared<OpenFileInfo>(0, std::make_shared<StdinFile>()));
  entries.push_back(std::make_shared<OpenFileInfo>(0, std::make_shared<StdoutFile>()));
  entries.push_back(std::make_shared<OpenFileInfo>(0, std::make_shared<StderrFile>()));
}

__wasi_fd_t FileTable::Handle::add(std::shared_ptr<OpenFileInfo> ptr) {
  for (__wasi_fd_t i = 0; i < fileTable.entries.size(); i++) {
    if (!fileTable.entries[i]) {
      // Free open file entry.
      fileTable.entries[i] = ptr;
      return i;
    }
  }

  // Could not find an empty open file table entry.
  fileTable.entries.push_back(ptr);

  return fileTable.entries.size() - 1;
}

void FileTable::Handle::remove(__wasi_fd_t fd) {
  // Check if the file descriptor is invalid.
  assert(fd < fileTable.entries.size() && fd >= 0);

  fileTable.entries[fd] = nullptr;
}

// Operator Overloading for FileTable::Handle::Entry
FileTable::Handle::Entry::operator std::shared_ptr<OpenFileInfo>() const {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return nullptr;
  }

  return fileTableHandle.fileTable.entries[fd];
}

FileTable::Handle::Entry& FileTable::Handle::Entry::operator=(std::shared_ptr<OpenFileInfo> ptr) {
  assert(fd >= 0);

  if (fd >= fileTableHandle.fileTable.entries.size()) {
    fileTableHandle.fileTable.entries.resize(fd + 1);
  }
  fileTableHandle.fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<OpenFileInfo>& FileTable::Handle::Entry::operator->() {
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
