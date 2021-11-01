// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the global state of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "wasmfs.h"
#include "file.h"
#include "streams.h"

namespace wasmfs {

// The below lines are included to make the preprocessor believe that the global
// constructor is included as a header. This ensures that the global state of
// the file system is constructed before anything else. ATTENTION: No other
// static global objects should be defined besides wasmFS.
// Due to # define _LIBCPP_INIT_PRIORITY_MAX
// __attribute__((init_priority(101))), we must use init priority 100 (reserved
// system priority) since wasmFS is a system level component.
# 19 "wasmfs.cpp" 3
__attribute__((init_priority(100))) WasmFS wasmFS;
# 21 "wasmfs.cpp"

std::shared_ptr<Directory> WasmFS::initRootDirectory() {
  std::shared_ptr<Directory> rootDirectory =
    std::make_shared<Directory>(S_IRUGO | S_IXUGO);
  auto devDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
  rootDirectory->locked().setEntry("dev", devDirectory);

  auto dir = devDirectory->locked();

  dir.setEntry("stdin", StdinFile::getSingleton());
  dir.setEntry("stdout", StdoutFile::getSingleton());
  dir.setEntry("stderr", StderrFile::getSingleton());

  return rootDirectory;
}

// Initialize files specified by --preload-file option.
// Set up directories and files from preloadedDirs and preloadedFiles
// from JS. This function will be called before any file operation to ensure any
// preloaded files are eagerly available for use.
void WasmFS::preloadFiles() {
  // Add check to ensure preloadFiles() is called once in Debug builds only.
#ifndef NDEBUG
  static std::atomic<int> timesCalled;
  timesCalled++;
  assert(timesCalled == 1);
#endif

  int numFiles = EM_ASM_INT({return FS.preloadedFiles.length});
  int numDirs = EM_ASM_INT({return FS.preloadedDirs.length});

  // If there are no preloaded files, exit early.
  if (numDirs == 0 && numFiles == 0) {
    return;
  }

  // Iterate through FS.preloadedDirs to obtain parent and child pair.
  // Ex. Module['FS_createPath']("/foo/parent", "child", true, true);
  for (int i = 0; i < numDirs; i++) {

    char parentPath[PATH_MAX] = {};
    EM_ASM(
      {
        var s = FS.preloadedDirs[$0].parentPath;
        var len = lengthBytesUTF8(s) + 1;
        var numBytesWritten = stringToUTF8(s, $1, len);
      },
      i,
      parentPath);

    auto pathParts = splitPath(parentPath);

    // TODO: Improvement - cache parent path names instead of looking up the
    // Directory every iteration.
    long err;
    auto parentDir = getDir(pathParts.begin(), pathParts.end(), err);

    if (!parentDir) {
      emscripten_console_log(
        "Fatal error during directory creation in file preloading.");
      abort();
    }

    char childName[PATH_MAX] = {};
    EM_ASM(
      {
        var s = FS.preloadedDirs[$0].childName;
        var len = lengthBytesUTF8(s) + 1;
        var numBytesWritten = stringToUTF8(s, $1, len);
      },
      i,
      childName);

    auto created = std::make_shared<Directory>(S_IRUGO | S_IXUGO);

    parentDir->locked().setEntry(childName, created);
  }

  for (int i = 0; i < numFiles; i++) {
    char fileName[PATH_MAX] = {};
    EM_ASM(
      {
        var s = FS.preloadedFiles[$0].pathName;
        var len = lengthBytesUTF8(s) + 1;
        stringToUTF8(s, $1, len);
      },
      i,
      fileName);

    auto mode = (mode_t)EM_ASM_INT({ return FS.preloadedFiles[$0].mode; }, i);

    auto size = EM_ASM_INT({return FS.preloadedFiles[$0].fileData.length}, i);

    auto pathParts = splitPath(fileName);

    auto base = pathParts[pathParts.size() - 1];

    auto created = std::make_shared<MemoryFile>((mode_t)mode);

    long err;
    auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

    if (!parentDir) {
      emscripten_console_log("Fatal error during file preloading");
      abort();
    }

    parentDir->locked().setEntry(base, created);

    created->locked().writeFromJS(i, size);
  }
}
} // namespace wasmfs
