/*
* Copyright 2020 The Emscripten Authors.  All rights reserved.
* Emscripten is available under two separate licenses, the MIT license and the
* University of Illinois/NCSA Open Source License.  Both these licenses can be
* found in the LICENSE file.
*/

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>

static uint32_t setjmpId = 0;

typedef struct TableEntry {
  uint32_t id, label;
} TableEntry;

extern void setTempRet0(uint32_t value);

TableEntry* saveSetjmp(uint32_t* env, uint32_t label, TableEntry* table, uint32_t size) {
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

uint32_t testSetjmp(uint32_t id, TableEntry* table, uint32_t size) {
  uint32_t i = 0, curr;
  while (i < size) {
    uint32_t curr = table[i].id;
    if (curr == 0) break;
    if (curr == id) {
      return table[i].label;
    }
    i++;
  }
  return 0;
}
