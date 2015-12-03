// 'use strict'
var funs = 
{
  signal__deps: ['_sig_handler_'],

  signal: function(sig, func)   {
    if ( typeof signal__deps === 'undefined' ) signal__deps = [] ;
    if ( typeof signal__deps['_sig_handler_'] === 'undefined' ) signal__deps['_sig_handler_'] = [] ;
    signal__deps['_sig_handler_'][sig] = func ;

	return sig;
  },
  sigemptyset: function(set)   {
	{{{ makeSetValue('set', '0', '0', 'i32') }}};
	return 0;
  },
  sigfillset: function(set)  {
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
    Module.printErr('Calling stub instead of sigaction()');
#endif
    return 0;
  },
  sigprocmask: function( how , set , old )   {
    var SIG_BLOCK = 1;
    var EINVAL = 22 ;
    if ( how - SIG_BLOCK >2 ) return EINVAL ;
	//ret = -__syscall(SYS_rt_sigprocmask, how, set, old, _NSIG/8);
	ret = 0 ;
     if (!ret && old) {
	{{{ makeSetValue('old', '0', '~0x80000000', 'i32') }}};
	{{{ makeSetValue('old', '1', '~0x00000003', 'i32') }}};				
    }	
    return ret;
  },
  __libc_current_sigrtmin: function()  {
	// POSIX timers use __SIGRTMIN + 0.
	// libbacktrace uses __SIGRTMIN + 1.
	// libcore uses __SIGRTMIN + 2.
	var __SIGRTMIN	= 34 
    return __SIGRTMIN+3;
  },
  __libc_current_sigrtmax: function() 
  {
	var  __SIGRTMAX	= 64;
    return __SIGRTMAX;
  },
  kill__deps: ['$ERRNO_CODES', '__setErrNo'],
  kill: function(pid, sig) {
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
    // Makes no sense in a single-process environment.
	  // Should kill itself somtimes depending on `pid`
#if ASSERTIONS
    Module.printErr('Calling stub instead of kill()');
#endif
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  killpg__deps: ['$ERRNO_CODES', '__setErrNo'],
  killpg: function() {
#if ASSERTIONS
    Module.printErr('Calling stub instead of killpg()');
#endif
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  siginterrupt: function() {
#if ASSERTIONS
    Module.printErr('Calling stub instead of siginterrupt()');
#endif
    return 0;
  },
  raise: function(sig)	{
    if ( typeof signal__deps === 'undefined' )
	return -1 ;		
    if ( typeof signal__deps['_sig_handler_'] === 'undefined' )	{
	Module.printErr ( 'undefined signal handler : ' + sig);
	return -1 ;
    }
    if ( typeof signal__deps['_sig_handler_'][sig] === 'undefined' ){
	Module.printErr ( 'undefined signal handler :' + sig );
	return -1;	
    }
    var ff = signal__deps['_sig_handler_'][sig] ;

    if ( ff == 0 ) 	{
	Module.printErr ( 'undefined signal handler :' + sig );
	return -1;	
    }
    Runtime.dynCall('vi', ff, [0]);
    ___setErrNo(ERRNO_CODES.ENOSYS);
    return sig;
  },
  // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
  alarm__deps: ['_sigalrm_handler'],
  alarm: function(seconds) {
    setTimeout(function() {
      if (__sigalrm_handler) Runtime.dynCall('vi', __sigalrm_handler, [0]);
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
	// implemened as #define in emscripten.h
	// #define pause( t ) 
  },
  sigpending: function(set)   {
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
