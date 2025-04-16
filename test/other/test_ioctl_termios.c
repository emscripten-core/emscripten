/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

int main() {
  struct termios t;
  struct winsize ws;

  assert(ioctl(STDOUT_FILENO, TCGETS, &t) == 0);

  assert(t.c_iflag == (ICRNL | IXON | IMAXBEL | IUTF8));
  assert(t.c_oflag == (OPOST | ONLCR));
  assert(t.c_cflag == (B38400 | CSIZE | CREAD));
  assert(t.c_lflag == (ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN));

  assert(ioctl(STDOUT_FILENO, TCSETS, &t) == 0);

  assert(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0);
  assert(ws.ws_col == 80);
  assert(ws.ws_row == 24);

  puts("success");
  return 0;
}
