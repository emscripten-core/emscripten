// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#ifdef __cplusplus
#error "should not be compiled as C++"
#endif

#include <stdio.h>

void theFunc(const char *str) {
  printf("side got: %s, over\n", str);
}
