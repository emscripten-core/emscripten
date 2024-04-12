// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE // for sighandler_t
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

bool recieved1 = false;
bool recieved2 = false;

void block_sigusr1() {
  sigset_t old;
  sigset_t set;
  sigaddset(&set, SIGUSR1);
  int rc = sigprocmask(SIG_BLOCK, &set, &old);
  assert(rc == 0);
  assert(!sigismember(&old, SIGUSR1));
}

void unblock_all() {
  sigset_t old;
  sigset_t set;
  sigfillset(&set);
  int rc = sigprocmask(SIG_UNBLOCK, &set, &old);
  assert(rc == 0);
}

void handler1(int sig) {
  printf("handler1: %d\n", sig);
  assert(sig == SIGUSR1);
  recieved1 = true;
}

void test_bad_signal() {
  printf("test_bad_signal\n");
  sighandler_t old = signal(1024, handler1);
  assert(old == SIG_ERR);
}

void test_raise_sigusr1() {
  printf("test_raise_sigusr1\n");
  sighandler_t old = signal(SIGUSR1, handler1);
  assert(old != SIG_ERR);

  raise(SIGUSR1);

  int count = 0;
  while (!recieved1) {
    usleep(1000);
    if (count++ == 1000) {
      printf("handler1 not run\n");
      assert(false);
    }
  }
}

void test_sigpenging() {
  printf("test_sigpending\n");
  recieved1 = false;

  // Test that the pending set is empty
  sigset_t pending;
  sigpending(&pending);
  assert(!sigismember(&pending, SIGUSR1));

  block_sigusr1();

  sighandler_t old_handler = signal(SIGUSR1, handler1);
  assert(old_handler != SIG_ERR);

  raise(SIGUSR1);

  while (1) {
    // Check that SIGUSR1 is now pending.
    sigpending(&pending);
    if (sigismember(&pending, SIGUSR1)) {
      printf("is now pending\n");
      break;
    }
    printf("is not pending\n");
  }

  // Unlock the signal and then check that is recieved.
  assert(!recieved1);
  unblock_all();

  int count = 0;
  while (!recieved1) {
    usleep(1000);
    if (count++ == 1000) {
      printf("handler1 not run\n");
      assert(false);
    }
  }

  // Signal should no longer be pending
  sigpending(&pending);
  assert(!sigismember(&pending, SIGUSR1));
}

void test_sigwaitinfo() {
  recieved1 = false;
  block_sigusr1();

  raise(SIGUSR1);

  sighandler_t old_handler = signal(SIGUSR1, handler1);
  assert(old_handler != SIG_ERR);

  sigset_t set;
  siginfo_t info;
  sigaddset(&set, SIGUSR1);
  int rc = sigwaitinfo(&set, &info);
  assert(rc == SIGUSR1);

  // Signal should no longer be pending
  sigset_t pending;
  sigpending(&pending);
  assert(!sigismember(&pending, SIGUSR1));

  unblock_all();
  assert(!recieved1);
}

void test_sigaction() {
  // Use sigaction to find the existing handlers
  struct sigaction action;
  sigaction(SIGUSR1, NULL, &action);
  assert(action.sa_handler == SIG_DFL);
  assert((void (*)(int))action.sa_sigaction == SIG_DFL);

  // Now install a new handler
  action.sa_handler = handler1;
  sigaction(SIGUSR1, &action, NULL);

  // Verify that the new handler is returned
  struct sigaction action2;
  sigaction(SIGUSR1, NULL, &action2);
  assert(action2.sa_handler == handler1);
}

void test_sigemptyset() {
  sigset_t s = { 0 };
  assert(sigisemptyset(&s));
  sigaddset(&s, SIGUSR1);
  assert(!sigisemptyset(&s));
  sigemptyset(&s);
  assert(sigisemptyset(&s));
}

int main() {
  test_sigaction();
  test_bad_signal();
  test_raise_sigusr1();
  test_sigpenging();
  test_sigwaitinfo();
  test_sigemptyset();
  return 0;
}
