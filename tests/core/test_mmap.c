/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  // Alignment check for mmap below should be consistent with the reported page size
  printf("getpagesize(): %d\n", getpagesize());
  printf("sysconf(_SC_PAGESIZE): %ld\n", sysconf(_SC_PAGESIZE));
  assert(getpagesize() == 65536);
  assert(sysconf(_SC_PAGESIZE) == 65536);

  int* maps[10];
  for (int i = 0; i < 10; i++) {
    int* map = (int*)mmap(0, 5000, PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANON, -1, 0);
    assert(map != MAP_FAILED);
    assert(((long)map) % 65536 == 0); // aligned
    maps[i] = map;
  }

  for (int i = 0; i < 10; i++) {
    assert(munmap(maps[i], 5000) == 0);
  }

  const int NUM_BYTES = 8 * 1024 * 1024;
  const int NUM_INTS = NUM_BYTES / sizeof(int);

  int* map = (int*)mmap(0, NUM_BYTES, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANON, -1, 0);
  assert(map != MAP_FAILED);

  for (int i = 0; i < NUM_INTS; i++) {
    map[i] = i;
  }

  for (int i = 0; i < NUM_INTS; i++) {
    assert(map[i] == i);
  }

  // Emscripten does not support partial unmapping
  int rtn = munmap(map, 65536);
  assert(rtn == -1);
  assert(errno == EINVAL);

  assert(munmap(map, NUM_BYTES) == 0);

  printf("hello,world\n");
  return 0;
}
