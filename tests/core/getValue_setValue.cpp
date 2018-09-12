// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include<emscripten.h>

int main() {
#ifdef DIRECT
  EM_ASM({
    setValue(8, 1234, 'i32');
    out('|' + getValue(8, 'i32') + '|');
  });
#else
  EM_ASM({
    Module['setValue'](8, 1234, 'i32');
    out('|' + Module['getValue'](8, 'i32') + '|');
  });
#endif
}

