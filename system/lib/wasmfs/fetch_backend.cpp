// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// This file defines the JS file backend and JS file of the new file system.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "proxied_async_js_impl_backend.h"
#include "wasmfs.h"

// See library_wasmfs_fetch.js

extern "C" {
void _wasmfs_create_fetch_backend_js(wasmfs::backend_t);
}

namespace wasmfs {

extern "C" backend_t wasmfs_create_fetch_backend(char* base_url) {
  // TODO: use base url, cache on JS side
  return wasmFS.addBackend(std::make_unique<ProxiedAsyncJSBackend>(
    [](backend_t backend) { _wasmfs_create_fetch_backend_js(backend); }));
}

} // namespace wasmfs
