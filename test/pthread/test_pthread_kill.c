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
_Atomic int gotTermSignal = 0;

pthread_t thr;

void signal_handler(int sig, siginfo_t * info, void * arg) {
  printf("signal: %d onthread=%d\n", sig, pthread_self() == thr);
  if (sig == SIGTERM) {
    gotTermSignal = 1;
  }
}

void setup_handler() {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = signal_handler;
  sigaction(SIGTERM, &act, NULL);
}


void sleepms(long msecs) {
  usleep(msecs * 1000);
}

void *thread_start(void *arg) {
  // As long as this thread is running, keep the shared variable latched to nonzero value.
  while (!gotTermSignal) {
    sleepms(1);
    ++sharedVar;
  }
  printf("got term signal, shutting down thread\n");
  pthread_exit(0);
}

int main() {
  setup_handler();

  sharedVar = 0;
  int s = pthread_create(&thr, NULL, thread_start, 0);
  assert(s == 0);

  // Wait until thread kicks in and sets the shared variable.
  while (sharedVar == 0) {
    sleepms(10);
  }
  printf("thread has started, sending SIGTERM\n");

  s = pthread_kill(thr, SIGTERM);
  printf("SIGTERM sent\n");

  assert(s == 0);

  pthread_join(thr, NULL);
  return 0;
}
