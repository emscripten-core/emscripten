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
#include "wasmfs_internal.h"

namespace wasmfs {

#ifdef WASMFS_CASE_INSENSITIVE
backend_t createIgnoreCaseBackend(std::function<backend_t()> createBacken);
#endif

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

// If the user does not implement this hook, do nothing.
__attribute__((weak)) extern "C" void wasmfs_before_preload(void) {}

// Set up global data structures and preload files.
WasmFS::WasmFS() : rootDirectory(initRootDirectory()), cwd(rootDirectory) {
  wasmfs_before_preload();
  preloadFiles();
}

// Manual integration with LSan. LSan installs itself during startup at the
// first allocation, which happens inside WasmFS code (since the WasmFS global
// object creates some data structures). As a result LSan's atexit() destructor
// will be called last, after WasmFS is cleaned up, since atexit() calls work
// are LIFO (like a stack). But that is a problem, since if WasmFS has shut
// down and deallocated itself then the leak code cannot actually print any of
// its findings, if it has any. To avoid that, define the LSan entry point as a
// weak symbol, and call it; if LSan is not enabled this can be optimized out,
// and if LSan is enabled then we'll check for leaks right at the start of the
// WasmFS destructor, which is the last time during which it is valid to print.
// (Note that this means we can't find leaks inside WasmFS code itself, but that
// seems fundamentally impossible for the above reasons, unless we made LSan log
// its findings in a way that does not depend on normal file I/O.)
__attribute__((weak)) extern "C" void __lsan_do_leak_check(void) {}

extern "C" void wasmfs_flush(void) {
  // Flush musl libc streams.
  fflush(0);

  // Flush our own streams. TODO: flush all backends.
  (void)SpecialFiles::getStdout()->locked().flush();
  (void)SpecialFiles::getStderr()->locked().flush();
}

WasmFS::~WasmFS() {
  // See comment above on this function.
  //
  // Note that it is ok for us to call it here, as LSan internally makes all
  // calls after the first into no-ops. That is, calling it here makes the one
  // time that leak checks are run be done here, or potentially earlier, but not
  // later; and as mentioned in the comment above, this is the latest possible
  // time for the checks to run (since right after this nothing can be printed).
  __lsan_do_leak_check();

  // TODO: Integrate musl exit() which would flush the libc part for us. That
  //       might also help with destructor priority - we need to happen last.
  //       (But we would still need to flush the internal WasmFS buffers, see
  //       wasmfs_flush() and the comment on it in the header.)
  wasmfs_flush();

  // Break the reference cycle caused by the root directory being its own
  // parent.
  rootDirectory->locked().setParent(nullptr);
}

// Special backends that want to install themselves as the root use this hook.
// Otherwise, we use the default backends.
__attribute__((weak)) extern backend_t wasmfs_create_root_dir(void) {
#ifdef WASMFS_CASE_INSENSITIVE
  return createIgnoreCaseBackend([]() { return createMemoryBackend(); });
#else
  return createMemoryBackend();
#endif
}

std::shared_ptr<Directory> WasmFS::initRootDirectory() {
  auto rootBackend = wasmfs_create_root_dir();
  auto rootDirectory =
    rootBackend->createDirectory(S_IRUGO | S_IXUGO | S_IWUGO);
  auto lockedRoot = rootDirectory->locked();

  // The root directory is its own parent.
  lockedRoot.setParent(rootDirectory);

  // If the /dev/ directory does not already exist, create it. (It may already
  // exist in NODERAWFS mode, or if those files have been preloaded.)
  auto devDir = lockedRoot.insertDirectory("dev", S_IRUGO | S_IXUGO);
  if (devDir) {
    auto lockedDev = devDir->locked();
    lockedDev.mountChild("null", SpecialFiles::getNull());
    lockedDev.mountChild("stdin", SpecialFiles::getStdin());
    lockedDev.mountChild("stdout", SpecialFiles::getStdout());
    lockedDev.mountChild("stderr", SpecialFiles::getStderr());
    lockedDev.mountChild("random", SpecialFiles::getRandom());
    lockedDev.mountChild("urandom", SpecialFiles::getURandom());
  }

  // As with the /dev/ directory, it is not an error for /tmp/ to already
  // exist.
  lockedRoot.insertDirectory("tmp", S_IRWXUGO);

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
      emscripten_err(
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
      emscripten_err("Fatal error during file preloading");
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
