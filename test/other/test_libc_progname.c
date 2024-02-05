/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>

extern const char *__progname;
extern const char *__progname_full;

int main(void) {
  printf("__progname: %s\n", __progname);

  /* Ensure the pointers aren't NULL. */
  assert(__progname && __progname_full);

  /* Ensure the basename is contained in the full path. */
  assert(strstr(__progname_full, __progname));

  /* Ensure the basename contains no path separator. */
  assert(!strchr(__progname, '/'));

  if (EM_ASM_INT({ return process.platform.startsWith("win") })) {
    // The rest of the test here assumes unix-style pathnames.
    return 0;
  }

  /* Ensure the full path starts with the root directory. */
  assert(*__progname_full == '/');

  /* Ensure the full path is a file and not a directory. */
  assert(__progname_full[strlen(__progname_full)-1] != '/');

  return 0;
}
