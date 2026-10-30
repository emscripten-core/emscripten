/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wasi/api.h>

#define CAPACITY 65536
#define TOTAL (1 << 20)
#define BIG (1 << 18)
#define CHUNK 16384

int p[2];
_Atomic size_t consumed;

void consume(const unsigned char* buf, ssize_t n) {
  for (ssize_t i = 0; i < n; i++)
    assert(buf[i] == ((consumed + i) & 0xff));
  consumed += n;
}

void fill(unsigned char* buf, size_t off, size_t n) {
  for (size_t i = 0; i < n; i++)
    buf[i] = (off + i) & 0xff;
}

void write_all(void) {
  unsigned char* buf = malloc(BIG);
  fill(buf, 0, BIG);
  assert(write(p[1], buf, BIG) == BIG);
  for (size_t written = BIG; written < TOTAL; written += 4096) {
    fill(buf, written, 4096);
    assert(write(p[1], buf, 4096) == 4096);
  }
  free(buf);
}

void* reader(void* arg) {
  unsigned char buf[CHUNK];
  while (consumed < TOTAL) {
    ssize_t n = read(p[0], buf, sizeof buf);
    if (n < 0) {
      assert(errno == EAGAIN);
      struct pollfd pfd = {p[0], POLLIN, 0};
      assert(poll(&pfd, 1, -1) == 1);
      continue;
    }
    consume(buf, n);
  }
  return NULL;
}

void* closer(void* arg) {
  usleep(50 * 1000);
  assert(close(p[0]) == 0);
  return NULL;
}

// Runs on the main thread, re-entered from a pthread's proxied call: its
// write is the main thread's, unbounded and not a pthread's wait. Calls the
// import directly so a count it fails to store reads as 0.
EMSCRIPTEN_KEEPALIVE int main_writer(int n) {
  unsigned char* buf = malloc(n);
  fill(buf, 0, n);
  __wasi_ciovec_t iov = {buf, n};
  __wasi_size_t num = 0;
  assert(__wasi_fd_write(p[1], &iov, 1, &num) == 0);
  free(buf);
  return num;
}

void read_back(size_t n) {
  unsigned char buf[CHUNK];
  consumed = 0;
  while (consumed < n) {
    ssize_t got = read(p[0], buf, sizeof buf);
    if (got < 0) {
      assert(errno == EAGAIN);
      usleep(1000);
      continue;
    }
    consume(buf, got);
  }
}

void* big_writer(void* arg) {
  unsigned char* buf = malloc(BIG);
  fill(buf, 0, BIG);
  assert(write(p[1], buf, BIG) == BIG);
  // Held to the capacity: the write completes only once the reader has taken
  // the rest, whichever thread reads. It completes inside the read that frees
  // the last room, before that chunk is counted.
  assert(consumed >= BIG - CAPACITY - CHUNK);
  free(buf);
  return NULL;
}

void* run(void* arg) {
  assert(pipe(p) == 0);
  unsigned char* big = malloc(BIG);
  memset(big, 'x', BIG);
  pthread_t t;
  assert(pthread_create(&t, NULL, closer, NULL) == 0);
  assert(write(p[1], big, BIG) == CAPACITY);
  assert(write(p[1], big, 1) == -1 && errno == EPIPE);
  free(big);
  assert(pthread_join(t, NULL) == 0);
  assert(close(p[1]) == 0);

  assert(pipe(p) == 0);
  assert(pthread_create(&t, NULL, reader, NULL) == 0);
  write_all();
  assert(close(p[1]) == 0);
  assert(pthread_join(t, NULL) == 0);
  assert(consumed == TOTAL);
  assert(close(p[0]) == 0);

  assert(pipe(p) == 0);
  assert(MAIN_THREAD_EM_ASM_INT({ return _main_writer($0); }, BIG) == BIG);
  read_back(BIG);
  assert(close(p[0]) == 0);
  assert(close(p[1]) == 0);
  return NULL;
}

// The writer is a pthread either way; without PROXY_TO_PTHREAD main blocks in
// pthread_join, servicing proxied calls but never its event loop.
int main() {
  pthread_t t;
  assert(pthread_create(&t, NULL, run, NULL) == 0);
  assert(pthread_join(t, NULL) == 0);

  assert(pipe(p) == 0);
  assert(pthread_create(&t, NULL, big_writer, NULL) == 0);
  read_back(BIG);
  assert(pthread_join(t, NULL) == 0);
  assert(close(p[0]) == 0);
  assert(close(p[1]) == 0);
  printf("done\n");
  return 0;
}
