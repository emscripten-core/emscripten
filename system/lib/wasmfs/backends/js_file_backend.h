// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "wasmfs.h"

extern "C" {

// These helper functions are defined in library_wasmfs_js_file.js

void _wasmfs_create_js_file_backend_js(wasmfs::backend_t);
}
