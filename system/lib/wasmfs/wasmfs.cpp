// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the global state of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "wasmfs.h"
#include "memory_file.h"
#include "streams.h"
#include <emscripten/threading.h>

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
# 26 "wasmfs.cpp" 3
__attribute__((init_priority(100))) WasmFS wasmFS;
# 28 "wasmfs.cpp"

// These helper functions will be linked in from library_wasmfs.js.
extern "C" {
int _wasmfs_get_num_preloaded_files();
int _wasmfs_get_num_preloaded_dirs();
int _wasmfs_get_preloaded_file_mode(int index);
void _wasmfs_get_preloaded_parent_path(int index, char* parentPath);
void _wasmfs_get_preloaded_path_name(int index, char* fileName);
void _wasmfs_get_preloaded_child_path(int index, char* childName);
}

std::shared_ptr<Directory> WasmFS::initRootDirectory() {
  auto rootBackend = createMemoryFileBackend();
  auto rootDirectory =
    std::make_shared<Directory>(S_IRUGO | S_IXUGO | S_IWUGO, rootBackend);
  auto devDirectory =
    std::make_shared<Directory>(S_IRUGO | S_IXUGO, rootBackend);
  rootDirectory->locked().setEntry("dev", devDirectory);

  auto dir = devDirectory->locked();

  dir.setEntry("stdin", StdinFile::getSingleton());
  dir.setEntry("stdout", StdoutFile::getSingleton());
  dir.setEntry("stderr", StderrFile::getSingleton());

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

  // Obtain the backend of the root directory.
  auto rootBackend = getRootDirectory()->getBackend();

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

    auto pathParts = splitPath(parentPath);

    // TODO: Improvement - cache parent pathnames instead of looking up the
    // directory every iteration.
    long err;
    auto parentDir = getDir(pathParts.begin(), pathParts.end(), err);

    if (!parentDir) {
      emscripten_console_error(
        "Fatal error during directory creation in file preloading.");
      abort();
    }

    char childName[PATH_MAX] = {};
    _wasmfs_get_preloaded_child_path(i, childName);

    auto created = rootBackend->createDirectory(S_IRUGO | S_IXUGO);

    parentDir->locked().setEntry(childName, created);
  }

  for (int i = 0; i < numFiles; i++) {
    char fileName[PATH_MAX] = {};
    _wasmfs_get_preloaded_path_name(i, fileName);

    auto mode = _wasmfs_get_preloaded_file_mode(i);

    auto pathParts = splitPath(fileName);

    auto base = pathParts.back();

    auto created = rootBackend->createFile((mode_t)mode);

    long err;
    auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

    if (!parentDir) {
      emscripten_console_error("Fatal error during file preloading");
      abort();
    }

    parentDir->locked().setEntry(base, created);

    created->locked().preloadFromJS(i);
  }
}
} // namespace wasmfs
