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

void action_abort(int sig) {
  abort();
}

void action_terminate(int sig) {
  // Prepare to forcibly shut down runtime even if it has async work in flight.
  _emscripten_runtime_keepalive_clear();
  // Intentionally exiting via a function that doesn't call atexit handlers.
  _Exit(128 + sig);
}

static sighandler_t default_actions[_NSIG] = {
  [SIGABRT] = action_abort,
  [SIGALRM] = action_terminate,
  [SIGBUS] = action_abort,
  [SIGFPE] = action_abort,
  [SIGHUP] = action_terminate,
  [SIGILL] = action_abort,
  [SIGINT] = action_terminate,
  [SIGKILL] = action_terminate,
  [SIGPIPE] = action_terminate,
  [SIGQUIT] = action_abort,
  [SIGSEGV] = action_abort,
  [SIGTERM] = action_terminate,
  [SIGUSR1] = action_terminate,
  [SIGUSR2] = action_terminate,
  [SIGPOLL] = action_terminate,
  [SIGPROF] = action_terminate,
  [SIGSYS] = action_abort,
  [SIGTRAP] = action_abort,
  [SIGVTALRM] = action_terminate,
  [SIGXCPU] = action_abort,
  [SIGXFSZ] = action_abort,
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
    sighandler_t handler = __sig_actions[sig].sa_handler;
    if (handler == SIG_DFL) {
      handler = default_actions[sig];
      if (handler) {
        handler(sig);
      }
    } else if (handler != SIG_IGN) {
        // Avoid a direct call to the handler, and instead call via JS so we can
        // avoid strict signature checking.
        // https://github.com/emscripten-core/posixtestsuite/issues/6
        __call_sighandler(handler, sig);
    }
  }
  return 0;
}
