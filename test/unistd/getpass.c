/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  char* pass = getpass("Password: ");
  assert(pass != NULL);
  assert(strcmp(pass, "secret") == 0);
  puts("done");
  return 0;
}
