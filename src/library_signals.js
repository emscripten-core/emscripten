// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// 'use strict'
var funs = {
  _sigalrm_handler: 0,

  signal__deps: ['_sigalrm_handler'],
  signal: function(sig, func) {
    if (sig == 14 /*SIGALRM*/) {
      __sigalrm_handler = func;
    } else {
#if ASSERTIONS
      err('Calling stub instead of signal()');
#endif
    }
    return 0;
  },
  sigemptyset: function(set) {
    {{{ makeSetValue('set', '0', '0', 'i32') }}};
    return 0;
  },
  sigfillset: function(set) {
    {{{ makeSetValue('set', '0', '-1>>>0', 'i32') }}};
    return 0;
  },
  sigaddset: function(set, signum) {
    {{{ makeSetValue('set', '0', makeGetValue('set', '0', 'i32') + '| (1 << (signum-1))', 'i32') }}};
    return 0;
  },
  sigdelset: function(set, signum) {
    {{{ makeSetValue('set', '0', makeGetValue('set', '0', 'i32') + '& (~(1 << (signum-1)))', 'i32') }}};
    return 0;
  },
  sigismember: function(set, signum) {
    return {{{ makeGetValue('set', '0', 'i32') }}} & (1 << (signum-1));
  },
  sigaction: function(signum, act, oldact) {
    //int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
#if ASSERTIONS
    err('Calling stub instead of sigaction()');
#endif
    return 0;
  },
  sigprocmask: function() {
#if ASSERTIONS
    err('Calling stub instead of sigprocmask()');
#endif
    return 0;
  },
  __libc_current_sigrtmin: function() {
#if ASSERTIONS
    err('Calling stub instead of __libc_current_sigrtmin');
#endif
    return 0;
  },
  __libc_current_sigrtmax: function() {
#if ASSERTIONS
    err('Calling stub instead of __libc_current_sigrtmax');
#endif
    return 0;
  },
  kill__deps: ['$ERRNO_CODES', '__setErrNo'],
  kill: function(pid, sig) {
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
    // Makes no sense in a single-process environment.
	  // Should kill itself somtimes depending on `pid`
#if ASSERTIONS
    err('Calling stub instead of kill()');
#endif
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  killpg__deps: ['$ERRNO_CODES', '__setErrNo'],
  killpg: function() {
#if ASSERTIONS
    err('Calling stub instead of killpg()');
#endif
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  siginterrupt: function() {
#if ASSERTIONS
    err('Calling stub instead of siginterrupt()');
#endif
    return 0;
  },

  raise__deps: ['$ERRNO_CODES', '__setErrNo'],
  raise: function(sig) {
#if ASSERTIONS
    err('Calling stub instead of raise()');
#endif
  ___setErrNo(ERRNO_CODES.ENOSYS);
#if ASSERTIONS
    warnOnce('raise() returning an error as we do not support it');
#endif
    return -1;
  },

  // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
  alarm__deps: ['_sigalrm_handler'],
  alarm: function(seconds) {
    setTimeout(function() {
      if (__sigalrm_handler) Module['dynCall_vi'](__sigalrm_handler, 0);
    }, seconds*1000);
  },
  ualarm: function() {
    throw 'ualarm() is not implemented yet';
  },
  setitimer: function() {
    throw 'setitimer() is not implemented yet';
  },
  getitimer: function() {
    throw 'getitimer() is not implemented yet';
  },

  pause__deps: ['__setErrNo', '$ERRNO_CODES'],
  pause: function() {
    // int pause(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/pause.html
    // We don't support signals, so we return immediately.
#if ASSERTIONS
    err('Calling stub instead of pause()');
#endif
    ___setErrNo(ERRNO_CODES.EINTR);
    return -1;
  },
#if ASSERTIONS
  siglongjmp__deps: ['longjmp'],
  siglongjmp: function(env, value) {
    // We cannot wrap the sigsetjmp, but I hope that
    // in most cases siglongjmp will be called later.

    // siglongjmp can be called very many times, so don't flood the stderr.
    warnOnce("Calling longjmp() instead of siglongjmp()");
    _longjmp(env, value);
  },
#else
  siglongjmp: 'longjmp',
#endif
  sigpending: function(set) {
    {{{ makeSetValue('set', 0, 0, 'i32') }}};
    return 0;
  }
  //signalfd
  //ppoll
  //epoll_pwait
  //pselect
  //sigvec
  //sigmask
  //sigblock
  //sigsetmask
  //siggetmask
  //sigsuspend
  //bsd_signal
  //siginterrupt
  //sigqueue
  //sysv_signal
  //signal
  //pthread_kill
  //gsignal
  //ssignal
  //psignal
  //psiginfo
  //sigpause
  //sigisemptyset
  //sigtimedwait
  //sigwaitinfo
  //sigreturn
  //sigstack
  //sigaltstack(2)
  //sigsetops(3),
  //sighold
  //sigrelse
  //sigignore
  //sigset
};

mergeInto(LibraryManager.library, funs);
