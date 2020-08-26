// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include<emscripten.h>

int main() {
  char buffer[8];
#ifdef DIRECT
  EM_ASM({
    setValue($0, 1234, 'i32');
    out('|' + getValue($0, 'i32') + '|');
  }, buffer);
#else
  EM_ASM({
    Module['setValue']($0, 1234, 'i32');
    out('|' + Module['getValue']($0, 'i32') + '|');
  }, buffer);
#endif
}

