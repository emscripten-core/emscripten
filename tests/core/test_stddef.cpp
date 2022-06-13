/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __cplusplus
#include <cassert>
#include <cstddef>
using namespace std;
#else
#include <assert.h>
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
  // max_align_t on wasm backend is 8.
  assert(_Alignof(max_align_t) == 8);
  puts("success");
  return 0;
}
