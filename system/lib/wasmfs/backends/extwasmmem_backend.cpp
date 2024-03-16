// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "../paths.h"
#include "backend.h"
#include "memory_backend.h"
#include "support.h"
#include "thread_utils.h"
#include "wasmfs.h"
#include <condition_variable>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include <memory>
#include <mutex>
#include <stdlib.h>
#include <thread>

extern "C" {
int wasmfs_extwasmmem_file_handle_alloc();
void wasmfs_extwasmmem_file_delete(int dataFileHandle);
int wasmfs_extwasmmem_file_open(int dataFileHandle, int bCreate);
ssize_t wasmfs_extwasmmem_file_read(int dataFileHandle,
                                    uint8_t* buf,
                                    unsigned long len,
                                    long offset);
ssize_t wasmfs_extwasmmem_file_write(int dataFileHandle,
                                     const uint8_t* buf,
                                     unsigned long len,
                                     long offset);
int wasmfs_extwasmmem_file_getSize(int dataFileHandle);
int wasmfs_extwasmmem_file_setSize(int dataFileHandle, unsigned long size);
}

namespace wasmfs {

class ExtWasmMemFSFile : public DataFile {
public:
  ExtWasmMemFSFile(mode_t mode, backend_t backend) : DataFile(mode, backend) {
    js_handle = wasmfs_extwasmmem_file_handle_alloc();
  }

  virtual ~ExtWasmMemFSFile() override {
    wasmfs_extwasmmem_file_delete(js_handle);
  }

  // Notify the backend when this file is opened or closed. The backend is
  // responsible for keeping files accessible as long as they are open, even if
  // they are unlinked. Returns 0 on success or a negative error code.
  virtual int open(oflags_t flags) override {
    return wasmfs_extwasmmem_file_open(js_handle, (flags & O_CREAT));
  }
  virtual int close() override { return 0; }

  // Return the accessed length or a negative error code. It is not an error to
  // access fewer bytes than requested. Will only be called on opened files.
  // TODO: Allow backends to override the version of read with
  // multiple iovecs to make it possible to implement pipes. See #16269.
  virtual ssize_t read(uint8_t* buf, size_t len, off_t offset) override {
    return wasmfs_extwasmmem_file_read(js_handle, buf, len, offset);
  }

  virtual ssize_t write(const uint8_t* buf, size_t len, off_t offset) override {
    return wasmfs_extwasmmem_file_write(js_handle, buf, len, offset);
  }

  // The the size in bytes of a file or return a negative error code. May be
  // called on files that have not been opened.
  virtual off_t getSize() override {
    return wasmfs_extwasmmem_file_getSize(js_handle);
  }

  // Sets the size of the file to a specific size. If new space is allocated, it
  // should be zero-initialized. May be called on files that have not been
  // opened. Returns 0 on success or a negative error code.
  virtual int setSize(off_t size) override {
    return wasmfs_extwasmmem_file_setSize(js_handle, size);
  }

  // Sync the file data to the underlying persistent storage, if any. Returns 0
  // on success or a negative error code.
  virtual int flush() override { return 0; }

  int getHandle() { return js_handle; }

private:
  int js_handle;
};

class ExtWasmMemFSBackEnd : public Backend {
public:
  std::shared_ptr<DataFile> createFile(mode_t mode) override {
    return std::make_shared<ExtWasmMemFSFile>(mode, this);
  }
  std::shared_ptr<Directory> createDirectory(mode_t mode) override {
    return std::make_shared<MemoryDirectory>(mode, this);
  }
  std::shared_ptr<Symlink> createSymlink(std::string target) override {
    return std::make_shared<MemorySymlink>(target, this);
  }
};
} // namespace wasmfs

using namespace wasmfs;
extern "C" {
backend_t wasmfs_create_extwasmmem_backend() {
  return wasmFS.addBackend(std::make_unique<ExtWasmMemFSBackEnd>());
}

int EMSCRIPTEN_KEEPALIVE
_extwasmmem_get_file_handle_by_path(const char* path, int createIfNotExist) {
  auto parsedParent = path::parseParent(path);
  if (parsedParent.getError()) {
    return -EFAULT;
  }
  auto& [parent, childNameView] = parsedParent.getParentChild();
  std::string childName(childNameView);

  std::shared_ptr<File> child;
  {
    auto lockedParent = parent->locked();
    child = lockedParent.getChild(childName);
    if (!child) {
      if (createIfNotExist) {
        // Lookup failed; try creating the file.
        child = lockedParent.insertDataFile(childName, 0777);
      }
      if (!child) {
        // File creation failed; nothing else to do.
        return -EFAULT;
      }
    }
  }

  auto dataFile = child->dynCast<DataFile>();
  if (!dataFile) {
    // There is something here but it isn't a data file.
    return -EFAULT;
  }

  auto extwasmmem_file = std::dynamic_pointer_cast<ExtWasmMemFSFile>(dataFile);
  if (extwasmmem_file == nullptr) {
    assert(extwasmmem_file &&
           "file is not hosted by extWasmMem, check your path.");
    return -EFAULT;
  }
  return extwasmmem_file->getHandle();
}
}