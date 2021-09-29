// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// wasmfs.cpp will implement a new file system that replaces the existing JS filesystem.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <mutex>
#include <stdlib.h>
#include <utility>
#include <vector>
#include <wasi/api.h>

extern "C" {

using block_t = uint32_t;

class File {
  std::string name;
  uint32_t uid;  // User ID of the owner
  uint32_t gid;  // Group ID of the owning group
  uint32_t mode; // r/w/x modes
  time_t ctime;  // Time when the inode was last modified
  time_t mtime;  // Time when the content was last modified
  time_t atime;  // Time when the content was last accessed

  std::vector<std::shared_ptr<block_t>> blocks; // pointers to data blocks

  std::mutex file_mutex;

public:
  virtual size_t read(
    __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
    return 1;
  };
  virtual size_t write(
    __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
    return 1;
  };
};

class OpenFile {
  std::shared_ptr<File> fileNode;
  uint32_t _offset;
  std::mutex openfile_mutex;

public:
  OpenFile(uint32_t offset) { this->_offset = offset; }
};

class FileTable {
  std::vector<std::shared_ptr<OpenFile>> handles;
  std::mutex ft_mutex;

public:
  std::vector<std::shared_ptr<OpenFile>> get_file_handles() { return handles; }
  std::shared_ptr<OpenFile> get_open_file(__wasi_fd_t fd) {
    // Check if the file descriptor is invalid
    const std::lock_guard<std::mutex> lock(ft_mutex);
    if (fd >= handles.size() || fd < 0)
      return nullptr;

    return handles[fd];
  }

  __wasi_fd_t add_open_file(std::shared_ptr<OpenFile> ptr) {
    const std::lock_guard<std::mutex> lock(ft_mutex);
    for (__wasi_fd_t i = 0; i < handles.size(); i++) {
      if (!handles[i]) {
        // free open file entry
        handles[i] = ptr;
        return i;
      }
    }

    // could not find an empty open file table entry
    handles.push_back(ptr);

    return handles.size() - 1;
  }

  int remove_open_file(__wasi_fd_t fd) {
    const std::lock_guard<std::mutex> lock(ft_mutex);
    // Check if the file descriptor is invalid
    if (fd >= handles.size() || fd < 0)
      return -1;

    handles[fd] = nullptr;
    return 0;
  }

  int set_open_file(__wasi_fd_t fd, std::shared_ptr<OpenFile> ptr) {
    const std::lock_guard<std::mutex> lock(ft_mutex);
    if (fd >= handles.size() || fd < 0)
      return -1;
    handles[fd] = ptr;

    return 0;
  }
};

static std::vector<char> fd_write_stdstream_buffer;
static FileTable* ft = nullptr;

void populate_file_table() {
  ft = new FileTable();
  ft->add_open_file(std::make_shared<OpenFile>(1));
  ft->add_open_file(std::make_shared<OpenFile>(1));
  ft->add_open_file(std::make_shared<OpenFile>(1));
}

__wasi_fd_t __syscall63(__wasi_fd_t oldfd, __wasi_fd_t newfd) { // dup2
  if (newfd < 0) {
    return -1;
  }
  if (!ft) {
    populate_file_table();
  }

  if (oldfd == newdf) {
    return oldfd;
  }

  // If oldfd is not a valid file descriptor, then the call fails,
  // and newfd is not closed.

  std::shared_ptr<OpenFile> oldfd_openfile = ft->get_open_file(fd);
  // Return -1 if the fd is invalid or points to a nullptr
  if (oldfd_openfile == nullptr)
    return -1;

  // If the file descriptor newfd was previously open, it is closed
  // before being reused; the close is performed silently.

  std::shared_ptr<OpenFile> newfd_openfile = ft->get_open_file(newfd);
  // TODO: what happens if newfd > size of the vector?
  if (newfd_openfile) {
    ft->remove_open_file(newfd);
  }

  set_open_file(newfd, oldfd_openfile);

  return newfd;
}

__wasi_fd_t __syscall41(__wasi_fd_t fd) { // dup
  EM_ASM({ Module.print("dup entered"); });

  if (!ft) {
    _populateFileTable();
  }

  std::shared_ptr<OpenFile> curr_openfile = ft->get_open_file(fd);
  // Return -1 if the fd is invalid
  if (curr_openfile == nullptr)
    return -1;

  // Find the first free open file entry
  __wasi_fd_t ret_fd = ft->add_open_file(curr_openfile);

  for (auto x : ft->get_file_handles()) {
    EM_ASM({ Module.print("something count:" + $0); }, x.use_count());
  }

  return ret_fd;
}

__wasi_errno_t __wasi_fd_write(
  __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  if (!ft) {
    _populateFileTable();
  }
  // FD 1 = STDOUT and FD 2 = STDERR.
  // Temporary hardcoding of filedescriptor values.
  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not show in HTML - a console.warn in a worker is suffficient.
  // This would be a change from the current FS.
  if (fd == 1 || fd == 2) {
    __wasi_size_t num = 0;
    for (size_t i = 0; i < iovs_len; i++) {
      const uint8_t* buf = iovs[i].buf;
      __wasi_size_t len = iovs[i].buf_len;
      for (__wasi_size_t j = 0; j < len; j++) {
        uint8_t current = buf[j];
        if (current == 0 || current == 10) {
          fd_write_stdstream_buffer.push_back('\0'); // for null-terminated C strings
          if (fd == 1) {
            emscripten_console_log(&fd_write_stdstream_buffer[0]);
          } else if (fd == 2) {
            emscripten_console_error(&fd_write_stdstream_buffer[0]);
          }
          fd_write_stdstream_buffer.clear();
        } else {
          fd_write_stdstream_buffer.push_back(current);
        }
      }
      num += len;
    }
    *nwritten = num;
  }
  return 0;
}

__wasi_errno_t __wasi_fd_seek(
  __wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t* newoffset) {
  emscripten_console_log("__wasi_fd_seek has been temporarily stubbed and is inert");
  abort();
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  emscripten_console_log("__wasi_fd_close has been temporarily stubbed and is inert");
  abort();
}

__wasi_errno_t __wasi_fd_read(
  __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
  emscripten_console_log("__wasi_fd_read has been temporarily stubbed and is inert");
  abort();
}
}
