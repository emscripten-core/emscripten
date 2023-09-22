/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE // for sighandler_t
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>

#include "emscripten_internal.h"

extern struct sigaction __sig_actions[_NSIG];
extern sigset_t __sig_pending;

bool __sig_is_blocked(int sig);

// Default handler actions ~auto-generated from https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html.
// Note that stop and continue actions are not supported and treated as ignored.

typedef enum {
  ACTION_IGNORE = 0,
  ACTION_TERMINATE,
  ACTION_ABORT,
} default_action_t;

static default_action_t default_actions[_NSIG] = {
  [SIGABRT] = ACTION_ABORT,
  [SIGALRM] = ACTION_TERMINATE,
  [SIGBUS] = ACTION_ABORT,
  [SIGFPE] = ACTION_ABORT,
  [SIGHUP] = ACTION_TERMINATE,
  [SIGILL] = ACTION_ABORT,
  [SIGINT] = ACTION_TERMINATE,
  [SIGKILL] = ACTION_TERMINATE,
  [SIGPIPE] = ACTION_TERMINATE,
  [SIGQUIT] = ACTION_ABORT,
  [SIGSEGV] = ACTION_ABORT,
  [SIGTERM] = ACTION_TERMINATE,
  [SIGUSR1] = ACTION_TERMINATE,
  [SIGUSR2] = ACTION_TERMINATE,
  [SIGPOLL] = ACTION_TERMINATE,
  [SIGPROF] = ACTION_TERMINATE,
  [SIGSYS] = ACTION_ABORT,
  [SIGTRAP] = ACTION_ABORT,
  [SIGVTALRM] = ACTION_TERMINATE,
  [SIGXCPU] = ACTION_ABORT,
  [SIGXFSZ] = ACTION_ABORT,
};

int raise(int sig) {
  if (__sig_is_blocked(sig)) {
    sigaddset(&__sig_pending, sig);
    return 0;
  }
  if (__sig_actions[sig].sa_flags & SA_SIGINFO) {
    siginfo_t t = {0};
    __sig_actions[sig].sa_sigaction(sig, &t, NULL);
  } else {
    void (*handler)(int) = __sig_actions[sig].sa_handler;
    if (handler == SIG_DFL) {
      switch (default_actions[sig]) {
        case ACTION_IGNORE:
          break;
        case ACTION_TERMINATE:
          // Prepare to forcibly shut down runtime even if it has async work in flight.
          emscripten_prepare_force_exit();
          // Intentionally exiting via a function that doesn't call atexit handlers.
          _Exit(128 + sig);
        case ACTION_ABORT:
          abort();
      }
    } else if (handler != SIG_IGN) {
        // Avoid a direct call to the handler, and instead call via JS so we can
        // avoid strict signature checking.
        // https://github.com/emscripten-core/posixtestsuite/issues/6
        __call_sighandler(__sig_actions[sig].sa_handler, sig);
    }
  }
  return 0;
}
