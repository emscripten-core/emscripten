// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#ifdef __EMSCRIPTEN__
// In emscripten, avoid iostream to avoid using WasmFS code during logging.
#include <emscripten/console.h>
#else
#include <iostream>
#endif

#include "support.h"

namespace wasmfs {

void handle_unreachable(const char* msg, const char* file, unsigned line) {
#ifndef NDEBUG
#ifdef __EMSCRIPTEN__
  if (msg) {
    emscripten_err(msg);
  }
  emscripten_err("UNREACHABLE executed");
  if (file) {
    emscripten_errf("at %s:%d", file, line);
  }
#else // EMSCRIPTEN
  if (msg) {
    std::cerr << msg << "\n";
  }
  std::cerr << "UNREACHABLE executed";
  if (file) {
    std::cerr << " at " << file << ":" << line;
  }
  std::cerr << "!\n";
#endif
  // TODO: sanitizer integration, see binaryen's similar code
#endif
  __builtin_trap();
}

} // namespace wasmfs
