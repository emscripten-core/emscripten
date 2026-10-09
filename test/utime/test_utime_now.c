/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Creating a file and then touching it must never move the mtime backwards. 
// Run many times in a loop to prevent flakiness.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int cmp_timespec(const struct timespec* a, const struct timespec* b) {
  if (a->tv_sec != b->tv_sec) {
    return a->tv_sec < b->tv_sec ? -1 : 1;
  }
  if (a->tv_nsec != b->tv_nsec) {
    return a->tv_nsec < b->tv_nsec ? -1 : 1;
  }
  return 0;
}

int main() {
  const char* path = "touchfile";
  for (int i = 0; i < 1000; i++) {
    // Create the file. Filesystem assigns initial mtime.
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
    assert(fd >= 0);
    close(fd);

    // stat 1
    struct stat before;
    assert(stat(path, &before) == 0);

    // touch
    struct timespec now[2];
    now[0].tv_nsec = UTIME_NOW;
    now[1].tv_nsec = UTIME_NOW;
    assert(utimensat(AT_FDCWD, path, now, 0) == 0);

    // stat 2
    struct stat after;
    assert(stat(path, &after) == 0);

    // assert stat 2 >= stat 1
    if (cmp_timespec(&after.st_mtim, &before.st_mtim) < 0) {
      printf("iteration %d: mtime went backwards: before=%lld.%09ld after=%lld.%09ld\n",
             i, (long long)before.st_mtim.tv_sec, before.st_mtim.tv_nsec,
             (long long)after.st_mtim.tv_sec, after.st_mtim.tv_nsec);
      return 1;
    }

    assert(unlink(path) == 0);
  }

  puts("done");
  return 0;
}
