// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// This file defines the memory file class of the new file system.
// This should be the only backend file type defined in a header since it is the
// default type. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "memory_file.h"

namespace wasmfs {
__wasi_errno_t MemoryFile::write(const uint8_t* buf, size_t len, off_t offset) {
  if (offset + len >= buffer.size()) {
    buffer.resize(offset + len);
  }
  memcpy(&buffer[offset], buf, len);

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t MemoryFile::read(uint8_t* buf, size_t len, off_t offset) {
  assert(offset + len - 1 < buffer.size());
  std::memcpy(buf, &buffer[offset], len);

  return __WASI_ERRNO_SUCCESS;
}

void MemoryFile::Handle::preloadFromJS(int index) {
  getFile()->buffer.resize(
    EM_ASM_INT({return wasmFS$preloadedFiles[$0].fileData.length}, index));
  // Ensure that files are preloaded from the main thread.
  assert(emscripten_is_main_runtime_thread());
  // TODO: Replace every EM_ASM with EM_JS.
  EM_ASM({ HEAPU8.set(wasmFS$preloadedFiles[$1].fileData, $0); },
         getFile()->buffer.data(),
         index);
}
} // namespace wasmfs
