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

std::shared_ptr<Directory> wasmfs_create_root_dir(void) {
  // If nothing is returned, the default root will be created.
  return {};
}

} // namespace wasmfs
