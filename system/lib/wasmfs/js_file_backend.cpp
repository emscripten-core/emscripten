// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "js_impl_backend.h"
#include "wasmfs.h"

// See library_wasmfs_js_file.

extern "C" {
void _wasmfs_backend_add_js_file(wasmfs::backend_t);
}

namespace wasmfs {

extern "C" backend_t wasmfs_create_js_file_backend() {
  backend_t backend = wasmFS.addBackend(std::make_unique<JSImplBackend>());
  _wasmfs_backend_add_js_file(backend);
  return backend;
  ;
}

} // namespace wasmfs
