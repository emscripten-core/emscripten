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

static __wasi_errno_t writeStdBuffer(const uint8_t* buf,
                                     size_t len,
                                     void (*console_write)(const char*),
                                     std::vector<char>& fd_write_buffer) {
  for (size_t j = 0; j < len; j++) {
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

class StdinFile : public DataFile {

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

public:
  static std::shared_ptr<StdinFile> getSingleton() {
    static const std::shared_ptr<StdinFile> stdinFile =
      std::make_shared<StdinFile>();
    return stdinFile;
  }
};

class StdoutFile : public DataFile {
  std::vector<char> writeBuffer;

  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return writeStdBuffer(buf, len, &emscripten_console_log, writeBuffer);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

public:
  static std::shared_ptr<StdoutFile> getSingleton() {
    static const std::shared_ptr<StdoutFile> stdoutFile =
      std::make_shared<StdoutFile>();
    return stdoutFile;
  }
};

class StderrFile : public DataFile {
  std::vector<char> writeBuffer;

  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is sufficient.
  // This would be a change from the current FS.
  __wasi_errno_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return writeStdBuffer(buf, len, &emscripten_console_error, writeBuffer);
  }

  __wasi_errno_t read(uint8_t* buf, size_t len, off_t offset) override {
    return __WASI_ERRNO_INVAL;
  };

public:
  static std::shared_ptr<StderrFile> getSingleton() {
    static const std::shared_ptr<StderrFile> stderrFile =
      std::make_shared<StderrFile>();
    return stderrFile;
  }
};

FileTable::FileTable() {
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_RDONLY, StdinFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StdoutFile::getSingleton()));
  entries.push_back(
    std::make_shared<OpenFileState>(0, O_WRONLY, StderrFile::getSingleton()));
}

// Initialize default directories including dev/stdin, dev/stdout, dev/stderr.
// Refers to same std streams in the open file table.
std::shared_ptr<Directory> getRootDirectory() {
  static const std::shared_ptr<Directory> rootDirectory = [] {
    std::shared_ptr<Directory> rootDirectory = std::make_shared<Directory>();
    auto devDirectory = std::make_shared<Directory>();
    rootDirectory->locked().setEntry("dev", devDirectory);

    auto dir = devDirectory->locked();

    dir.setEntry("stdin", StdinFile::getSingleton());
    dir.setEntry("stdout", StdoutFile::getSingleton());
    dir.setEntry("stderr", StderrFile::getSingleton());

    return rootDirectory;
  }();

  return rootDirectory;
}

FileTable::Handle FileTable::get() {
  static FileTable fileTable;
  return FileTable::Handle(fileTable);
}

FileTable::Handle::Entry&
FileTable::Handle::Entry::operator=(std::shared_ptr<OpenFileState> ptr) {
  assert(fd >= 0);

  if (fd >= fileTableHandle.fileTable.entries.size()) {
    fileTableHandle.fileTable.entries.resize(fd + 1);
  }
  fileTableHandle.fileTable.entries[fd] = ptr;

  return *this;
}

std::shared_ptr<OpenFileState> FileTable::Handle::Entry::unlocked() {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return nullptr;
  }

  return fileTableHandle.fileTable.entries[fd];
}

FileTable::Handle::Entry::operator bool() const {
  if (fd >= fileTableHandle.fileTable.entries.size() || fd < 0) {
    return false;
  }

  return fileTableHandle.fileTable.entries[fd] != nullptr;
}

__wasi_fd_t
FileTable::Handle::add(std::shared_ptr<OpenFileState> openFileState) {
  Handle& self = *this;
  // TODO: add freelist to avoid linear lookup time.
  for (__wasi_fd_t i = 0;; i++) {
    if (!self[i]) {
      // Free open file entry.
      self[i] = openFileState;
      return i;
    }
  }
  return -(EBADF);
}

__wasi_errno_t offsetWrite(__wasi_fd_t fd,
                           const __wasi_ciovec_t* iovs,
                           size_t iovs_len,
                           __wasi_filesize_t offset,
                           __wasi_size_t* nwritten,
                           bool setPosition) {
  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  auto openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto lockedOpenFile = openFile.locked();
  auto* file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setPosition ? lockedOpenFile.position() : offset;
  off_t oldOffset = currOffset;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    size_t len = iovs[i].buf_len;

    // Check if buf_len specifies a positive length buffer but buf is a
    // null pointer
    if (!buf && len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    auto result = lockedFile.write(buf, len, currOffset);

    if (result != __WASI_ERRNO_SUCCESS) {
      *nwritten = currOffset - oldOffset;
      if (setPosition) {
        lockedOpenFile.position() = currOffset;
      }

      return result;
    }
    currOffset += len;
  }
  *nwritten = currOffset - oldOffset;
  if (setPosition) {
    lockedOpenFile.position() = currOffset;
  }
  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t offsetRead(__wasi_fd_t fd,
                          const __wasi_iovec_t* iovs,
                          size_t iovs_len,
                          __wasi_filesize_t offset,
                          __wasi_size_t* nread,
                          bool setPosition) {
  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  auto openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto lockedOpenFile = openFile.locked();
  auto* file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setPosition ? lockedOpenFile.position() : offset;
  off_t oldOffset = currOffset;
  size_t size = lockedFile.size();
  for (size_t i = 0; i < iovs_len; i++) {
    // Check if currOffset has exceeded size of file data.
    ssize_t dataLeft = size - currOffset;
    if (dataLeft <= 0) {
      break;
    }

    uint8_t* buf = iovs[i].buf;

    // Check if buf_len specifies a positive length buffer
    // but buf is a null pointer.
    if (!buf && iovs[i].buf_len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    size_t bytesToRead = std::min(size_t(dataLeft), iovs[i].buf_len);

    auto result = lockedFile.read(buf, bytesToRead, currOffset);

    if (result != __WASI_ERRNO_SUCCESS) {
      *nread = currOffset - lockedOpenFile.position();
      if (setPosition) {
        lockedOpenFile.position() = currOffset;
      }
      return result;
    }
    currOffset += bytesToRead;
  }
  *nread = currOffset - oldOffset;
  if (setPosition) {
    lockedOpenFile.position() = currOffset;
  }
  return __WASI_ERRNO_SUCCESS;
}
} // namespace wasmfs
