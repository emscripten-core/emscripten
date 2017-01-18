// 'use strict'
var funs = 
{
	signal__deps: ['_sig_handler_'], 
	
	signal: function(sig, func) 
	{
		if ( typeof signal__deps === 'undefined' ) signal__deps = [] ;
		if ( typeof signal__deps['_sig_handler_'] === 'undefined' ) signal__deps['_sig_handler_'] = [] ;
		signal__deps['_sig_handler_'][sig] = func ;

		return sig;
	},

	sigemptyset: function(set) 
	{
		{{{ makeSetValue('set', '0', '0', 'i32') }}};
		return 0;
	},
	sigfillset: function(set) 
	{
		{{{ makeSetValue('set', '0', '-1>>>0', 'i32') }}};
		return 0;
	},
	sigaddset: function(set, signum) 
	{
		{{{ makeSetValue('set', '0', makeGetValue('set', '0', 'i32') + '| (1 << (signum-1))', 'i32') }}};
		return 0;
	},
	sigdelset: function(set, signum) 
	{
		{{{ makeSetValue('set', '0', makeGetValue('set', '0', 'i32') + '& (~(1 << (signum-1)))', 'i32') }}};
		return 0;
	},
	sigismember: function(set, signum) 
	{
		return {{{ makeGetValue('set', '0', 'i32') }}} & (1 << (signum-1));
	},
	sigaction: function(signum, act, oldact) 
	{
	//int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of sigaction()');
#endif
		return 0;
	},
	sigprocmask: function() 
	{
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of sigprocmask()');
#endif
		return 0;
	},
	__libc_current_sigrtmin: function() 
	{
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of __libc_current_sigrtmin');
#endif
		return 0;
	},
	__libc_current_sigrtmax: function() 
	{
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of __libc_current_sigrtmax');
#endif
		return 0;
	},
	kill__deps: ['$ERRNO_CODES', '__setErrNo'],
	kill: function(pid, sig) 
	{
	// http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
	// Makes no sense in a single-process environment.
	  // Should kill itself somtimes depending on `pid`
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of kill()');
#endif
		___setErrNo(ERRNO_CODES.EPERM);
		return -1;
	},
	killpg__deps: ['$ERRNO_CODES', '__setErrNo'],
	killpg: function() 
	{
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of killpg()');
#endif
		___setErrNo(ERRNO_CODES.EPERM);
		return -1;
	},
  siginterrupt: function() {
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of siginterrupt()');
#endif
		return 0;
	},
	raise: function(sig) 
	{
		var ff = signal__deps['_sig_handler_'][sig] ;
		if ( typeof signal__deps['_sig_handler_'][sig] === 'undefined' )
		  return -1;
		  
		Runtime.dynCall('vi', ff, [0]);
		___setErrNo(ERRNO_CODES.ENOSYS);
			
		return sig;
	},
	// http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
	alarm__deps: ['_sigalrm_handler'],
	alarm: function(seconds) 
	{
		setTimeout(function() {
		  if (__sigalrm_handler) Runtime.dynCall('vi', __sigalrm_handler, [0]);
		}, seconds*1000);
	},
	ualarm: function() 
	{
		throw 'ualarm() is not implemented yet';
	},
	setitimer: function() 
	{
		throw 'setitimer() is not implemented yet';
	},
	getitimer: function() 
	{
		throw 'getitimer() is not implemented yet';
	},
	pause__deps: ['__setErrNo', '$ERRNO_CODES'],
	pause: function() 
	{
		// int pause(void);
		// http://pubs.opengroup.org/onlinepubs/000095399/functions/pause.html
		// We don't support signals, so we return immediately.
#if ASSERTIONS
    signal__deps.printErr('Calling stub instead of pause()');
#endif
		___setErrNo(ERRNO_CODES.EINTR);
		return -1;
  },
	sigpending: function(set) 
	{
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
 
