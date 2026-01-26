/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

void *emscripten_return_address(int level);
const char *emscripten_pc_get_function(void* pc);
uintptr_t emscripten_stack_snapshot();

__attribute__((noinline)) void func() {
  // emscripten_pc_get_function only works for addresses that are in the
  // `UNWIND_CACHE`.  This is populated on demand by
  // `emscripten_stack_snapshot`.
  emscripten_stack_snapshot();
  void* rtn_addr = emscripten_return_address(0);
  void* rtn_addr2 = __builtin_return_address(0);
  const char* caller_name = emscripten_pc_get_function(rtn_addr);
  printf("emscripten_return_address: %p\n", rtn_addr);
  printf("emscripten_pc_get_function: %s\n", caller_name);
  assert(rtn_addr != 0);
  assert(rtn_addr2 != 0);
  assert(rtn_addr == rtn_addr2);
  assert(strcmp(caller_name, "main") == 0);
  assert(emscripten_return_address(50) == 0);
}

// We need to take these two arguments or clang can potentially generate
// a main function that takes two arguments and calls our main, messing up
// the stack trace and breaking this test.
int main(int argc, char **argv) {
  assert(emscripten_return_address(50) == 0);
  func();
  puts("passed");
}
