// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main() {
  printf("main() called.\n");
  int result = emscripten_run_script_int("Module.okk");
  REPORT_RESULT(result);
  return 1;
}

