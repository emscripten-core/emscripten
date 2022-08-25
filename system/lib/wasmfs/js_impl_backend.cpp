#include "js_impl_backend.h"

namespace wasmfs {

std::atomic<js_index_t> JSImplFile::next_index;

std::atomic<js_index_t> JSImplBackend::next_index;

js_index_t JSImplFile::getBackendIndex() {
  JSImplBackend* jsbackend = static_cast<JSImplBackend*>(getBackend());
  return jsbackend->getIndex();
}

} // namespace wasmfs
