/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

int64_t ccv_cache_generate_signature(char *msg, int len, int64_t sig_start,
                                     ...) {
  if (sig_start < 10123) printf("%s\n", msg + len);
  va_list v;
  va_start(v, sig_start);
  if (sig_start > 1413)
    printf("%d\n", va_arg(v, int));
  else
    printf("nada\n");
  va_end(v);
  return len * sig_start * (msg[0] + 1);
}

int main(int argc, char **argv) {
  argv[0] = (char*)"...";
  for (int i = 0; i < argc; i++) {
    int64_t x;
    if (i % 123123 == 0)
      x = ccv_cache_generate_signature(argv[i], i + 2, (int64_t)argc * argc,
                                       54.111);
    else
      x = ccv_cache_generate_signature(argv[i], i + 2, (int64_t)argc * argc,
                                       13);
    printf("%lld\n", x);
  }
};
