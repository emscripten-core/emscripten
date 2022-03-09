// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the modular backend abstract class.
// Other file system backends can use this to interface with the new file
// system. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#pragma once

#include "file.h"
#include "memory_backend.h"

namespace wasmfs {
// A backend (or modular backend) provides a base for the new file system to
// extend its storage capabilities. Files and directories will be represented
// in the file system structure, but their underlying backing could exist in
// persistent storage, another thread, etc.
class Backend {

public:
  virtual std::shared_ptr<DataFile> createFile(mode_t mode) = 0;

  // By default all backends create normal Directory instances for directories.
  // That is, the default behavior is to keep directory structure in-memory.
  virtual std::shared_ptr<Directory> createDirectory(mode_t mode) {
    return std::make_shared<MemoryDirectory>(mode, this);
  }

  // By default all backends create normal Symlink instances for symlinks.
  // That is, the default behavior is to implement symlinks in the trivial
  // manner and in-memory.
  virtual std::shared_ptr<Symlink> createSymlink(std::string target) {
    return std::make_shared<Symlink>(target, this);
  }

  virtual ~Backend() = default;
};

// This will return an instance of a MemoryFileBackend.
// Note: Backends will be defined in cpp files, but functions to instantiate
// them will be defined in a header file. This is so that any unused backends
// are not linked in if they are not called.
backend_t createMemoryFileBackend();

typedef backend_t (*backend_constructor_t)(void*);
} // namespace wasmfs
