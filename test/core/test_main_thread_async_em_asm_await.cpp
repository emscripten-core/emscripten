// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main()
{
  printf("Before MAIN_THREAD_EM_ASM_AWAIT\n");
  int res = MAIN_THREAD_EM_ASM_AWAIT({
    out('Inside MAIN_THREAD_EM_ASM_AWAIT: ' + $0 + ' ' + $1);
    const asyncOp = new Promise((resolve,reject) => {
      setTimeout(() => {
        out('Inside asyncOp');
        resolve(2);
      }, 1000);
    });
    return asyncOp;
  }, 42, 3.5);
  printf("After MAIN_THREAD_EM_ASM_AWAIT\n");
  printf("result: %d\n", res);
  return 0;
}
