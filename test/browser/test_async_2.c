// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

__attribute__((noinline)) int calc(int x) {
  printf("..%d..\n", x);
  if (x == 0) {
    // We reached the desired number of stack frames. Log out the stack trace
    // for purposes of debugging this test.
    char buffer[10000];
    emscripten_get_callstack(EM_LOG_JS_STACK, buffer, 10000);
    int newlines = 0;
    char *b = buffer;
    while (*b) {
      if (*b == '\n') newlines++;
      b++;
    }
    // The runtime adds more frames from JS, so do not look for a specific
    // number, but ensure it is quite large.
    if (newlines > 40) newlines = 40;
    printf("stack: %s => %d, sleeping...\n", buffer, newlines);
    // Sleep to test the saving of a fairly deep stack, including locals along
    // the way.
    emscripten_sleep(1);
    printf("..and we're back, returning %d!\n", newlines);
    // Return a value to test it passing through all the stack frames to the
    // caller, after the sleep.
    return newlines;
  }
  // Keep the recursion by passing the function pointer between C++ and JS, so
  // that we have a deeply nested stack.
  int (*fp)(int) = (int(*)(int))EM_ASM_PTR({
    return $0;
  }, &calc);
  return fp(x - 1);
}

int main() {
  // Ensure at least 40 stack frames.
  int x = 40;
  int result = calc(x);
  printf("calc(%d) = %d\n", x, result);
  assert(result == 40);
  return 0;
}

