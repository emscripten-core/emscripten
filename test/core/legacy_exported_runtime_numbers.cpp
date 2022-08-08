// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main() {
  int rtn = EM_ASM_INT({
#ifdef DIRECT
    out('|' +  ALLOC_STACK + '|');
    return ALLOC_STACK;
#else
    out('|' + Module['ALLOC_STACK'] + '|');
    return Module['ALLOC_STACK'];
#endif
  });
  printf("|%d|\n", rtn);
  return 0;
}
