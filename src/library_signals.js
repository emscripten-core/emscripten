/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// 'use strict'
var LibrarySignals = {
  _sigalrm_handler: 0,

  __sigaction__deps: ['_sigalrm_handler'],
  __sigaction: function(sig, act, oldact) {
    //int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
    if (sig == {{{ cDefine('SIGALRM') }}}) {
      __sigalrm_handler = {{{ makeGetValue('act', '0', 'i32') }}};
      return 0;
    }
#if ASSERTIONS
    err('sigaction: signal type not supported: this is a no-op.');
#endif
    return 0;
  },
  sigaction__sig: 'viii',
  sigaction: '__sigaction',

  // pthread_sigmask - examine and change mask of blocked signals
  pthread_sigmask: function(how, set, oldset) {
    err('pthread_sigmask() is not supported: this is a no-op.');
    return 0;
  },

  kill__deps: ['$ERRNO_CODES', '$setErrNo'],
  kill: function(pid, sig) {
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
    // Makes no sense in a single-process environment.
    // Should kill itself somtimes depending on `pid`
#if ASSERTIONS
    err('Calling stub instead of kill()');
#endif
    setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  siginterrupt: function() {
#if ASSERTIONS
    err('Calling stub instead of siginterrupt()');
#endif
    return 0;
  },

  raise__deps: ['$ERRNO_CODES', '$setErrNo'],
  raise: function(sig) {
#if ASSERTIONS
    err('Calling stub instead of raise()');
#endif
    setErrNo(ERRNO_CODES.ENOSYS);
    return -1;
  },

  // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
  alarm__deps: ['_sigalrm_handler'],
  alarm: function(seconds) {
    setTimeout(function() {
      if (__sigalrm_handler) {{{ makeDynCall('vi', '__sigalrm_handler') }}}(0);
    }, seconds*1000);
  },

  sigpending: function(set) {
    {{{ makeSetValue('set', 0, 0, 'i32') }}};
    return 0;
  },

  //int sigtimedwait(const sigset_t *restrict mask, siginfo_t *restrict si, const struct timespec *restrict timeout)
  sigtimedwait: function(set, sig, timeout) {
    // POSIX SIGNALS are not supported
    // if set contains an invalid signal number, EINVAL is returned
    // in our case we return EINVAL all the time
#if ASSERTIONS
    err('Calling stub instead of sigwait()');
#endif
    return {{{ cDefine('EINVAL') }}};
  }
};

mergeInto(LibraryManager.library, LibrarySignals);
