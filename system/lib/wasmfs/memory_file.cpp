
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