// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory>

#include "backend.h"
#include "file.h"
#include "support.h" // ?
#include "wasmfs.h" // ?

namespace wasmfs {

class NodeBackend;

backend_t wasmfs_create_root_dir(void) {
  return wasmfs_create_node_backend();
}

} // namespace wasmfs
