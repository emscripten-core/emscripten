/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

mode_t get_umask() {
  mode_t current = umask(0);  // Set umask to 0 and get the old value
  umask(current);  // Immediately set it back
  return current;
}

void create_file(const char *path, const char *buffer) {
  mode_t mode = 0777 - get_umask();
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

int main() {
  // Get the default umask
  mode_t default_umask = get_umask();
  printf("default umask: %o\n", default_umask);
  assert(default_umask == 022);

  // Create a new file with default umask
  create_file("umask_test_file", "abcdef");
  struct stat st;
  stat("umask_test_file", &st);
  printf("default_umask - stat: %o\n", st.st_mode);
  assert((st.st_mode & 0666) == 0644);
  unlink("umask_test_file");

  // Set new umask
  mode_t new_umask = 027;
  mode_t old_umask = umask(new_umask);

  // Create a new file with new umask
  create_file("umask_test_file", "abcdef");
  stat("umask_test_file", &st);
  printf("new_umask - stat: %o\n", st.st_mode);
  assert((st.st_mode & 0666) == 0640);

  // Restore the old umask
  umask(old_umask);
  
  puts("success");
  return EXIT_SUCCESS;
}
