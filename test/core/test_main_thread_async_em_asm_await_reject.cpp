// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/console.h>
#include <pthread.h>

void* thread_func(void*) {
  emscripten_out("Before MAIN_THREAD_EM_ASM_AWAIT");
  int res = MAIN_THREAD_EM_ASM_AWAIT({
    out('Inside MAIN_THREAD_EM_ASM_AWAIT: ' + $0 + ' ' + $1);
    const asyncOp = new Promise((resolve, reject) => {
      setTimeout(() => {
        out('Inside asyncOp');
        reject(new Error('asyncOp rejected'));
      }, 1000);
    });
    return asyncOp;
  }, 42, 3.5);
  emscripten_out("After MAIN_THREAD_EM_ASM_AWAIT rejected");
  emscripten_outf("result: %d", res);
  return NULL;
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread_func, NULL);
  pthread_join(thread, NULL);
  return 0;
}
