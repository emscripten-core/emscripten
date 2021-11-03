// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

extern "C" {

int noted = 1;

const char* EMSCRIPTEN_KEEPALIVE note(int n) {
  EM_ASM({ out([$0, $1]) }, n, noted);
  noted = noted | n;
  EM_ASM({ out(['noted is now', $0]) }, noted);
  return "silly-string";
}

}

int main() {
  EM_ASM( myJSCallback() ); // calls a global JS func
  assert(noted == 3);
  return 0;
}

