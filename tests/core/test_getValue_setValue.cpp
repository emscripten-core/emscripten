// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include<emscripten.h>

int main() {
  char buffer_char[8] = { 'x' };
  void* buffer_ptr[] = { (void*)0x12345678 };
#ifdef DIRECT
  EM_ASM({
    out('i32: ' + getValue($0, 'i32'));
    setValue($0, 1234, 'i32');
    out('i32: ' + getValue($0, 'i32'));
    out('ptr: 0x' + getValue($1, '*').toString(16));
  }, buffer_char, buffer_ptr);
#else
  EM_ASM({
    out('i32: ' + getValue($0, 'i32'));
    Module['setValue']($0, 1234, 'i32');
    out('i32: ' + Module['getValue']($0, 'i32'));
    out('ptr: 0x' + Module['getValue']($1, 'i32').toString(16));
  }, buffer_char, buffer_ptr);
#endif
}

