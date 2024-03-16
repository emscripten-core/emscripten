/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file allows tests to run using a particular backend as the root.

#pragma once

#include <emscripten/wasmfs.h>

backend_t wasmfs_create_root_dir() {
#ifdef WASMFS_NODE_BACKEND
  return wasmfs_create_node_backend(".");
#elif defined(WASMFS_FETCH_BACKEND)
  return wasmfs_create_fetch_backend("http://localhost:8888/")
#elif defined(WASMFS_OPFS_BACKEND)
  return wasmfs_create_opfs_backend();
#elif defined(WASMFS_JS_FILE_BACKEND)
  return wasmfs_create_js_file_backend();
#else // defined(WASMFS_MEMORY_BACKEND)
  return wasmfs_create_memory_backend();
#endif
}
