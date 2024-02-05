// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <memory.h>
#include <emscripten.h>

const char *str = "this is static data";

int main() {
#ifdef REPORT_RESULT
  REPORT_RESULT(strlen(str + 
    // throw off optimization
    (int)(emscripten_get_now() / 10000.0)));
#endif
}
