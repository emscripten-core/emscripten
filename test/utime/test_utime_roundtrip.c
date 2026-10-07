/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Reading a file's timestamps with stat() and writing them back with
// utimensat() must produce the same timestamps when read back again. Run in a
// loop to exercise different rounding cases.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define ITERATIONS 2000

static int same_timespec(const struct timespec* a, const struct timespec* b) {
  return a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec;
}

static void print_timespec(const char* label, const struct timespec* ts) {
  printf("%s=%lld.%09ld", label, (long long)ts->tv_sec, ts->tv_nsec);
}

int main() {
  const char* path = "roundtripfile";
  for (int i = 0; i < ITERATIONS; i++) {
    // Create the file. Filesystem assigns mtime.
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
    assert(fd >= 0);
    close(fd);

    struct stat before;
    assert(stat(path, &before) == 0);

    // Write the timestamps back exactly as we read them.
    struct timespec times[2] = {before.st_atim, before.st_mtim};
    assert(utimensat(AT_FDCWD, path, times, 0) == 0);


    // Writing back same timestamp shouldn't have changed it.
    struct stat after;
    assert(stat(path, &after) == 0);

    if (!same_timespec(&before.st_atim, &after.st_atim) ||
        !same_timespec(&before.st_mtim, &after.st_mtim)) {
      printf("iteration %d: timestamps did not round-trip: ", i);
      print_timespec("atime before", &before.st_atim);
      print_timespec(" after", &after.st_atim);
      print_timespec(" mtime before", &before.st_mtim);
      print_timespec(" after", &after.st_mtim);
      printf("\n");
      return 1;
    }

    assert(unlink(path) == 0);
  }

  puts("done");
  return 0;
}
