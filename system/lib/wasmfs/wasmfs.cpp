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

WasmFS wasmFS __attribute__((init_priority(65535)));

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
} // namespace wasmfs
