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
  StdinFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StdinFile> getSingleton() {
    static const std::shared_ptr<StdinFile> stdinFile =
      std::make_shared<StdinFile>(S_IRUGO);
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
  StdoutFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StdoutFile> getSingleton() {
    static const std::shared_ptr<StdoutFile> stdoutFile =
      std::make_shared<StdoutFile>(S_IWUGO);
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
  StderrFile(mode_t mode) : DataFile(mode) {}
  static std::shared_ptr<StderrFile> getSingleton() {
    static const std::shared_ptr<StderrFile> stderrFile =
      std::make_shared<StderrFile>(S_IWUGO);
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
    std::shared_ptr<Directory> rootDirectory =
      std::make_shared<Directory>(S_IRUGO | S_IXUGO);
    auto devDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
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
  return -EBADF;
}

std::vector<std::string> splitPath(char* pathname) {
  std::vector<std::string> pathParts;
  char newPathName[strlen(pathname) + 1];
  strcpy(newPathName, pathname);

  // TODO: Support relative paths. i.e. specify cwd if path is relative.
  // TODO: Other path parsing edge cases.
  char* current;
  // Handle absolute path.
  if (newPathName[0] == '/') {
    pathParts.push_back("/");
  }

  current = strtok(newPathName, "/");
  while (current != NULL) {
    pathParts.push_back(current);
    current = strtok(NULL, "/");
  }

  return pathParts;
}

std::shared_ptr<Directory> getDir(std::vector<std::string>::iterator begin,
                                  std::vector<std::string>::iterator end,
                                  long& err) {

  std::shared_ptr<File> curr;
  // Check if the first path element is '/', indicating an absolute path.
  if (*begin == "/") {
    curr = getRootDirectory();
    begin++;
  }

  for (auto it = begin; it != end; ++it) {
    auto directory = curr->dynCast<Directory>();

    // If file is nullptr, then the file was not a Directory.
    // TODO: Change this to accommodate symlinks
    if (!directory) {
      err = -ENOTDIR;
      return nullptr;
    }

    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    directory->locked().printKeys();
#endif
    curr = directory->locked().getEntry(*it);

    // Requested entry (file or directory)
    if (!curr) {
      err = -ENOENT;
      return nullptr;
    }

#ifdef WASMFS_DEBUG
    emscripten_console_log(it->c_str());
#endif
  }

  auto currDirectory = curr->dynCast<Directory>();

  if (!currDirectory) {
    err = -ENOTDIR;
    return nullptr;
  }

  return currDirectory;
}
} // namespace wasmfs
