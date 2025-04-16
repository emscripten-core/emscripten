This is Poppler. See README and COPYING.

Changes for Emscripten:
  Object.h, Array.cc, Dict.cc are modified to avoid uninitialization errors (search for 'Emscripten')
  poppler-global.h is modified to use std::basic_string<char16_t> to compile with newer versions of
    LLVM (this matches newer upstream versions of poppler)
