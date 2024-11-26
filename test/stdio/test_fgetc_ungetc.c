/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("/tmp/file.txt", "cd", 0666);
}

void test() {
  FILE *file;
  int err;
  char buffer[256];

  file = fopen("/tmp/file.txt", "r");
  assert(file);

  // pushing EOF always returns EOF
  rewind(file);
  err = ungetc(EOF, file);
  assert(err == EOF);

  // ungetc should return itself
  err = ungetc('a', file);
  assert(err == (int)'a');

  // fgetc should get it (note that we cannot push more than 1, as there is no portability guarantee for that)
  err = fgetc(file);
  assert(err == (int)'a');

  // fread should get it first
  ungetc('b', file);
  int r = fread(buffer, sizeof(char), sizeof(buffer), file);
  assert(r == 3);
  buffer[3] = 0;
  printf("buffer: %s\n", buffer);
  assert(!strcmp(buffer, "bcd"));

  // rewind and fseek should reset anything that's been
  // pushed to the stream
  ungetc('a', file);
  rewind(file);
  err = fgetc(file);
  assert(err == (int)'c');
  ungetc('a', file);
  fseek(file, 0, SEEK_SET);
  err = fgetc(file);
  assert(err == (int)'c');

  // fgetc, when nothing is left, should return EOF
  fseek(file, 0, SEEK_END);
  err = fgetc(file);
  assert(err == EOF);
  err = feof(file);
  assert(err);

  // ungetc should reset the EOF indicator
  ungetc('e', file);
  err = feof(file);
  // XXX musl fails here. it does not allow ungetc on a stream in EOF mode, which has been confirmed as a bug upstream

  fclose(file);

  puts("success");
}

int main() {
#ifdef __EMSCRIPTEN__
#ifdef NODEFS
  EM_ASM(FS.mount(NODEFS, { root: '.' }, '/tmp'));
#elif MEMFS
  EM_ASM(FS.mount(MEMFS, {}, '/tmp'));
#endif
#endif
  setup();
  test();
  return 0;
}
