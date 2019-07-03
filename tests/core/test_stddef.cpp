/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __cplusplus
#include <cstddef>
using namespace std;
#else
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Declare puts manually, we don't want to include any other headers here
int puts(const char*);
void abort(void);

#ifdef __cplusplus
}
#endif

int main() {
  if (_Alignof(max_align_t) < sizeof(long double))
    abort();
  if (_Alignof(max_align_t) < sizeof(long long))
    abort();
  puts("success");
  return 0;
}
