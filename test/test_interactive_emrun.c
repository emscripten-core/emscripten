/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  fd_set readfds;
  char buf[124];
  ssize_t n;
  int nr = 0;

  while (nr < sizeof(buf)) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    assert(select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) == 1);
    assert(FD_ISSET(STDIN_FILENO, &readfds));
    n = read(STDIN_FILENO, &(buf[nr]), sizeof(buf) - nr);
    assert(n >= 0);
    if (n == 0) break;
    nr += n;
  }

  printf("%s\n", buf);

  exit(0);
}
