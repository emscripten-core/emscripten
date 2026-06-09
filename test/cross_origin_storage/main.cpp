// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Example: Cross-Origin Storage (COS) integration
//
// Build with:
//   emcc main.cpp -o index.js \
//       -O2 \
//       -sCROSS_ORIGIN_STORAGE \
//       -sENVIRONMENT=web \
//       -sEXPORTED_RUNTIME_METHODS=ccall \
//       -sEXPORTED_FUNCTIONS=_greet \
//       -sALLOW_MEMORY_GROWTH
//
// Serve the directory over HTTP (e.g. `emrun .` or `python3 -m http.server`)
// and open index.html in a browser that has the COS extension installed.

#include <stdio.h>
#include <emscripten/emscripten.h>

// Called from JavaScript after the module loads.
extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* greet() {
  return "Hello from WebAssembly!";
}

} // extern "C"
