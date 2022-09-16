/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>

int main() {

  int64_t s, m, l;
  printf("%d\n", sscanf("123 1073741823 1125899906842620", "%lld %lld %lld", &s,
                        &m, &l));
  printf("%lld,%lld,%lld\n", s, m, l);

  int64_t negS, negM, negL;
  printf("%d\n", sscanf("-123 -1073741823 -1125899906842620", "%lld %lld %lld",
                        &negS, &negM, &negL));
  printf("%lld,%lld,%lld\n", negS, negM, negL);

  return 0;
}
