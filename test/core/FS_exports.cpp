// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main() {
#ifdef USE_FILES
  if (fopen("nonexistend", "r")) {
    puts("that was bad");
    return 1;
  }
#endif
#ifdef DIRECT
  EM_ASM({
    FS.createDataFile("/", "file.txt", [1, 2, 3]);
  });
#else
  EM_ASM({
    Module["FS_createDataFile"]("/", "file.txt", [1, 2, 3]);
  });
#endif
  EM_ASM({
    // use eval, so that the compiler can't see FS usage statically
    eval('out("Data: " + JSON.stringify(Array.from(MEMFS.getFileDataAsTypedArray(FS.root.contents["file.txt"]))))');
  });
}

