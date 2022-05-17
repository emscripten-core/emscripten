// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the global state of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include <emscripten/threading.h>

#include "memory_backend.h"
#include "paths.h"
#include "special_files.h"
#include "wasmfs.h"

namespace wasmfs {

// The below lines are included to make the compiler believe that the global
// constructor is part of a system header, which is necessary to work around a
// compilation error about using a reserved init priority less than 101. This
// ensures that the global state of the file system is constructed before
// anything else. ATTENTION: No other static global objects should be defined
// besides wasmFS. Due to # define _LIBCPP_INIT_PRIORITY_MAX
// __attribute__((init_priority(101))), we must use init priority 100 (reserved
// system priority) since wasmFS is a system level component.
// TODO: consider instead adding this in libc's startup code.
// WARNING: Maintain # n + 1 "wasmfs.cpp" 3 where n = line number.
# 29 "wasmfs.cpp" 3
__attribute__((init_priority(100))) WasmFS wasmFS;
# 31 "wasmfs.cpp"

// These helper functions will be linked in from library_wasmfs.js.
extern "C" {
int _wasmfs_get_num_preloaded_files();
int _wasmfs_get_num_preloaded_dirs();
int _wasmfs_get_preloaded_file_mode(int index);
void _wasmfs_get_preloaded_parent_path(int index, char* parentPath);
void _wasmfs_get_preloaded_path_name(int index, char* fileName);
void _wasmfs_get_preloaded_child_path(int index, char* childName);
}

// If the user does not implement this hook, do nothing.
__attribute__((weak)) extern "C" void wasmfs_before_preload(void) {}

// Set up global data structures and preload files.
WasmFS::WasmFS() : rootDirectory(initRootDirectory()), cwd(rootDirectory) {
  wasmfs_before_preload();
  preloadFiles();
}

WasmFS::~WasmFS() {
  // Flush musl libc streams.
  // TODO: Integrate musl exit() which would call this for us. That might also
  //       help with destructor priority - we need to happen last.
  fflush(0);

  // Flush our own streams. TODO: flush all possible streams.
  // Note that we lock here, although strictly speaking it is unnecessary given
  // that we are in the destructor of WasmFS: nothing can possibly be running
  // on files at this time.
  SpecialFiles::getStdout()->locked().flush();
  SpecialFiles::getStderr()->locked().flush();

  // Break the reference cycle caused by the root directory being its own
  // parent.
  rootDirectory->locked().setParent(nullptr);
}

std::shared_ptr<Directory> WasmFS::initRootDirectory() {
  auto rootBackend = createMemoryFileBackend();
  auto rootDirectory =
    std::make_shared<MemoryDirectory>(S_IRUGO | S_IXUGO | S_IWUGO, rootBackend);
  auto lockedRoot = rootDirectory->locked();

  // The root directory is its own parent.
  lockedRoot.setParent(rootDirectory);

  auto devDirectory =
    std::make_shared<MemoryDirectory>(S_IRUGO | S_IXUGO, rootBackend);
  lockedRoot.mountChild("dev", devDirectory);
  auto lockedDev = devDirectory->locked();

  lockedDev.mountChild("stdin", SpecialFiles::getStdin());
  lockedDev.mountChild("stdout", SpecialFiles::getStdout());
  lockedDev.mountChild("stderr", SpecialFiles::getStderr());
  lockedDev.mountChild("random", SpecialFiles::getRandom());
  lockedDev.mountChild("urandom", SpecialFiles::getURandom());

  return rootDirectory;
}

// Initialize files specified by the --preload-file option.
// Set up directories and files in wasmFS$preloadedDirs and
// wasmFS$preloadedFiles from JS. This function will be called before any file
// operation to ensure any preloaded files are eagerly available for use.
void WasmFS::preloadFiles() {
  // Debug builds only: add check to ensure preloadFiles() is called once.
#ifndef NDEBUG
  static std::atomic<int> timesCalled;
  timesCalled++;
  assert(timesCalled == 1);
#endif

  // Ensure that files are preloaded from the main thread.
  assert(emscripten_is_main_runtime_thread());

  auto numFiles = _wasmfs_get_num_preloaded_files();
  auto numDirs = _wasmfs_get_num_preloaded_dirs();

  // If there are no preloaded files, exit early.
  if (numDirs == 0 && numFiles == 0) {
    return;
  }

  // Iterate through wasmFS$preloadedDirs to obtain a parent and child pair.
  // Ex. Module['FS_createPath']("/foo/parent", "child", true, true);
  for (int i = 0; i < numDirs; i++) {
    char parentPath[PATH_MAX] = {};
    _wasmfs_get_preloaded_parent_path(i, parentPath);

    auto parsed = path::parseFile(parentPath);
    std::shared_ptr<Directory> parentDir;
    if (parsed.getError() ||
        !(parentDir = parsed.getFile()->dynCast<Directory>())) {
      emscripten_console_error(
        "Fatal error during directory creation in file preloading.");
      abort();
    }

    char childName[PATH_MAX] = {};
    _wasmfs_get_preloaded_child_path(i, childName);

    auto lockedParentDir = parentDir->locked();
    if (lockedParentDir.getChild(childName)) {
      // The child already exists, so we don't need to do anything here.
      continue;
    }

    auto inserted =
      lockedParentDir.insertDirectory(childName, S_IRUGO | S_IXUGO);
    assert(inserted && "TODO: handle preload insertion errors");
  }

  for (int i = 0; i < numFiles; i++) {
    char fileName[PATH_MAX] = {};
    _wasmfs_get_preloaded_path_name(i, fileName);

    auto mode = _wasmfs_get_preloaded_file_mode(i);

    auto parsed = path::parseParent(fileName);
    if (parsed.getError()) {
      emscripten_console_error("Fatal error during file preloading");
      abort();
    }
    auto& [parent, childName] = parsed.getParentChild();
    auto created =
      parent->locked().insertDataFile(std::string(childName), (mode_t)mode);
    assert(created && "TODO: handle preload insertion errors");
    created->locked().preloadFromJS(i);
  }
}

} // namespace wasmfs
