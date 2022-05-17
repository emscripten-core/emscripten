/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/wasmfs.h>
#include <stdio.h>

void wasmfs_before_preload(void) {
  puts("waka");
}

int main() {
}
