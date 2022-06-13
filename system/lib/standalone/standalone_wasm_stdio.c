/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <stdio.h>

void emscripten_console_log(const char *utf8String) {
  puts(utf8String);
}

void emscripten_console_warn(const char *utf8String) {
  fprintf(stderr, "%s\n", utf8String);
}

void emscripten_console_error(const char *utf8String) {
  emscripten_console_warn(utf8String);
}
