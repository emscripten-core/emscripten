// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten/emscripten.h>

// Called from JavaScript after the module loads.
extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* greet() {
  return "Hello from WebAssembly!";
}

} // extern "C"
