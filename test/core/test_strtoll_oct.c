/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main() {
  const char *STRING = "0 -035 +04711";
  char *end_char;

  // undefined base
  long long int l1 = strtoll(STRING, &end_char, 0);
  assert(l1 == 0);
  long long int l2 = strtoll(end_char, &end_char, 0);
  assert(l2 == -29);
  long long int l3 = strtoll(end_char, NULL, 0);
  assert(l3 == 2505);

  // defined base
  long long int l4 = strtoll(STRING, &end_char, 8);
  assert(l4 == 0);
  long long int l5 = strtoll(end_char, &end_char, 8);
  assert(l5 == -29);
  long long int l6 = strtoll(end_char, NULL, 8);
  assert(l6 == 2505);

  printf("%lli\n", l1);
  printf("%lli\n", l2);
  printf("%lli\n", l3);
  printf("%lli\n", l4);
  printf("%lli\n", l5);
  printf("%lli\n", l6);
  return 0;
}
