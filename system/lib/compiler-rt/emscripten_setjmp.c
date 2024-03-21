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

// 0 - Nothing thrown
// 1 - Exception thrown
// Other values - jmpbuf pointer in the case that longjmp was thrown
static uintptr_t setjmpId = 0;

typedef struct TableEntry {
  uintptr_t id;
  uint32_t label;
} TableEntry;

extern void setTempRet0(uint32_t value);
extern void setThrew(uintptr_t threw, int value);

TableEntry* saveSetjmp(uintptr_t* env, uint32_t label, TableEntry* table, uint32_t size) {
  // Not particularly fast: slow table lookup of setjmpId to label. But setjmp
  // prevents relooping anyhow, so slowness is to be expected. And typical case
  // is 1 setjmp per invocation, or less.
  uint32_t i = 0;
  setjmpId++;
  *env = setjmpId;
  while (i < size) {
    if (table[i].id == 0) {
      table[i].id = setjmpId;
      table[i].label = label;
      // prepare next slot
      table[i + 1].id = 0;
      setTempRet0(size);
      return table;
    }
    i++;
  }
  // grow the table
  size *= 2;
  table = (TableEntry*)realloc(table, sizeof(TableEntry) * (size +1));
  table = saveSetjmp(env, label, table, size);
  setTempRet0(size); // FIXME: unneeded?
  return table;
}

uint32_t testSetjmp(uintptr_t id, TableEntry* table, uint32_t size) {
  uint32_t i = 0;
  while (i < size) {
    uintptr_t curr = table[i].id;
    if (curr == 0) break;
    if (curr == id) {
      return table[i].label;
    }
    i++;
  }
  return 0;
}

#if !defined(__USING_WASM_SJLJ__)

#include "emscripten_internal.h"

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

#ifdef __USING_WASM_SJLJ__
struct __WasmLongjmpArgs {
  void *env;
  int val;
};

// llvm uses `1` for the __c_longjmp tag.
// See https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/CodeGen/WasmEHFuncInfo.h
#define C_LONGJMP 1
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
#endif
