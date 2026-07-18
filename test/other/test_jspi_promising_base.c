// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>
#include <emscripten/stack.h>

uintptr_t main_base;
uintptr_t nested_base;

// A user-defined reader of the promising stack base marker.
EM_JS(uintptr_t, get_promising_base, (), {
  return Asyncify.lastPromisingStackBase;
});

// Called from JS while main is suspended, as a second promising export.
EMSCRIPTEN_KEEPALIVE void nested(void) {
  nested_base = get_promising_base();
  assert(nested_base >= emscripten_stack_get_current());
  // main consumed stack before suspending, so this promising call was
  // entered deeper into the stack.
  assert(nested_base < main_base);
}

EM_ASYNC_JS(void, call_nested, (), {
  await new Promise((resolve) => setTimeout(resolve, 0));
  await _nested();
});

__attribute__((noinline)) void check_deeper(void) {
  // The marker is stable across call depth within a promising export.
  assert(get_promising_base() == main_base);
}

int main(void) {
  volatile char pad[256];
  pad[0] = 1;
  main_base = get_promising_base();
  assert(main_base != 0);
  assert(main_base >= emscripten_stack_get_current());
  assert(main_base <= emscripten_stack_get_base());
  check_deeper();
  call_nested();
  // The marker tracks the last promising entry; suspensions are not guarded.
  assert(get_promising_base() == nested_base);
  printf("done\n");
  return 0;
}
