// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdint.h>

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

    /* In ASSERTIONS=2 mode we check the limits of input values */
    var min = $2;
    var max = $3;
    setValue($1, max, 'i8');
    setValue($1, min, 'i8');
#ifdef ASSERTIONS_2
    function checkAsserts(f, expected) {
      try {
        f();
      } catch(e) {
        assert(e.message.includes(expected), "expected assertion to include:" + expected);
        return;
      }
      assert(false, "expected assertion");
    }
    checkAsserts(() => setValue($1, max + 1, 'i8'), "value (256) too large to write as 8-bit value");
    checkAsserts(() => setValue($1, min - 1, 'i8'), "value (-129) too small to write as 8-bit value");
#else
    setValue($1, max + 1, 'i8');
    setValue($1, min - 1, 'i8');
#endif
  }, buffer_char, buffer_ptr, INT8_MIN, UINT8_MAX);
}

