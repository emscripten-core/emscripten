/**
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t started = PTHREAD_COND_INITIALIZER;
pthread_cond_t unblock = PTHREAD_COND_INITIALIZER;
pthread_cond_t pending = PTHREAD_COND_INITIALIZER;
_Atomic bool got_sigterm = false;
_Atomic bool is_pending = false;
_Atomic bool unblock_signal = false;

pthread_t child_thread;

void signal_handler(int sig, siginfo_t * info, void * arg) {
  printf("signal_handler: sig=%d onthread=%d\n", sig, pthread_self() == child_thread);
  assert(sig == SIGTERM);
  assert(pthread_self() == child_thread);
  got_sigterm = true;
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
  sigset_t set;
  int ret;

  // First we block all signals.
  sigfillset(&set);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  // Now we signal that we are running
  pthread_mutex_lock(&lock);
  pthread_cond_signal(&started);
  pthread_mutex_unlock(&lock);

  printf("Waiting for SIGTERM to becoming pending\n");

  // Now loop until we see that SIGTERM is pending.
  while (1) {
    sigpending(&set);
    if (sigismember(&set, SIGTERM)) {
      printf("SIGTERM is now pending\n");
      pthread_mutex_lock(&lock);
      is_pending = true;
      pthread_cond_signal(&pending);
      pthread_mutex_unlock(&lock);
      break;
    }
    sleepms(1);
  }

  assert(!got_sigterm);

  pthread_mutex_lock(&lock);
  if (!unblock_signal) {
    pthread_cond_wait(&unblock, &lock);
  }
  pthread_mutex_unlock(&lock);

  // Now unlock all signals and we should receive SIGTERM here.
  printf("Unblocking signals\n");
  sigfillset(&set);
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);

  assert(got_sigterm);

  return NULL;
}

int main() {
  int ret;
  setup_handler();

  pthread_mutex_lock(&lock);
  ret = pthread_create(&child_thread, NULL, thread_start, 0);
  assert(ret == 0);

  // Wait until thread kicks in and sets the shared variable.
  pthread_cond_wait(&started, &lock);
  pthread_mutex_unlock(&lock);
  printf("thread has started, sending SIGTERM\n");

  ret = pthread_kill(child_thread, SIGTERM);
  assert(ret == 0);
  printf("SIGTERM sent\n");

  pthread_mutex_lock(&lock);
  if (!is_pending) {
    pthread_cond_wait(&pending, &lock);
  }

  // Now that the signal is pending on the child thread, we block and unblock
  // all signals on the mains thread, which should be a no-op.
  // We had a bug where pending signals were not stored in TLS which would
  // cause this to raise the pending signals erroneously here.
  sigset_t set;
  sigfillset(&set);
  pthread_sigmask(SIG_BLOCK, &set, NULL);
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);

  // Signal the child thread to unlock and receive the signal
  unblock_signal = true;
  pthread_cond_signal(&unblock);
  pthread_mutex_unlock(&lock);

  pthread_join(child_thread, NULL);
  printf("done\n");
  return 0;
}
