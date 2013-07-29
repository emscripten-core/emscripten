#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("file.txt", "cd", 0666);
}

void cleanup() {
  unlink("file.txt");
}

void test() {
  FILE *file;
  int err;
  char buffer[256];

  file = fopen("file.txt", "r");
  assert(file);

  // pushing EOF always returns EOF
  rewind(file);
  err = ungetc(EOF, file);
  assert(err == EOF);

  // ungetc should return itself
  err = ungetc('a', file);
  assert(err == (int)'a');

  // push two chars and make sure they're read back in
  // the correct order (both by fgetc and fread)
  rewind(file);
  ungetc('b', file);
  ungetc('a', file);
  err = fgetc(file);
  assert(err == (int)'a');
  fread(buffer, sizeof(char), sizeof(buffer), file);
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
  assert(!err);

  fclose(file);

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}