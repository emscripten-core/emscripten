/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
	create_file("test.txt", "abcdefg", 0666);
}

void cleanup() {
	unlink("test.txt");
}

void test() {
	struct termios tc;
	int ret;
	int fd;

	fd = open("test.txt", O_RDONLY);

	ret = tcgetattr(fd, &tc);
	assert(ret == -1);
	assert(errno = ENOTTY);

	ret = tcgetattr(STDIN_FILENO, &tc);
	assert(!ret);

	ret = tcsetattr(fd, 0, &tc);
	assert(ret == -1);
	assert(errno = ENOTTY);

	ret = tcsetattr(STDIN_FILENO, 0, &tc);
	assert(!ret);

	close(fd);

	puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
