/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>

int cmp(const void* key, const void* member) {
  return *(int*)key - *(int*)member;
}

void printResult(int* needle, int* haystack, unsigned int len) {
  void* result = bsearch(needle, haystack, len, sizeof(unsigned int), cmp);

  if (result == NULL) {
    printf("null\n");
  } else {
    printf("%d\n", *(unsigned int*)result);
  }
}

int main() {
  int a[] = {-2, -1, 0, 6, 7, 9};
  int b[] = {0, 1};

  /* Find all keys that exist. */
  for (int i = 0; i < 6; i++) {
    int val = a[i];

    printResult(&val, a, 6);
  }

  /* Keys that are covered by the range of the array but aren't in
   * the array cannot be found.
   */
  int v1 = 3;
  int v2 = 8;
  printResult(&v1, a, 6);
  printResult(&v2, a, 6);

  /* Keys outside the range of the array cannot be found. */
  int v3 = -1;
  int v4 = 2;

  printResult(&v3, b, 2);
  printResult(&v4, b, 2);

  return 0;
}
