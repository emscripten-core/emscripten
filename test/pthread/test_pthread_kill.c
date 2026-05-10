// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <emscripten/console.h>

pthread_barrier_t started;
_Atomic bool got_sigterm = false;
_Atomic bool got_sigusr1 = false;

pthread_t main_thread;
pthread_t child_thread;

void signal_handler(int sig, siginfo_t * info, void * arg) {
  printf("signal: %d onthread=%d\n", sig, pthread_self() == child_thread);
  if (sig == SIGTERM) {
    got_sigterm = true;
  } else if (sig == SIGUSR1) {
    got_sigusr1 = true;
  }
}

void setup_handler() {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = signal_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

void sleepms(long msecs) {
  usleep(msecs * 1000);
}

void *thread_start(void *arg) {
  pthread_barrier_wait(&started);
  // As long as this thread is running, keep the shared variable latched to nonzero value.
  while (!got_sigterm) {
    sleepms(1);
  }
  printf("got term signal, sending signal back to main thread\n");
  pthread_kill(main_thread, SIGUSR1);
  return NULL;
}

int main() {
  main_thread = pthread_self();
  setup_handler();

  printf("tesing pthread_kill with pthread_self\n");
  assert(!got_sigterm);
  int s = pthread_kill(pthread_self(), SIGTERM);
  assert(got_sigterm);
  got_sigterm = false;
  assert(s == 0);

  pthread_barrier_init(&started, NULL, 2);
  s = pthread_create(&child_thread, NULL, thread_start, 0);
  assert(s == 0);

  // Wait until thread kicks in and sets the shared variable.
  pthread_barrier_wait(&started);
  printf("thread has started, sending SIGTERM\n");

  s = pthread_kill(child_thread, SIGTERM);
  assert(s == 0);
  printf("SIGTERM sent\n");

  pthread_join(child_thread, NULL);
  printf("joined child_thread\n");
  while (!got_sigusr1) {
    sleepms(1);
  }
  printf("got SIGUSR1. all done.\n");
  return 0;
}
