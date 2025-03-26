/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

int main() {
  EM_ASM({
    customMessageData += '[main]';
    postCustomMessage({ op: 'fromMain' });
  });

  emscripten_exit_with_live_runtime();
  return 0;
}
