/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This file provides a `get_backend` function based on defines set on test
// command lines. Test programs should call `get_backend()` to get the backend
// to use when setting up their test file systems.

#pragma once

#include <emscripten/wasmfs.h>

static backend_t get_backend() {
#ifdef WASMFS_MEMORY_BACKEND
  return wasmfs_create_memory_backend();
#else
#ifdef WASMFS_NODE_BACKEND
  return wasmfs_create_node_backend(".");
#else
#error "Expected backend define in compile command"
#endif
#endif
}
