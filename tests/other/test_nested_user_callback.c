#include <assert.h>
#include <emscripten.h>
#include <stdlib.h>
#include <stdio.h>

void myatexit() {
  puts("myatexit\n");
}

int main() {
  atexit(myatexit);

  puts("in main");

  EM_ASM({
    // Without this Push, the runtime will exit after the user callback.
    runtimeKeepalivePush();
    // Should generate a warning because we are already within user code.
    callUserCallback(() => out("in user callback"));
    runtimeKeepalivePop();

    assert(!runtimeExited);
    out('before: ' + runtimeExited);
    callUserCallback(() => out("in user callback; without runtimeKeepalivePush"));
    // callUserCallback without runtimeKeepalivePush will result in the runtime
    // exiting at the end of the user callback.
    // So we should never get here:
    assert(false);
  });

  puts("returning from main");
  // Should never get here since the EM_ASM block above causes application to
  // exit.
  assert(0);
  return 0;
}
