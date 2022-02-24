// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>

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
  // TODO: sanitizer integration, see binaryen's similar code
#endif
  __builtin_trap();
}

} // namespace wasmfs
