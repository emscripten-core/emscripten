// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

_Atomic int sharedVar = 0;

static void *thread_start(void *arg) {
  // As long as this thread is running, keep the shared variable latched to nonzero value.
  for (;;) {
    ++sharedVar;
  }

  pthread_exit(0);
}

void BusySleep(long msecs) {
  usleep(msecs * 1000);
}

int main() {
  pthread_t thr;

  sharedVar = 0;
  int s = pthread_create(&thr, NULL, thread_start, 0);
  assert(s == 0);

  // Wait until thread kicks in and sets the shared variable.
  while (sharedVar == 0)
    BusySleep(10);
  printf("thread startd\n");

  s = pthread_kill(thr, SIGKILL);
  printf("thread killed\n");
  assert(s == 0);

  // Wait until we see the shared variable stop incrementing. (This is a bit heuristic and hacky)
  for(;;) {
    int val = sharedVar;
    BusySleep(100);
    int val2 = sharedVar;
    if (val == val2) break;
  }

  // Reset to 0.
  sharedVar = 0;

  // Wait for a long time, if the thread is still running, it should progress and set sharedVar by this time.
  BusySleep(1000);

  // Finally test that the thread is not doing any work and it is dead.
  assert(sharedVar == 0);
  printf("Main: Done. Successfully killed thread. sharedVar: %d\n", sharedVar);
  return 0;
}
