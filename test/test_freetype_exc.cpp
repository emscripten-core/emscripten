/*
 * Minimal test that links FreeType and exercises C++ exception handling.
 * When the freetype port is built with legacy EH but the application uses
 * standardized Wasm EH (-sWASM_LEGACY_EXCEPTIONS=0), the resulting .wasm
 * contains a mix of legacy and new EH instructions and fails at
 * instantiation time.
 */
#include <cstdio>
#include <stdexcept>
#include <ft2build.h>
#include FT_FREETYPE_H

int main() {
  // Exercise C++ exception handling so that new-EH instructions are emitted.
  try {
    throw std::runtime_error("test");
  } catch (const std::exception& e) {
    printf("caught: %s\n", e.what());
  }

  // Also call into FreeType so it is actually linked.
  FT_Library library;
  FT_Error error = FT_Init_FreeType(&library);
  if (error) {
    printf("FT_Init_FreeType failed: %d\n", error);
    return 1;
  }
  printf("FreeType initialized successfully\n");
  FT_Done_FreeType(library);
  return 0;
}
