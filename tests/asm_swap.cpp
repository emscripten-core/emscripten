// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

extern "C" {

int EMSCRIPTEN_KEEPALIVE func() {
  return 10;
}

void EMSCRIPTEN_KEEPALIVE report(int result) {
  REPORT_RESULT(result);
}

}

