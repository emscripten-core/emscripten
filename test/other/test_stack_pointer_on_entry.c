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

// A user-defined reader of the entry stack pointer marker.
EM_JS(uintptr_t, get_entry_sp, (), {
  return Asyncify.stackPointerOnEntry;
});

#ifdef TEST_JSPI
uintptr_t nested_base;

// Called from JS while main is suspended, as a second promising export.
EMSCRIPTEN_KEEPALIVE void nested(void) {
  nested_base = get_entry_sp();
  assert(nested_base >= emscripten_stack_get_current());
  // main consumed stack before suspending, so this promising call was
  // entered deeper into the stack.
  assert(nested_base < main_base);
}

EM_ASYNC_JS(void, call_nested, (), {
  await new Promise((resolve) => setTimeout(resolve, 0));
  await _nested();
});
#endif

__attribute__((noinline)) void check_deeper(void) {
  // The marker is stable across call depth within an export.
  assert(get_entry_sp() == main_base);
}

int main(void) {
  volatile char pad[256];
  pad[0] = 1;
  main_base = get_entry_sp();
  assert(main_base != 0);
  assert(main_base >= emscripten_stack_get_current());
  assert(main_base <= emscripten_stack_get_base());
  check_deeper();
#ifdef TEST_JSPI
  call_nested();
  // The marker tracks the last promising entry; suspensions are not guarded.
  assert(get_entry_sp() == nested_base);
#else
  emscripten_sleep(0);
  // The rewind re-enters the outermost export with the stack pointer restored
  // to its entry value, so the marker is stable across sleeps.
  assert(get_entry_sp() == main_base);
#endif
  printf("done\n");
  return 0;
}
