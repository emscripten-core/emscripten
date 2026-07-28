/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Binding an AF_UNIX socket to a path that is already bound must fail
 * synchronously with EADDRINUSE, exactly as POSIX bind() does. Self-contained
 * and also runs natively.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static const char* SOCK_PATH = "/tmp/emscripten_unix_inuse.sock";

int main(void) {
  unlink(SOCK_PATH);

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SOCK_PATH);

  int a = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(a >= 0);
  assert(bind(a, (struct sockaddr*)&addr, sizeof(addr)) == 0 && "first bind");

  int b = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(b >= 0);
  int r = bind(b, (struct sockaddr*)&addr, sizeof(addr));
  printf("second bind returned %d, errno %d (%s)\n", r, errno, strerror(errno));
  assert(r == -1 && errno == EADDRINUSE && "expected EADDRINUSE");

  close(a);
  close(b);
  unlink(SOCK_PATH);
  printf("done\n");
  return 0;
}
