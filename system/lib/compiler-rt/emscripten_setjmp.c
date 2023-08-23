/*
* Copyright 2020 The Emscripten Authors.  All rights reserved.
* Emscripten is available under two separate licenses, the MIT license and the
* University of Illinois/NCSA Open Source License.  Both these licenses can be
* found in the LICENSE file.
*/

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
  setThrew(env, val);
  _emscripten_throw_longjmp();
}
#endif

#ifdef __USING_WASM_SJLJ__

struct __WasmLongjmpArgs {
  void *env;
  int val;
};

thread_local struct __WasmLongjmpArgs __wasm_longjmp_args;

// Wasm EH allows us to throw and catch multiple values, but that requires
// multivalue support in the toolchain, whch is not reliable at the time.
// TODO Consider switching to throwing two values at the same time later.
void __wasm_longjmp(void *env, int val) {
  __wasm_longjmp_args.env = env;
  __wasm_longjmp_args.val = val;
  __builtin_wasm_throw(1, &__wasm_longjmp_args);
}

#endif
