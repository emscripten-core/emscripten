// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>
#include <pthread.h>

void *thread_func(void* arg) {
  printf("Before MAIN_THREAD_EM_ASM_AWAIT\n");
  int res = MAIN_THREAD_EM_ASM_AWAIT({
    out('Inside MAIN_THREAD_EM_ASM_AWAIT: ' + $0 + ' ' + $1);
    const asyncOp = new Promise((resolve,reject) => {
      setTimeout(() => {
        out('Inside asyncOp');
        reject(new Error('asyncOp rejected'));
      }, 1000);
    });
    return asyncOp;
  }, 42, 3.5);
  printf("After MAIN_THREAD_EM_ASM_AWAIT rejected\n");
  printf("result: %d\n", res);
  return NULL;
}

int main() {
  // start new thread
  pthread_t thread;
  pthread_create(&thread, NULL, thread_func, NULL);

  // wait for thread to finish
  pthread_join(thread, NULL);
  return 0;
}
