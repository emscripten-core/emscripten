// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "emscripten/wasmfs.h"

namespace wasmfs {

backend_t wasmfs_create_root_dir(void) {
  return wasmfs_create_node_backend(".");
}

} // namespace wasmfs
