/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int line = 0;

void main_loop() {
  char str[10] = {0};
  int ret;

  errno = 0;
  while (errno != EAGAIN) {
    if (line == 0) {
      ret = fgetc(stdin);
      if (ret != EOF) putc(ret, stdout);
      if (ret == '\n') line++;
    } else if (line > 0) {
      ret = scanf("%9s", str);
      if (ret > 0) puts(str);
    }

    int err = ferror(stdin);
    if (err && errno != EAGAIN) {
      printf("error %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (feof(stdin)) {
      puts("eof");
      exit(EXIT_SUCCESS);
    }

    clearerr(stdin);
  }
}

int main(int argc, char const *argv[]) {
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

  // SM shell doesn't implement an event loop and therefor doesn't support
  // emscripten_set_main_loop. However, its stdin reads are sync so it
  // should exit out after calling main_loop once.
  main_loop();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 60, 0);
#else
  while (1) main_loop(); sleep(1);
#endif
  return 0;
}
