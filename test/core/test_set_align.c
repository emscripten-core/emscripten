/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

volatile char data[16];

__attribute__((noinline)) void *get_aligned(int align)
{
  char *ptr = (char*)(((uintptr_t)(data + 7)) & ~7); // Make 8-byte aligned
  ptr += align; // Now 'align' aligned
  return (void*)ptr;
}

int main()
{
  emscripten_align4_double *d4 = (emscripten_align4_double*)get_aligned(4);
  *d4 = 17.0;
  printf("addr: %ld, value: %f\n", ((uintptr_t)d4) % 8, *d4);

  emscripten_align2_double *d2 = (emscripten_align2_double*)get_aligned(2);
  *d2 = 18.0;
  printf("addr: %ld, value: %f\n", ((uintptr_t)d2) % 8, *d2);

  emscripten_align1_double *d1 = (emscripten_align1_double*)get_aligned(1);
  *d1 = 19.0;
  printf("addr: %ld, value: %f\n", ((uintptr_t)d1) % 8, *d1);

  emscripten_align2_float *f2 = (emscripten_align2_float*)get_aligned(2);
  *f2 = 20.0;
  printf("addr: %ld, value: %f\n", ((uintptr_t)f2) % 4, *f2);

  emscripten_align1_float *f1 = (emscripten_align1_float*)get_aligned(1);
  *f1 = 21.0;
  printf("addr: %ld, value: %f\n", ((uintptr_t)f1) % 4, *f1);

  emscripten_align2_int *i2 = (emscripten_align2_int*)get_aligned(2);
  *i2 = 22;
  printf("addr: %ld, value: %d\n", ((uintptr_t)i2) % 4, *i2);

  emscripten_align1_int *i1 = (emscripten_align1_int*)get_aligned(1);
  *i1 = 23;
  printf("addr: %ld, value: %d\n", ((uintptr_t)i1) % 4, *i1);

  emscripten_align1_short *s1 = (emscripten_align1_short*)get_aligned(1);
  *s1 = 24;
  printf("addr: %ld, value: %d\n", ((uintptr_t)s1) % 4, (int)*s1);

  return 0;
}

