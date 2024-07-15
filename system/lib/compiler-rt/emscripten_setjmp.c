/*
* Copyright 2020 The Emscripten Authors.  All rights reserved.
* Emscripten is available under two separate licenses, the MIT license and the
* University of Illinois/NCSA Open Source License.  Both these licenses can be
* found in the LICENSE file.
*/

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <threads.h>

#include "emscripten_internal.h"

#ifdef __USING_WASM_SJLJ__
struct __WasmLongjmpArgs {
  void *env;
  int val;
};
#endif

// jmp_buf should have large enough size and alignment to contain
// this structure.
struct jmp_buf_impl {
  void* func_invocation_id;
  uint32_t label;
#ifdef __USING_WASM_SJLJ__
  struct __WasmLongjmpArgs arg;
#endif
};

void __wasm_setjmp(void* env, uint32_t label, void* func_invocation_id) {
  struct jmp_buf_impl* jb = env;
  assert(label != 0);                 // ABI contract
  assert(func_invocation_id != NULL); // sanity check
  jb->func_invocation_id = func_invocation_id;
  jb->label = label;
}

uint32_t __wasm_setjmp_test(void* env, void* func_invocation_id) {
  struct jmp_buf_impl* jb = env;
  assert(jb->label != 0);             // ABI contract
  assert(func_invocation_id != NULL); // sanity check
  if (jb->func_invocation_id == func_invocation_id) {
    return jb->label;
  }
  return 0;
}

#ifdef __USING_WASM_SJLJ__
// llvm uses `1` for the __c_longjmp tag.
// See https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/CodeGen/WasmEHFuncInfo.h
#define C_LONGJMP 1

// Wasm EH allows us to throw and catch multiple values, but that requires
// multivalue support in the toolchain, whch is not reliable at the time.
// TODO Consider switching to throwing two values at the same time later.
void __wasm_longjmp(void* env, int val) {
  struct jmp_buf_impl* jb = env;
  struct __WasmLongjmpArgs* arg = &jb->arg;
  // C standard says:
  // The longjmp function cannot cause the setjmp macro to return
  // the value 0; if val is 0, the setjmp macro returns the value 1.
  if (val == 0) {
    val = 1;
  }
  arg->env = env;
  arg->val = val;
  __builtin_wasm_throw(C_LONGJMP, arg);
}
#else
void emscripten_longjmp(uintptr_t env, int val) {
  // C standard:
  //   The longjmp function cannot cause the setjmp macro to return
  //   the value 0; if val is 0, the setjmp macro returns the value 1.
  if (val == 0) {
    val = 1;
  }
  setThrew(env, val);
  _emscripten_throw_longjmp();
}
#endif
