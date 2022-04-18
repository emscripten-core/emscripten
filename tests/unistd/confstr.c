/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  int vals[] = {
    _CS_PATH,
    _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS,
    _CS_GNU_LIBC_VERSION,
    _CS_GNU_LIBPTHREAD_VERSION,
    _CS_POSIX_V6_ILP32_OFF32_LIBS,
    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,
    _CS_POSIX_V6_LP64_OFF64_CFLAGS,
    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,
    _CS_POSIX_V6_LP64_OFF64_LIBS,
    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,
    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,
    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,
    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,
    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,
    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,
    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS
  };
  const char* names[] = {
    "_CS_PATH",
    "_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS",
    "_CS_GNU_LIBC_VERSION",
    "_CS_GNU_LIBPTHREAD_VERSION",
    "_CS_POSIX_V6_ILP32_OFF32_LIBS",
    "_CS_POSIX_V6_ILP32_OFFBIG_LIBS",
    "_CS_POSIX_V6_LP64_OFF64_CFLAGS",
    "_CS_POSIX_V6_LP64_OFF64_LDFLAGS",
    "_CS_POSIX_V6_LP64_OFF64_LIBS",
    "_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS",
    "_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS",
    "_CS_POSIX_V6_LPBIG_OFFBIG_LIBS",
    "_CS_POSIX_V6_ILP32_OFF32_CFLAGS",
    "_CS_POSIX_V6_ILP32_OFF32_LDFLAGS",
    "_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS",
    "_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS"
  };
  char buffer[256];

  for (int i = 0; i < sizeof vals / sizeof vals[0]; i++) {
    printf("ret: %zi\n", confstr(vals[i], buffer, 256));
    printf("%s: %s\n", names[i], buffer);
    printf("errno: %d\n\n", errno);
    errno = 0;
  }

  printf("(invalid) ret: %zi\n", confstr(-123, buffer, 256));
  printf("errno: %d\n", errno);

  return 0;
}
