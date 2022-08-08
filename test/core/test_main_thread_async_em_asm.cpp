// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main()
{
  // Test that on main browser thread, MAIN_THREAD_ASYNC_EM_ASM() will get
  // synchronously executed.
  printf("Before MAIN_THREAD_ASYNC_EM_ASM\n");
  MAIN_THREAD_ASYNC_EM_ASM({
    out('Inside MAIN_THREAD_ASYNC_EM_ASM: ' + $0 + ' ' + $1)
  }, 42, 3.5);
  printf("After MAIN_THREAD_ASYNC_EM_ASM\n");
#if __EMSCRIPTEN_PTHREADS__
  // Test sending a bunch of async messages, and seeing that they arrive ok
  // on the main thread (this runs in PROXY_TO_PTHREAD, so we are not the
  // main thread).
  MAIN_THREAD_EM_ASM({
    Module.totalStuffSent = 0;
  });
  for (int i = 0; i < 10; i++) {
    // Use a bunch of arguments, to ensure they arrive ok.
    MAIN_THREAD_ASYNC_EM_ASM({
      Module.totalStuffSent += $0 + $1 + $2 + $3 + $4;
    }, 1 * i, 2 * i, 3 * i, 4 * i, 5 * i);
  }
  // Poll for them all arriving - we just need to wait, but their arrival is
  // guaranteed.
  while (!MAIN_THREAD_EM_ASM_INT({
    // We only add, so this should never exceed the target.
    assert(Module.totalStuffSent <= 675);
    return Module.totalStuffSent == 675;
  })) {}
#endif
  return 0;
}
