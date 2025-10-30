#include <emscripten.h>

// Should NOT be removed (used by index.mjs).
EMSCRIPTEN_KEEPALIVE int used_externally() {
  return 42;
}

// Should NOT be removed (used by library.js).
EMSCRIPTEN_KEEPALIVE int used_internally() {
  return 99;
}

// Should be removed.
EMSCRIPTEN_KEEPALIVE int unused() {
  return 0xDEAD;
}
