/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <assert.h>

int main() {
  int x = EM_ASM_INT({ return stackSave(); });
  int y = EM_ASM_INT({ return stackSave(); });
  EM_ASM_INT({ out($0); }, &x);
  EM_ASM_INT({ out($0); }, &y);
  assert(x == y);
  EM_ASM({ out('success'); });
}
