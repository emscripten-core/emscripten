// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the global state of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "wasmfs.h"
#include "streams.h"

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
# 25 "wasmfs.cpp" 3
__attribute__((init_priority(100))) WasmFS wasmFS;
# 27 "wasmfs.cpp"

std::shared_ptr<Directory> WasmFS::initRootDirectory() {
  auto rootDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
  auto devDirectory = std::make_shared<Directory>(S_IRUGO | S_IXUGO);
  rootDirectory->locked().setEntry("dev", devDirectory);

  auto dir = devDirectory->locked();
  dir.setParent(rootDirectory);

  auto stdinFile = StdinFile::getSingleton();
  auto stdoutFile = StdoutFile::getSingleton();
  auto stderrFile = StderrFile::getSingleton();

  dir.setEntry("stdin", stdinFile);
  dir.setEntry("stdout", stdoutFile);
  dir.setEntry("stderr", stderrFile);

  stdinFile->locked().setParent(devDirectory);
  stdoutFile->locked().setParent(devDirectory);
  stderrFile->locked().setParent(devDirectory);

  return rootDirectory;
}
} // namespace wasmfs
