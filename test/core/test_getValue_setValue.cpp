// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

int main() {
  char buffer_char[8] = { 'x' };
  int buffer_ptr[] = { 0x12345678, 0x77665544 };
  EM_ASM({
#ifdef DIRECT
    out('i32: ' + getValue($0, 'i32'));
    setValue($0, 1234, 'i32');
    out('i32: ' + getValue($0, 'i32'));
#ifdef WASM_BIGINT
    i64 = getValue($1, 'i64');
    out('i64: 0x' + i64.toString(16) + ' ' + typeof(i64));
#endif
    ptr = getValue($1, '*');
    out('ptr: 0x' + ptr.toString(16) + ' ' + typeof(ptr));
#else
    out('i32: ' + getValue($0, 'i32'));
    Module['setValue']($0, 1234, 'i32');
    out('i32: ' + Module['getValue']($0, 'i32'));
#ifdef WASM_BIGINT
    i64 = Module['getValue']($1, 'i64');
    out('i64: 0x' + i64.toString(16) + ' ' + typeof(i64));
#endif
    ptr = Module['getValue']($1, '*');
    out('ptr: 0x' + ptr.toString(16) + ' ' + typeof(ptr));
#endif
  }, buffer_char, buffer_ptr);
}

