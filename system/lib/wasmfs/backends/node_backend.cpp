// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory>

#include "backend.h"
#include "backends/node_backend.h"
#include "file.h"

namespace wasmfs {

extern "C" {

backend_t wasmfs_create_node_backend(const char* root) {
  return wasmFS.addBackend(std::make_unique<NodeBackend>(root));
}

void EMSCRIPTEN_KEEPALIVE _wasmfs_node_record_dirent(
  std::vector<Directory::Entry>* entries, const char* name, int type) {
  entries->push_back({name, File::FileKind(type), 0});
}

} // extern "C"

} // namespace wasmfs
