// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the modular backend abstract class.

#pragma once

#include "file.h"

namespace wasmfs {
// A backend (or modular backend) provides a base to extend WasmFS with new
// storage capabilities. Files and directories will be represented in the file
// system structure, but their underlying backing could exist in persistent
// storage, another thread, etc.
class Backend {

public:
  virtual std::shared_ptr<DataFile> createFile(mode_t mode) = 0;
  virtual std::shared_ptr<Directory> createDirectory(mode_t mode) = 0;
  virtual std::shared_ptr<Symlink> createSymlink(std::string target) = 0;

  virtual void populateRoot(Directory::Handle& lockedRoot) {}

  // Indicates whether this backend relies on WasmFS to resolve paths and
  // traverse the directory hierarchy.
  // - true (default): WasmFS performs path parsing, symlink resolution,
  //   and intermediate directory traversal for this backend.
  // - false: the backend handles full paths itself (e.g. NODERAWFS), so
  //   WasmFS should pass paths as-is without interpreting them.
  virtual bool requiresPathResolution() { return true; }

  virtual ~Backend() = default;
};

typedef backend_t (*backend_constructor_t)(void*);
} // namespace wasmfs
