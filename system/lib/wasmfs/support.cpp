// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "support.h"

namespace wasmfs {

void handle_unreachable(const char* msg, const char* file, unsigned line) {
#ifndef NDEBUG
  if (msg) {
    std::cerr << msg << "\n";
  }
  std::cerr << "UNREACHABLE executed";
  if (file) {
    std::cerr << " at " << file << ":" << line;
  }
  std::cerr << "!\n";
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
  __sanitizer_print_stack_trace();
  __builtin_trap();
#endif
#endif
  abort();
}

} // namespace wasmfs
