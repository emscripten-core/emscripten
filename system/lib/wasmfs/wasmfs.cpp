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

  int numFiles = EM_ASM_INT({return wasmFS$preloadedFiles.length});
  int numDirs = EM_ASM_INT({return wasmFS$preloadedDirs.length});

  // If there are no preloaded files, exit early.
  if (numDirs == 0 && numFiles == 0) {
    return;
  }

  // Iterate through wasmFS$preloadedDirs to obtain a parent and child pair.
  // Ex. Module['FS_createPath']("/foo/parent", "child", true, true);
  for (int i = 0; i < numDirs; i++) {
    // TODO: Convert every EM_ASM to EM_JS.
    char parentPath[PATH_MAX] = {};
    EM_ASM(
      {
        var s = wasmFS$preloadedDirs[$0].parentPath;
        var len = lengthBytesUTF8(s) + 1;
        stringToUTF8(s, $1, len);
      },
      i,
      parentPath);

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
    EM_ASM(
      {
        var s = wasmFS$preloadedDirs[$0].childName;
        var len = lengthBytesUTF8(s) + 1;
        stringToUTF8(s, $1, len);
      },
      i,
      childName);

    auto created = rootBackend->createDirectory(S_IRUGO | S_IXUGO);

    parentDir->locked().setEntry(childName, created);
  }

  for (int i = 0; i < numFiles; i++) {
    char fileName[PATH_MAX] = {};
    EM_ASM(
      {
        var s = wasmFS$preloadedFiles[$0].pathName;
        var len = lengthBytesUTF8(s) + 1;
        stringToUTF8(s, $1, len);
      },
      i,
      fileName);

    auto mode = EM_ASM_INT({ return wasmFS$preloadedFiles[$0].mode; }, i);

    auto pathParts = splitPath(fileName);

    auto base = pathParts.back();

    // TODO: Generalize so that MemoryFile is not hard-coded.
    auto created = std::make_shared<MemoryFile>((mode_t)mode, rootBackend);

    long err;
    auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

    if (!parentDir) {
      emscripten_console_error("Fatal error during file preloading");
      abort();
    }

    parentDir->locked().setEntry(base, created);

    // TODO: Generalize preloadFromJS to use generic file operations.
    created->locked().preloadFromJS(i);
  }
}
} // namespace wasmfs
