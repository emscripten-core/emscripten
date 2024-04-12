// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "js_file_backend.h"
#include "backend.h"
#include "js_impl_backend.h"
#include "wasmfs.h"

namespace wasmfs {

extern "C" backend_t wasmfs_create_js_file_backend() {
  backend_t backend = wasmFS.addBackend(std::make_unique<JSImplBackend>());
  _wasmfs_create_js_file_backend_js(backend);
  return backend;
}

} // namespace wasmfs
