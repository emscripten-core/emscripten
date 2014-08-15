// 'use strict'
var funs = {
  signal: function(sig, func) {
    Module.printErr('Calling stub instead of signal()');
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
    Module.printErr('Calling stub instead of sigaction()');
    return 0;
  },
  sigprocmask: function() {
    Module.printErr('Calling stub instead of sigprocmask()');
    return 0;
  },
  __libc_current_sigrtmin: function() {
    Module.printErr('Calling stub instead of __libc_current_sigrtmin');
    return 0;
  },
  __libc_current_sigrtmax: function() {
    Module.printErr('Calling stub instead of __libc_current_sigrtmax');
    return 0;
  },
  kill__deps: ['$ERRNO_CODES', '__setErrNo'],
  kill: function(pid, sig) {
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
    // Makes no sense in a single-process environment.
	  // Should kill itself somtimes depending on `pid`
    Module.printErr('Calling stub instead of kill()');
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  killpg__deps: ['$ERRNO_CODES', '__setErrNo'],
  killpg: function() {
    Module.printErr('Calling stub instead of killpg()');
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  siginterrupt: function() {
    Module.printErr('Calling stub instead of siginterrupt()');
    return 0;
  },

  raise__deps: ['$ERRNO_CODES', '__setErrNo'],
  raise: function(sig) {
    Module.printErr('Calling stub instead of raise()');
  ___setErrNo(ERRNO_CODES.ENOSYS);
#if ASSERTIONS
      Runtime.warnOnce('raise() returning an error as we do not support it');
#endif
    return -1;
  },

  alarm: function(seconds) {
    // unsigned alarm(unsigned seconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
    // We don't support signals, and there's no way to indicate failure, so just
    // fail silently.
    throw 'alarm() is not implemented yet';
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
    Module.printErr('Calling stub instead of pause()');
    ___setErrNo(ERRNO_CODES.EINTR);
    return -1;
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
  //sigpending
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
