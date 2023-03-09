/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <stdio.h>


int main() {
  EM_ASM(
    FS.writeFile('testfile', 'a=1\nb=2\n'); // writeFile already works with WasmFS
    var readStream = FS.open('testfile', 'r');
    assert(readStream >= 0);

    var writeStream = FS.open('testfile', 'w');
    assert(writeStream >= 0);

    var writePlusStream = FS.open('testfile', 'w+');
    assert(writePlusStream >= 0);

    var appendStream = FS.open('testfile', 'a');
    assert(appendStream >= 0);

    var notFound = FS.open('filenothere', 'r');
    assert(notFound < 0);

    var createFileNotHere = FS.open('filenothere', 'w+');
    assert(createFileNotHere >= 0);
  );
  puts("success");
  

  return 0;
}
