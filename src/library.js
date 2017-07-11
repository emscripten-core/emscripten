//"use strict";

// An implementation of basic necessary libraries for the web. This integrates
// with a compiled libc and with the rest of the JS runtime.
//
// We search the Library object when there is an external function. If the
// entry in the Library is a function, we insert it. If it is a string, we
// do another lookup in the library (a simple way to write a function once,
// if it can be called by different names). We also allow dependencies,
// using __deps. Initialization code to be run after allocating all
// global constants can be defined by __postset.
//
// Note that the full function name will be '_' + the name in the Library
// object. For convenience, the short name appears here. Note that if you add a
// new function with an '_', it will not be found.

// Memory allocated during startup, in postsets, should only be ALLOC_STATIC

LibraryManager.library = {
  // keep this low in memory, because we flatten arrays with them in them
#if USE_PTHREADS
  stdin: '; if (ENVIRONMENT_IS_PTHREAD) _stdin = PthreadWorkerInit._stdin; else PthreadWorkerInit._stdin = _stdin = allocate(1, "i32*", ALLOC_STATIC)',
  stdout: '; if (ENVIRONMENT_IS_PTHREAD) _stdout = PthreadWorkerInit._stdout; else PthreadWorkerInit._stdout = _stdout = allocate(1, "i32*", ALLOC_STATIC)',
  stderr: '; if (ENVIRONMENT_IS_PTHREAD) _stderr = PthreadWorkerInit._stderr; else PthreadWorkerInit._stderr = _stderr = allocate(1, "i32*", ALLOC_STATIC)',
  _impure_ptr: '; if (ENVIRONMENT_IS_PTHREAD) __impure_ptr = PthreadWorkerInit.__impure_ptr; else PthreadWorkerInit.__impure_ptr __impure_ptr = allocate(1, "i32*", ALLOC_STATIC)',
  __dso_handle: '; if (ENVIRONMENT_IS_PTHREAD) ___dso_handle = PthreadWorkerInit.___dso_handle; else PthreadWorkerInit.___dso_handle = ___dso_handle = allocate(1, "i32*", ALLOC_STATIC)',
#else
  stdin: '{{{ makeStaticAlloc(1) }}}',
  stdout: '{{{ makeStaticAlloc(1) }}}',
  stderr: '{{{ makeStaticAlloc(1) }}}',
  _impure_ptr: '{{{ makeStaticAlloc(1) }}}',
  __dso_handle: '{{{ makeStaticAlloc(1) }}}',
#endif

  $PROCINFO: {
    // permissions
    /*
    uid: 0,
    gid: 0,
    euid: 0,
    egid: 0,
    suid: 0,
    sgid: 0,
    fsuid: 0,
    fsgid: 0,
    */
    // process identification
    ppid: 1,
    pid: 42,
    sid: 42,
    pgid: 42
  },

  // ==========================================================================
  // utime.h
  // ==========================================================================

  utime__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  utime: function(path, times) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_UTIME') }}}, path, times);
#endif
    // int utime(const char *path, const struct utimbuf *times);
    // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/utime.h.html
    var time;
    if (times) {
      // NOTE: We don't keep track of access timestamps.
      var offset = {{{ C_STRUCTS.utimbuf.modtime }}};
      time = {{{ makeGetValue('times', 'offset', 'i32') }}};
      time *= 1000;
    } else {
      time = Date.now();
    }
    path = Pointer_stringify(path);
    try {
      FS.utime(path, time, time);
      return 0;
    } catch (e) {
      FS.handleFSError(e);
      return -1;
    }
  },

  utimes__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  utimes: function(path, times) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_UTIMES') }}}, path, times);
#endif
    var time;
    if (times) {
      var offset = {{{ C_STRUCTS.timeval.__size__ }}} + {{{ C_STRUCTS.timeval.tv_sec }}};
      time = {{{ makeGetValue('times', 'offset', 'i32') }}} * 1000;
      offset = {{{ C_STRUCTS.timeval.__size__ }}} + {{{ C_STRUCTS.timeval.tv_usec }}};
      time += {{{ makeGetValue('times', 'offset', 'i32') }}} / 1000;
    } else {
      time = Date.now();
    }
    path = Pointer_stringify(path);
    try {
      FS.utime(path, time, time);
      return 0;
    } catch (e) {
      FS.handleFSError(e);
      return -1;
    }
  },

  // ==========================================================================
  // sys/file.h
  // ==========================================================================

  flock: function(fd, operation) {
    // int flock(int fd, int operation);
    // Pretend to succeed
    return 0;
  },

  chroot__deps: ['__setErrNo', '$ERRNO_CODES'],
  chroot: function(path) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_CHROOT') }}}, path);
#endif
    // int chroot(const char *path);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
    ___setErrNo(ERRNO_CODES.EACCES);
    return -1;
  },

  fpathconf__deps: ['__setErrNo', '$ERRNO_CODES'],
  fpathconf: function(fildes, name) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_FPATHCONF') }}}, fildes, name);
#endif
    // long fpathconf(int fildes, int name);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/encrypt.html
    // NOTE: The first parameter is ignored, so pathconf == fpathconf.
    // The constants here aren't real values. Just mimicking glibc.
    switch (name) {
      case {{{ cDefine('_PC_LINK_MAX') }}}:
        return 32000;
      case {{{ cDefine('_PC_MAX_CANON') }}}:
      case {{{ cDefine('_PC_MAX_INPUT') }}}:
      case {{{ cDefine('_PC_NAME_MAX') }}}:
        return 255;
      case {{{ cDefine('_PC_PATH_MAX') }}}:
      case {{{ cDefine('_PC_PIPE_BUF') }}}:
      case {{{ cDefine('_PC_REC_MIN_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_REC_XFER_ALIGN') }}}:
      case {{{ cDefine('_PC_ALLOC_SIZE_MIN') }}}:
        return 4096;
      case {{{ cDefine('_PC_CHOWN_RESTRICTED') }}}:
      case {{{ cDefine('_PC_NO_TRUNC') }}}:
      case {{{ cDefine('_PC_2_SYMLINKS') }}}:
        return 1;
      case {{{ cDefine('_PC_VDISABLE') }}}:
        return 0;
      case {{{ cDefine('_PC_SYNC_IO') }}}:
      case {{{ cDefine('_PC_ASYNC_IO') }}}:
      case {{{ cDefine('_PC_PRIO_IO') }}}:
      case {{{ cDefine('_PC_SOCK_MAXBUF') }}}:
      case {{{ cDefine('_PC_REC_INCR_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_REC_MAX_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_SYMLINK_MAX') }}}:
        return -1;
      case {{{ cDefine('_PC_FILESIZEBITS') }}}:
        return 64;
    }
    ___setErrNo(ERRNO_CODES.EINVAL);
    return -1;
  },
  pathconf: 'fpathconf',

  confstr__deps: ['__setErrNo', '$ERRNO_CODES', '$ENV'],
  confstr: function(name, buf, len) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_3({{{ cDefine('EM_PROXIED_CONFSTR') }}}, name, buf, len);
#endif
    // size_t confstr(int name, char *buf, size_t len);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/confstr.html
    var value;
    switch (name) {
      case {{{ cDefine('_CS_PATH') }}}:
        value = ENV['PATH'] || '/';
        break;
      case {{{ cDefine('_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS') }}}:
        // Mimicking glibc.
        value = 'POSIX_V6_ILP32_OFF32\nPOSIX_V6_ILP32_OFFBIG';
        break;
      case {{{ cDefine('_CS_GNU_LIBC_VERSION') }}}:
        // This JS implementation was tested against this glibc version.
        value = 'glibc 2.14';
        break;
      case {{{ cDefine('_CS_GNU_LIBPTHREAD_VERSION') }}}:
        // We don't support pthreads.
        value = '';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_LIBS') }}}:
        value = '';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS') }}}:
        value = '-m32';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS') }}}:
        value = '-m32 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64';
        break;
      default:
        ___setErrNo(ERRNO_CODES.EINVAL);
        return 0;
    }
    if (len == 0 || buf == 0) {
      return value.length + 1;
    } else {
      var length = Math.min(len, value.length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('buf', 'i', 'value.charCodeAt(i)', 'i8') }}};
      }
      if (len > length) {{{ makeSetValue('buf', 'i++', '0', 'i8') }}};
      return i;
    }
  },

  execl__deps: ['__setErrNo', '$ERRNO_CODES'],
  execl: function(/* ... */) {
    // int execl(const char *path, const char *arg0, ... /*, (char *)0 */);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/exec.html
    // We don't support executing external code.
    ___setErrNo(ERRNO_CODES.ENOEXEC);
    return -1;
  },
  execle: 'execl',
  execlp: 'execl',
  execv: 'execl',
  execve: 'execl',
  execvp: 'execl',
  __execvpe: 'execl',
  fexecve: 'execl',

  _exit: function(status) {
    // void _exit(int status);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html
    Module['exit'](status);
  },

  fork__deps: ['__setErrNo', '$ERRNO_CODES'],
  fork: function() {
    // pid_t fork(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
    // We don't support multiple processes.
    ___setErrNo(ERRNO_CODES.EAGAIN);
    return -1;
  },
  vfork: 'fork',
  posix_spawn: 'fork',
  posix_spawnp: 'fork',

  setgroups__deps: ['__setErrNo', '$ERRNO_CODES', 'sysconf'],
  setgroups: function(ngroups, gidset) {
    // int setgroups(int ngroups, const gid_t *gidset);
    // https://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man2/setgroups.2.html
    if (ngroups < 1 || ngroups > _sysconf({{{ cDefine('_SC_NGROUPS_MAX') }}})) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      // We have just one process/user/group, so it makes no sense to set groups.
      ___setErrNo(ERRNO_CODES.EPERM);
      return -1;
    }
  },
  getpagesize: function() {
    // int getpagesize(void);
    return PAGE_SIZE;
  },

  sysconf__deps: ['__setErrNo', '$ERRNO_CODES'],
  sysconf: function(name) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_SYSCONF') }}}, name);
#endif
    // long sysconf(int name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/sysconf.html
    switch(name) {
      case {{{ cDefine('_SC_PAGE_SIZE') }}}: return PAGE_SIZE;
      case {{{ cDefine('_SC_PHYS_PAGES') }}}:
#if BINARYEN
        var maxHeapSize = 2*1024*1024*1024 - 65536;
#else
        var maxHeapSize = 2*1024*1024*1024 - 16777216;
#endif
#if BINARYEN_MEM_MAX != -1
        maxHeapSize = {{{ BINARYEN_MEM_MAX }}};
#endif
#if !ALLOW_MEMORY_GROWTH
        maxHeapSize = HEAPU8.length;
#endif
        return maxHeapSize / PAGE_SIZE;
      case {{{ cDefine('_SC_ADVISORY_INFO') }}}:
      case {{{ cDefine('_SC_BARRIERS') }}}:
      case {{{ cDefine('_SC_ASYNCHRONOUS_IO') }}}:
      case {{{ cDefine('_SC_CLOCK_SELECTION') }}}:
      case {{{ cDefine('_SC_CPUTIME') }}}:
      case {{{ cDefine('_SC_FSYNC') }}}:
      case {{{ cDefine('_SC_IPV6') }}}:
      case {{{ cDefine('_SC_MAPPED_FILES') }}}:
      case {{{ cDefine('_SC_MEMLOCK') }}}:
      case {{{ cDefine('_SC_MEMLOCK_RANGE') }}}:
      case {{{ cDefine('_SC_MEMORY_PROTECTION') }}}:
      case {{{ cDefine('_SC_MESSAGE_PASSING') }}}:
      case {{{ cDefine('_SC_MONOTONIC_CLOCK') }}}:
      case {{{ cDefine('_SC_PRIORITIZED_IO') }}}:
      case {{{ cDefine('_SC_PRIORITY_SCHEDULING') }}}:
      case {{{ cDefine('_SC_RAW_SOCKETS') }}}:
      case {{{ cDefine('_SC_READER_WRITER_LOCKS') }}}:
      case {{{ cDefine('_SC_REALTIME_SIGNALS') }}}:
      case {{{ cDefine('_SC_SEMAPHORES') }}}:
      case {{{ cDefine('_SC_SHARED_MEMORY_OBJECTS') }}}:
      case {{{ cDefine('_SC_SPAWN') }}}:
      case {{{ cDefine('_SC_SPIN_LOCKS') }}}:
      case {{{ cDefine('_SC_SYNCHRONIZED_IO') }}}:
      case {{{ cDefine('_SC_THREAD_ATTR_STACKADDR') }}}:
      case {{{ cDefine('_SC_THREAD_ATTR_STACKSIZE') }}}:
      case {{{ cDefine('_SC_THREAD_CPUTIME') }}}:
      case {{{ cDefine('_SC_THREAD_PRIO_INHERIT') }}}:
      case {{{ cDefine('_SC_THREAD_PRIO_PROTECT') }}}:
      case {{{ cDefine('_SC_THREAD_PROCESS_SHARED') }}}:
      case {{{ cDefine('_SC_THREAD_SAFE_FUNCTIONS') }}}:
      case {{{ cDefine('_SC_THREADS') }}}:
      case {{{ cDefine('_SC_TIMEOUTS') }}}:
      case {{{ cDefine('_SC_TIMERS') }}}:
      case {{{ cDefine('_SC_VERSION') }}}:
      case {{{ cDefine('_SC_2_C_BIND') }}}:
      case {{{ cDefine('_SC_2_C_DEV') }}}:
      case {{{ cDefine('_SC_2_CHAR_TERM') }}}:
      case {{{ cDefine('_SC_2_LOCALEDEF') }}}:
      case {{{ cDefine('_SC_2_SW_DEV') }}}:
      case {{{ cDefine('_SC_2_VERSION') }}}:
        return 200809;
      case {{{ cDefine('_SC_THREAD_PRIORITY_SCHEDULING') }}}:
        return 0;
      case {{{ cDefine('_SC_MQ_OPEN_MAX') }}}:
      case {{{ cDefine('_SC_XOPEN_STREAMS') }}}:
      case {{{ cDefine('_SC_XBS5_LP64_OFF64') }}}:
      case {{{ cDefine('_SC_XBS5_LPBIG_OFFBIG') }}}:
      case {{{ cDefine('_SC_AIO_LISTIO_MAX') }}}:
      case {{{ cDefine('_SC_AIO_MAX') }}}:
      case {{{ cDefine('_SC_SPORADIC_SERVER') }}}:
      case {{{ cDefine('_SC_THREAD_SPORADIC_SERVER') }}}:
      case {{{ cDefine('_SC_TRACE') }}}:
      case {{{ cDefine('_SC_TRACE_EVENT_FILTER') }}}:
      case {{{ cDefine('_SC_TRACE_EVENT_NAME_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_INHERIT') }}}:
      case {{{ cDefine('_SC_TRACE_LOG') }}}:
      case {{{ cDefine('_SC_TRACE_NAME_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_SYS_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_USER_EVENT_MAX') }}}:
      case {{{ cDefine('_SC_TYPED_MEMORY_OBJECTS') }}}:
      case {{{ cDefine('_SC_V6_LP64_OFF64') }}}:
      case {{{ cDefine('_SC_V6_LPBIG_OFFBIG') }}}:
      case {{{ cDefine('_SC_2_FORT_DEV') }}}:
      case {{{ cDefine('_SC_2_FORT_RUN') }}}:
      case {{{ cDefine('_SC_2_PBS') }}}:
      case {{{ cDefine('_SC_2_PBS_ACCOUNTING') }}}:
      case {{{ cDefine('_SC_2_PBS_CHECKPOINT') }}}:
      case {{{ cDefine('_SC_2_PBS_LOCATE') }}}:
      case {{{ cDefine('_SC_2_PBS_MESSAGE') }}}:
      case {{{ cDefine('_SC_2_PBS_TRACK') }}}:
      case {{{ cDefine('_SC_2_UPE') }}}:
      case {{{ cDefine('_SC_THREAD_THREADS_MAX') }}}:
      case {{{ cDefine('_SC_SEM_NSEMS_MAX') }}}:
      case {{{ cDefine('_SC_SYMLOOP_MAX') }}}:
      case {{{ cDefine('_SC_TIMER_MAX') }}}:
        return -1;
      case {{{ cDefine('_SC_V6_ILP32_OFF32') }}}:
      case {{{ cDefine('_SC_V6_ILP32_OFFBIG') }}}:
      case {{{ cDefine('_SC_JOB_CONTROL') }}}:
      case {{{ cDefine('_SC_REGEXP') }}}:
      case {{{ cDefine('_SC_SAVED_IDS') }}}:
      case {{{ cDefine('_SC_SHELL') }}}:
      case {{{ cDefine('_SC_XBS5_ILP32_OFF32') }}}:
      case {{{ cDefine('_SC_XBS5_ILP32_OFFBIG') }}}:
      case {{{ cDefine('_SC_XOPEN_CRYPT') }}}:
      case {{{ cDefine('_SC_XOPEN_ENH_I18N') }}}:
      case {{{ cDefine('_SC_XOPEN_LEGACY') }}}:
      case {{{ cDefine('_SC_XOPEN_REALTIME') }}}:
      case {{{ cDefine('_SC_XOPEN_REALTIME_THREADS') }}}:
      case {{{ cDefine('_SC_XOPEN_SHM') }}}:
      case {{{ cDefine('_SC_XOPEN_UNIX') }}}:
        return 1;
      case {{{ cDefine('_SC_THREAD_KEYS_MAX') }}}:
      case {{{ cDefine('_SC_IOV_MAX') }}}:
      case {{{ cDefine('_SC_GETGR_R_SIZE_MAX') }}}:
      case {{{ cDefine('_SC_GETPW_R_SIZE_MAX') }}}:
      case {{{ cDefine('_SC_OPEN_MAX') }}}:
        return 1024;
      case {{{ cDefine('_SC_RTSIG_MAX') }}}:
      case {{{ cDefine('_SC_EXPR_NEST_MAX') }}}:
      case {{{ cDefine('_SC_TTY_NAME_MAX') }}}:
        return 32;
      case {{{ cDefine('_SC_ATEXIT_MAX') }}}:
      case {{{ cDefine('_SC_DELAYTIMER_MAX') }}}:
      case {{{ cDefine('_SC_SEM_VALUE_MAX') }}}:
        return 2147483647;
      case {{{ cDefine('_SC_SIGQUEUE_MAX') }}}:
      case {{{ cDefine('_SC_CHILD_MAX') }}}:
        return 47839;
      case {{{ cDefine('_SC_BC_SCALE_MAX') }}}:
      case {{{ cDefine('_SC_BC_BASE_MAX') }}}:
        return 99;
      case {{{ cDefine('_SC_LINE_MAX') }}}:
      case {{{ cDefine('_SC_BC_DIM_MAX') }}}:
        return 2048;
      case {{{ cDefine('_SC_ARG_MAX') }}}: return 2097152;
      case {{{ cDefine('_SC_NGROUPS_MAX') }}}: return 65536;
      case {{{ cDefine('_SC_MQ_PRIO_MAX') }}}: return 32768;
      case {{{ cDefine('_SC_RE_DUP_MAX') }}}: return 32767;
      case {{{ cDefine('_SC_THREAD_STACK_MIN') }}}: return 16384;
      case {{{ cDefine('_SC_BC_STRING_MAX') }}}: return 1000;
      case {{{ cDefine('_SC_XOPEN_VERSION') }}}: return 700;
      case {{{ cDefine('_SC_LOGIN_NAME_MAX') }}}: return 256;
      case {{{ cDefine('_SC_COLL_WEIGHTS_MAX') }}}: return 255;
      case {{{ cDefine('_SC_CLK_TCK') }}}: return 100;
      case {{{ cDefine('_SC_HOST_NAME_MAX') }}}: return 64;
      case {{{ cDefine('_SC_AIO_PRIO_DELTA_MAX') }}}: return 20;
      case {{{ cDefine('_SC_STREAM_MAX') }}}: return 16;
      case {{{ cDefine('_SC_TZNAME_MAX') }}}: return 6;
      case {{{ cDefine('_SC_THREAD_DESTRUCTOR_ITERATIONS') }}}: return 4;
      case {{{ cDefine('_SC_NPROCESSORS_ONLN') }}}: {
        if (typeof navigator === 'object') return navigator['hardwareConcurrency'] || 1;
        return 1;
      }
    }
    ___setErrNo(ERRNO_CODES.EINVAL);
    return -1;
  },

  // Implement a Linux-like 'memory area' for our 'process'.
  // Changes the size of the memory area by |bytes|; returns the
  // address of the previous top ('break') of the memory area
  // We control the "dynamic" memory - DYNAMIC_BASE to DYNAMICTOP
  sbrk__asm: true,
  sbrk__sig: ['ii'],
  sbrk__deps: ['__setErrNo'],
  sbrk: function(increment) {
    increment = increment|0;
    var oldDynamicTop = 0;
    var oldDynamicTopOnChange = 0;
    var newDynamicTop = 0;
    var totalMemory = 0;
    increment = ((increment + 15) & -16)|0;
#if USE_PTHREADS
    totalMemory = getTotalMemory()|0;

    // Perform a compare-and-swap loop to update the new dynamic top value. This is because
    // this function can becalled simultaneously in multiple threads.
    do {
      oldDynamicTop = Atomics_load(HEAP32, DYNAMICTOP_PTR>>2)|0;
      newDynamicTop = oldDynamicTop + increment | 0;
      // Asking to increase dynamic top to a too high value? In pthreads builds we cannot
      // enlarge memory, so this needs to fail.
      if (((increment|0) > 0 & (newDynamicTop|0) < (oldDynamicTop|0)) // Detect and fail if we would wrap around signed 32-bit int.
        | (newDynamicTop|0) < 0 // Also underflow, sbrk() should be able to be used to subtract.
        | (newDynamicTop|0) > (totalMemory|0)) {
#if ABORTING_MALLOC
        abortOnCannotGrowMemory()|0;
#else
        ___setErrNo({{{ cDefine('ENOMEM') }}});
        return -1;
#endif
      }
      // Attempt to update the dynamic top to new value. Another thread may have beat this thread to the update,
      // in which case we will need to start over by iterating the loop body again.
      oldDynamicTopOnChange = Atomics_compareExchange(HEAP32, DYNAMICTOP_PTR>>2, oldDynamicTop|0, newDynamicTop|0)|0;
    } while((oldDynamicTopOnChange|0) != (oldDynamicTop|0));
#else // singlethreaded build: (-s USE_PTHREADS=0)
    oldDynamicTop = HEAP32[DYNAMICTOP_PTR>>2]|0;
    newDynamicTop = oldDynamicTop + increment | 0;

    if (((increment|0) > 0 & (newDynamicTop|0) < (oldDynamicTop|0)) // Detect and fail if we would wrap around signed 32-bit int.
      | (newDynamicTop|0) < 0) { // Also underflow, sbrk() should be able to be used to subtract.
#if ABORTING_MALLOC
      abortOnCannotGrowMemory()|0;
#endif
      ___setErrNo({{{ cDefine('ENOMEM') }}});
      return -1;
    }

    HEAP32[DYNAMICTOP_PTR>>2] = newDynamicTop;
    totalMemory = getTotalMemory()|0;
    if ((newDynamicTop|0) > (totalMemory|0)) {
      if ((enlargeMemory()|0) == 0) {
        HEAP32[DYNAMICTOP_PTR>>2] = oldDynamicTop;
        ___setErrNo({{{ cDefine('ENOMEM') }}});
        return -1;
      }
    }
#endif
    return oldDynamicTop|0;
  },

  brk__asm: true,
  brk__sig: ['ii'],
  brk: function(newDynamicTop) {
    newDynamicTop = newDynamicTop|0;
    var oldDynamicTop = 0;
    var totalMemory = 0;
#if USE_PTHREADS
    totalMemory = getTotalMemory()|0;
    // Asking to increase dynamic top to a too high value? In pthreads builds we cannot
    // enlarge memory, so this needs to fail.
    if ((newDynamicTop|0) < 0 | (newDynamicTop|0) > (totalMemory|0)) {
#if ABORTING_MALLOC
      abortOnCannotGrowMemory()|0;
#else
      ___setErrNo({{{ cDefine('ENOMEM') }}});
      return -1;
#endif
    }
    Atomics_store(HEAP32, DYNAMICTOP_PTR>>2, newDynamicTop|0)|0;
#else // singlethreaded build: (-s USE_PTHREADS=0)
    if ((newDynamicTop|0) < 0) {
#if ABORTING_MALLOC
      abortOnCannotGrowMemory()|0;
#endif
      ___setErrNo({{{ cDefine('ENOMEM') }}});
      return -1;
    }

    oldDynamicTop = HEAP32[DYNAMICTOP_PTR>>2]|0;
    HEAP32[DYNAMICTOP_PTR>>2] = newDynamicTop;
    totalMemory = getTotalMemory()|0;
    if ((newDynamicTop|0) > (totalMemory|0)) {
      if ((enlargeMemory()|0) == 0) {
        ___setErrNo({{{ cDefine('ENOMEM') }}});
        HEAP32[DYNAMICTOP_PTR>>2] = oldDynamicTop;
        return -1;
      }
    }
#endif
    return 0;
  },

  system__deps: ['__setErrNo', '$ERRNO_CODES'],
  system: function(command) {
    // int system(const char *command);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/system.html
    // Can't call external programs.
    ___setErrNo(ERRNO_CODES.EAGAIN);
    return -1;
  },

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  // tiny, fake malloc/free implementation. If the program actually uses malloc,
  // a compiled version will be used; this will only be used if the runtime
  // needs to allocate something, for which this is good enough if otherwise
  // no malloc is needed.
  malloc: function(bytes) {
    /* Over-allocate to make sure it is byte-aligned by 8.
     * This will leak memory, but this is only the dummy
     * implementation (replaced by dlmalloc normally) so
     * not an issue.
     */
#if ASSERTIONS == 2
    Runtime.warnOnce('using stub malloc (reference it from C to have the real one included)');
#endif
    var ptr = Runtime.dynamicAlloc(bytes + 8);
    return (ptr+8) & 0xFFFFFFF8;
  },
  free: function() {
#if ASSERTIONS == 2
    Runtime.warnOnce('using stub free (reference it from C to have the real one included)');
#endif
},

  abs: 'Math_abs',
  labs: 'Math_abs',

  exit__deps: ['_exit'],
  exit: function(status) {
    __exit(status);
  },
  _Exit__deps: ['exit'],
  _Exit: function(status) {
    __exit(status);
  },

  _ZSt9terminatev__deps: ['exit'],
  _ZSt9terminatev: function() {
    _exit(-1234);
  },

  atexit: function(func, arg) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_ATEXIT') }}}, func, arg);
#endif
    __ATEXIT__.unshift({ func: func, arg: arg });
  },
  __cxa_atexit: 'atexit',

  // used in rust, clang when doing thread_local statics
  __cxa_thread_atexit: 'atexit',
  __cxa_thread_atexit_impl: 'atexit',

  abort: function() {
    Module['abort']();
  },

  environ__deps: ['$ENV'],
#if USE_PTHREADS
  environ: '; if (ENVIRONMENT_IS_PTHREAD) _environ = PthreadWorkerInit._environ; else PthreadWorkerInit._environ = _environ = allocate(1, "i32*", ALLOC_STATIC)',
#else
  environ: '{{{ makeStaticAlloc(1) }}}',
#endif
  __environ__deps: ['environ'],
  __environ: 'environ',
  __buildEnvironment__deps: ['__environ'],
  __buildEnvironment: function(env) {
    // WARNING: Arbitrary limit!
    var MAX_ENV_VALUES = 64;
    var TOTAL_ENV_SIZE = 1024;

    // Statically allocate memory for the environment.
    var poolPtr;
    var envPtr;
    if (!___buildEnvironment.called) {
      ___buildEnvironment.called = true;
      // Set default values. Use string keys for Closure Compiler compatibility.
      ENV['USER'] = ENV['LOGNAME'] = 'web_user';
      ENV['PATH'] = '/';
      ENV['PWD'] = '/';
      ENV['HOME'] = '/home/web_user';
      ENV['LANG'] = 'C';
      ENV['_'] = Module['thisProgram'];
      // Allocate memory.
      poolPtr = allocate(TOTAL_ENV_SIZE, 'i8', ALLOC_STATIC);
      envPtr = allocate(MAX_ENV_VALUES * {{{ Runtime.QUANTUM_SIZE }}},
                        'i8*', ALLOC_STATIC);
      {{{ makeSetValue('envPtr', '0', 'poolPtr', 'i8*') }}};
      {{{ makeSetValue(makeGlobalUse('_environ'), 0, 'envPtr', 'i8*') }}};
    } else {
      envPtr = {{{ makeGetValue(makeGlobalUse('_environ'), '0', 'i8**') }}};
      poolPtr = {{{ makeGetValue('envPtr', '0', 'i8*') }}};
    }

    // Collect key=value lines.
    var strings = [];
    var totalSize = 0;
    for (var key in env) {
      if (typeof env[key] === 'string') {
        var line = key + '=' + env[key];
        strings.push(line);
        totalSize += line.length;
      }
    }
    if (totalSize > TOTAL_ENV_SIZE) {
      throw new Error('Environment size exceeded TOTAL_ENV_SIZE!');
    }

    // Make new.
    var ptrSize = {{{ Runtime.getNativeTypeSize('i8*') }}};
    for (var i = 0; i < strings.length; i++) {
      var line = strings[i];
      writeAsciiToMemory(line, poolPtr);
      {{{ makeSetValue('envPtr', 'i * ptrSize', 'poolPtr', 'i8*') }}};
      poolPtr += line.length + 1;
    }
    {{{ makeSetValue('envPtr', 'strings.length * ptrSize', '0', 'i8*') }}};
  },
  $ENV__deps: ['__buildEnvironment'],
#if USE_PTHREADS
  $ENV__postset: 'if (!ENVIRONMENT_IS_PTHREAD) ___buildEnvironment(ENV);',
#else
  $ENV__postset: '___buildEnvironment(ENV);',
#endif
  $ENV: {},
  getenv__deps: ['$ENV'],
  getenv: function(name) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_GETENV') }}}, name);
#endif
    // char *getenv(const char *name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/getenv.html
    if (name === 0) return 0;
    name = Pointer_stringify(name);
    if (!ENV.hasOwnProperty(name)) return 0;

    if (_getenv.ret) _free(_getenv.ret);
    _getenv.ret = allocate(intArrayFromString(ENV[name]), 'i8', ALLOC_NORMAL);
    return _getenv.ret;
  },
  clearenv__deps: ['$ENV', '__buildEnvironment'],
  clearenv: function(name) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_CLEARENV') }}}, name);
#endif
    // int clearenv (void);
    // http://www.gnu.org/s/hello/manual/libc/Environment-Access.html#index-clearenv-3107
    ENV = {};
    ___buildEnvironment(ENV);
    return 0;
  },
  setenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  setenv: function(envname, envval, overwrite) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_3({{{ cDefine('EM_PROXIED_SETENV') }}}, envname, envval, overwrite);
#endif
    // int setenv(const char *envname, const char *envval, int overwrite);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/setenv.html
    if (envname === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    var name = Pointer_stringify(envname);
    var val = Pointer_stringify(envval);
    if (name === '' || name.indexOf('=') !== -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    if (ENV.hasOwnProperty(name) && !overwrite) return 0;
    ENV[name] = val;
    ___buildEnvironment(ENV);
    return 0;
  },
  unsetenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  unsetenv: function(name) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_UNSETENV') }}}, name);
#endif
    // int unsetenv(const char *name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
    if (name === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    name = Pointer_stringify(name);
    if (name === '' || name.indexOf('=') !== -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    if (ENV.hasOwnProperty(name)) {
      delete ENV[name];
      ___buildEnvironment(ENV);
    }
    return 0;
  },
  putenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  putenv: function(string) {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_1({{{ cDefine('EM_PROXIED_PUTENV') }}}, string);
#endif
    // int putenv(char *string);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/putenv.html
    // WARNING: According to the standard (and the glibc implementation), the
    //          string is taken by reference so future changes are reflected.
    //          We copy it instead, possibly breaking some uses.
    if (string === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    string = Pointer_stringify(string);
    var splitPoint = string.indexOf('=')
    if (string === '' || string.indexOf('=') === -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    var name = string.slice(0, splitPoint);
    var value = string.slice(splitPoint + 1);
    if (!(name in ENV) || ENV[name] !== value) {
      ENV[name] = value;
      ___buildEnvironment(ENV);
    }
    return 0;
  },

  getloadavg: function(loadavg, nelem) {
    // int getloadavg(double loadavg[], int nelem);
    // http://linux.die.net/man/3/getloadavg
    var limit = Math.min(nelem, 3);
    var doubleSize = {{{ Runtime.getNativeTypeSize('double') }}};
    for (var i = 0; i < limit; i++) {
      {{{ makeSetValue('loadavg', 'i * doubleSize', '0.1', 'double') }}};
    }
    return limit;
  },

  // For compatibility, call to rand() when code requests arc4random(), although this is *not* at all
  // as strong as rc4 is. See https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/arc4random.3.html
  arc4random: 'rand',

  // ==========================================================================
  // string.h
  // ==========================================================================

  memcpy__inline: function(dest, src, num, align) {
    var ret = '';
    ret += makeCopyValues(dest, src, num, 'null', null, align);
    return ret;
  },

  emscripten_memcpy_big: function(dest, src, num) {
    HEAPU8.set(HEAPU8.subarray(src, src+num), dest);
    return dest;
  },

  memcpy__asm: true,
  memcpy__sig: 'iiii',
  memcpy__deps: ['emscripten_memcpy_big'],
  memcpy: function(dest, src, num) {
    dest = dest|0; src = src|0; num = num|0;
    var ret = 0;
    var aligned_dest_end = 0;
    var block_aligned_dest_end = 0;
    var dest_end = 0;
    // Test against a benchmarked cutoff limit for when HEAPU8.set() becomes faster to use.
    if ((num|0) >=
#if SIMD
      196608
#else
      8192
#endif
    ) {
      return _emscripten_memcpy_big(dest|0, src|0, num|0)|0;
    }

    ret = dest|0;
    dest_end = (dest + num)|0;
    if ((dest&3) == (src&3)) {
      // The initial unaligned < 4-byte front.
      while (dest & 3) {
        if ((num|0) == 0) return ret|0;
        {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i8'), 'i8') }}};
        dest = (dest+1)|0;
        src = (src+1)|0;
        num = (num-1)|0;
      }
      aligned_dest_end = (dest_end & -4)|0;
      block_aligned_dest_end = (aligned_dest_end - 64)|0;
      while ((dest|0) <= (block_aligned_dest_end|0) ) {
#if SIMD
        SIMD_Int32x4_store(HEAPU8, dest, SIMD_Int32x4_load(HEAPU8, src));
        SIMD_Int32x4_store(HEAPU8, dest+16, SIMD_Int32x4_load(HEAPU8, src+16));
        SIMD_Int32x4_store(HEAPU8, dest+32, SIMD_Int32x4_load(HEAPU8, src+32));
        SIMD_Int32x4_store(HEAPU8, dest+48, SIMD_Int32x4_load(HEAPU8, src+48));
#else
        {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 4, makeGetValueAsm('src', 4, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 8, makeGetValueAsm('src', 8, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 12, makeGetValueAsm('src', 12, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 16, makeGetValueAsm('src', 16, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 20, makeGetValueAsm('src', 20, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 24, makeGetValueAsm('src', 24, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 28, makeGetValueAsm('src', 28, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 32, makeGetValueAsm('src', 32, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 36, makeGetValueAsm('src', 36, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 40, makeGetValueAsm('src', 40, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 44, makeGetValueAsm('src', 44, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 48, makeGetValueAsm('src', 48, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 52, makeGetValueAsm('src', 52, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 56, makeGetValueAsm('src', 56, 'i32'), 'i32') }}};
        {{{ makeSetValueAsm('dest', 60, makeGetValueAsm('src', 60, 'i32'), 'i32') }}};
#endif
        dest = (dest+64)|0;
        src = (src+64)|0;
      }
      while ((dest|0) < (aligned_dest_end|0) ) {
        {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i32'), 'i32') }}};
        dest = (dest+4)|0;
        src = (src+4)|0;
      }
    } else {
      // In the unaligned copy case, unroll a bit as well.
      aligned_dest_end = (dest_end - 4)|0;
      while ((dest|0) < (aligned_dest_end|0) ) {
        {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i8'), 'i8') }}};
        {{{ makeSetValueAsm('dest', 1, makeGetValueAsm('src', 1, 'i8'), 'i8') }}};
        {{{ makeSetValueAsm('dest', 2, makeGetValueAsm('src', 2, 'i8'), 'i8') }}};
        {{{ makeSetValueAsm('dest', 3, makeGetValueAsm('src', 3, 'i8'), 'i8') }}};
        dest = (dest+4)|0;
        src = (src+4)|0;
      }
    }
    // The remaining unaligned < 4 byte tail.
    while ((dest|0) < (dest_end|0)) {
      {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i8'), 'i8') }}};
      dest = (dest+1)|0;
      src = (src+1)|0;
    }
    return ret|0;
  },

  llvm_memcpy_i32: 'memcpy',
  llvm_memcpy_i64: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i32: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i64: 'memcpy',

  memmove__sig: 'iiii',
  memmove__asm: true,
  memmove__deps: ['memcpy'],
  memmove: function(dest, src, num) {
    dest = dest|0; src = src|0; num = num|0;
    var ret = 0;
    if (((src|0) < (dest|0)) & ((dest|0) < ((src + num)|0))) {
      // Unlikely case: Copy backwards in a safe manner
      ret = dest;
      src = (src + num)|0;
      dest = (dest + num)|0;
      while ((num|0) > 0) {
        dest = (dest - 1)|0;
        src = (src - 1)|0;
        num = (num - 1)|0;
        {{{ makeSetValueAsm('dest', 0, makeGetValueAsm('src', 0, 'i8'), 'i8') }}};
      }
      dest = ret;
    } else {
      _memcpy(dest, src, num) | 0;
    }
    return dest | 0;
  },
  llvm_memmove_i32: 'memmove',
  llvm_memmove_i64: 'memmove',
  llvm_memmove_p0i8_p0i8_i32: 'memmove',
  llvm_memmove_p0i8_p0i8_i64: 'memmove',

  memset__inline: function(ptr, value, num, align) {
    return makeSetValues(ptr, 0, value, 'null', num, align);
  },
  memset__sig: 'iiii',
  memset__asm: true,
  memset: function(ptr, value, num) {
    ptr = ptr|0; value = value|0; num = num|0;
    var end = 0, aligned_end = 0, block_aligned_end = 0, value4 = 0;
#if SIMD
    var value16 = SIMD_Int32x4(0,0,0,0);
#endif
    end = (ptr + num)|0;

    value = value & 0xff;
    if ((num|0) >= 67 /* 64 bytes for an unrolled loop + 3 bytes for unaligned head*/) {
      while ((ptr&3) != 0) {
        {{{ makeSetValueAsm('ptr', 0, 'value', 'i8') }}};
        ptr = (ptr+1)|0;
      }

      aligned_end = (end & -4)|0;
      block_aligned_end = (aligned_end - 64)|0;
      value4 = value | (value << 8) | (value << 16) | (value << 24);
#if SIMD
      value16 = SIMD_Int32x4_splat(value4);
#endif

      while((ptr|0) <= (block_aligned_end|0)) {
#if SIMD
        SIMD_Int32x4_store(HEAPU8, ptr, value16);
        SIMD_Int32x4_store(HEAPU8, ptr+16, value16);
        SIMD_Int32x4_store(HEAPU8, ptr+32, value16);
        SIMD_Int32x4_store(HEAPU8, ptr+48, value16);
#else
        {{{ makeSetValueAsm('ptr', 0, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 4, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 8, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 12, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 16, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 20, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 24, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 28, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 32, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 36, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 40, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 44, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 48, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 52, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 56, 'value4', 'i32') }}};
        {{{ makeSetValueAsm('ptr', 60, 'value4', 'i32') }}};
#endif
        ptr = (ptr + 64)|0;
      }

      while ((ptr|0) < (aligned_end|0) ) {
        {{{ makeSetValueAsm('ptr', 0, 'value4', 'i32') }}};
        ptr = (ptr+4)|0;
      }
    }
    // The remaining bytes.
    while ((ptr|0) < (end|0)) {
      {{{ makeSetValueAsm('ptr', 0, 'value', 'i8') }}};
      ptr = (ptr+1)|0;
    }
    return (end-num)|0;
  },
  llvm_memset_i32: 'memset',
  llvm_memset_p0i8_i32: 'memset',
  llvm_memset_p0i8_i64: 'memset',

  // ==========================================================================
  // GCC/LLVM specifics
  // ==========================================================================
  __builtin_prefetch: function(){},

  // ==========================================================================
  // LLVM specifics
  // ==========================================================================

  llvm_va_start__inline: function(ptr) {
    // varargs - we received a pointer to the varargs as a final 'extra' parameter called 'varrp'
    // 2-word structure: struct { void* start; void* currentOffset; }
    return makeSetValue(ptr, 0, 'varrp', 'void*') + ';' + makeSetValue(ptr, Runtime.QUANTUM_SIZE, 0, 'void*');
  },

  llvm_va_end: function() {},

  llvm_va_copy: function(ppdest, ppsrc) {
    // copy the list start
    {{{ makeCopyValues('ppdest', 'ppsrc', Runtime.QUANTUM_SIZE, 'null', null, 1) }}};
    
    // copy the list's current offset (will be advanced with each call to va_arg)
    {{{ makeCopyValues('(ppdest+'+Runtime.QUANTUM_SIZE+')', '(ppsrc+'+Runtime.QUANTUM_SIZE+')', Runtime.QUANTUM_SIZE, 'null', null, 1) }}};
  },

  llvm_bswap_i16__asm: true,
  llvm_bswap_i16__sig: 'ii',
  llvm_bswap_i16: function(x) {
    x = x|0;
    return (((x&0xff)<<8) | ((x>>8)&0xff))|0;
  },

  llvm_bswap_i32__asm: true,
  llvm_bswap_i32__sig: 'ii',
  llvm_bswap_i32: function(x) {
    x = x|0;
    return (((x&0xff)<<24) | (((x>>8)&0xff)<<16) | (((x>>16)&0xff)<<8) | (x>>>24))|0;
  },

  llvm_bswap_i64__deps: ['llvm_bswap_i32'],
  llvm_bswap_i64: function(l, h) {
    var retl = _llvm_bswap_i32(h)>>>0;
    var reth = _llvm_bswap_i32(l)>>>0;
    {{{ makeStructuralReturn(['retl', 'reth']) }}};
  },

  llvm_ctlz_i8__asm: true,
  llvm_ctlz_i8__sig: 'ii',
  llvm_ctlz_i8: function(x, isZeroUndef) {
    x = x | 0;
    isZeroUndef = isZeroUndef | 0;
    return (Math_clz32(x & 0xff) | 0) - 24 | 0;
  },

  llvm_ctlz_i16__asm: true,
  llvm_ctlz_i16__sig: 'ii',
  llvm_ctlz_i16: function(x, isZeroUndef) {
    x = x | 0;
    isZeroUndef = isZeroUndef | 0;
    return (Math_clz32(x & 0xffff) | 0) - 16 | 0
  },

  llvm_ctlz_i64__asm: true,
  llvm_ctlz_i64__sig: 'iii',
  llvm_ctlz_i64: function(l, h, isZeroUndef) {
    l = l | 0;
    h = h | 0;
    isZeroUndef = isZeroUndef | 0;
    var ret = 0;
    ret = Math_clz32(h) | 0;
    if ((ret | 0) == 32) ret = ret + (Math_clz32(l) | 0) | 0;
    {{{ makeSetTempRet0('0') }}};
    return ret | 0;
  },

  llvm_cttz_i32__deps: [function() {
    function cttz(x) {
      for (var i = 0; i < 8; i++) {
        if (x & (1 << i)) {
          return i;
        }
      }
      return 8;
    }
    if (SIDE_MODULE) return ''; // uses it from the parent

#if USE_PTHREADS
    return 'var cttz_i8; if (ENVIRONMENT_IS_PTHREAD) cttz_i8 = PthreadWorkerInit.cttz_i8; else PthreadWorkerInit.cttz_i8 = cttz_i8 = allocate([' + range(256).map(function(x) { return cttz(x) }).join(',') + '], "i8", ALLOC_STATIC);';
#else
    return 'var cttz_i8 = allocate([' + range(256).map(function(x) { return cttz(x) }).join(',') + '], "i8", ALLOC_STATIC);';
#endif
  }],
#if BINARYEN == 0 // binaryen will convert these calls to wasm anyhow
  llvm_cttz_i32__asm: true,
#endif
  llvm_cttz_i32__sig: 'ii',
  llvm_cttz_i32: function(x) {
    x = x|0;
    var ret = 0;
    ret = {{{ makeGetValueAsm('cttz_i8', 'x & 0xff', 'i8') }}};
    if ((ret|0) < 8) return ret|0;
    ret = {{{ makeGetValueAsm('cttz_i8', '(x >> 8)&0xff', 'i8') }}};
    if ((ret|0) < 8) return (ret + 8)|0;
    ret = {{{ makeGetValueAsm('cttz_i8', '(x >> 16)&0xff', 'i8') }}};
    if ((ret|0) < 8) return (ret + 16)|0;
    return ({{{ makeGetValueAsm('cttz_i8', 'x >>> 24', 'i8') }}} + 24)|0;
  },

  llvm_cttz_i64__deps: ['llvm_cttz_i32'],
  llvm_cttz_i64: function(l, h) {
    var ret = _llvm_cttz_i32(l);
    if (ret == 32) ret += _llvm_cttz_i32(h);
    {{{ makeStructuralReturn(['ret', '0']) }}};
  },

  llvm_ctpop_i32__asm: true,
  llvm_ctpop_i32__sig: 'ii',
  llvm_ctpop_i32: function(x) {
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    // http://bits.stephan-brumme.com/countBits.html
    x = x | 0;
    x = x - ((x >>> 1) & 0x55555555) | 0;
    x = (x & 0x33333333) + ((x >>> 2) & 0x33333333) | 0;
    return (Math_imul((x + (x >>> 4) & 252645135 /* 0xF0F0F0F, but hits uglify parse bug? */), 0x1010101) >>> 24) | 0;
  },

  llvm_ctpop_i64__deps: ['llvm_ctpop_i32'],
  llvm_ctpop_i64__asm: true,
  llvm_ctpop_i64__sig: 'iii',
  llvm_ctpop_i64: function(l, h) {
    l = l | 0;
    h = h | 0;
    return (_llvm_ctpop_i32(l) | 0) + (_llvm_ctpop_i32(h) | 0) | 0;
  },

  llvm_trap: function() {
    abort('trap!');
  },

  llvm_prefetch: function(){},

  __assert_fail: function(condition, filename, line, func) {
    ABORT = true;
    throw 'Assertion failed: ' + Pointer_stringify(condition) + ', at: ' + [filename ? Pointer_stringify(filename) : 'unknown filename', line, func ? Pointer_stringify(func) : 'unknown function'] + ' at ' + stackTrace();
  },

  __assert_func: function(filename, line, func, condition) {
    throw 'Assertion failed: ' + (condition ? Pointer_stringify(condition) : 'unknown condition') + ', at: ' + [filename ? Pointer_stringify(filename) : 'unknown filename', line, func ? Pointer_stringify(func) : 'unknown function'] + ' at ' + stackTrace();
  },

  $EXCEPTIONS: {
    last: 0,
    caught: [],
    infos: {},
    deAdjust: function(adjusted) {
      if (!adjusted || EXCEPTIONS.infos[adjusted]) return adjusted;
      for (var ptr in EXCEPTIONS.infos) {
        var info = EXCEPTIONS.infos[ptr];
        if (info.adjusted === adjusted) {
#if EXCEPTION_DEBUG
          Module.printErr('de-adjusted exception ptr ' + adjusted + ' to ' + ptr);
#endif
          return ptr;
        }
      }
#if EXCEPTION_DEBUG
      Module.printErr('no de-adjustment for unknown exception ptr ' + adjusted);
#endif
      return adjusted;
    },
    addRef: function(ptr) {
#if EXCEPTION_DEBUG
      Module.printErr('addref ' + ptr);
#endif
      if (!ptr) return;
      var info = EXCEPTIONS.infos[ptr];
      info.refcount++;
    },
    decRef: function(ptr) {
#if EXCEPTION_DEBUG
      Module.printErr('decref ' + ptr);
#endif
      if (!ptr) return;
      var info = EXCEPTIONS.infos[ptr];
      assert(info.refcount > 0);
      info.refcount--;
      // A rethrown exception can reach refcount 0; it must not be discarded
      // Its next handler will clear the rethrown flag and addRef it, prior to
      // final decRef and destruction here
      if (info.refcount === 0 && !info.rethrown) {
        if (info.destructor) {
#if WASM_BACKEND == 0
          Module['dynCall_vi'](info.destructor, ptr);
#else
          // In Wasm, destructors return 'this' as in ARM
          Module['dynCall_ii'](info.destructor, ptr);
#endif
        }
        delete EXCEPTIONS.infos[ptr];
        ___cxa_free_exception(ptr);
#if EXCEPTION_DEBUG
        Module.printErr('decref freeing exception ' + [ptr, EXCEPTIONS.last, 'stack', EXCEPTIONS.caught]);
#endif
      }
    },
    clearRef: function(ptr) {
      if (!ptr) return;
      var info = EXCEPTIONS.infos[ptr];
      info.refcount = 0;
    },
  },

  // Exceptions
  __cxa_allocate_exception__deps: ['malloc'],
  __cxa_allocate_exception: function(size) {
    return _malloc(size);
  },
  __cxa_free_exception__deps: ['free'],
  __cxa_free_exception: function(ptr) {
    try {
      return _free(ptr);
    } catch(e) { // XXX FIXME
#if ASSERTIONS
      Module.printErr('exception during cxa_free_exception: ' + e);
#endif
    }
  },
  __cxa_increment_exception_refcount__deps: ['$EXCEPTIONS'],
  __cxa_increment_exception_refcount: function(ptr) {
    EXCEPTIONS.addRef(EXCEPTIONS.deAdjust(ptr));
  },
  __cxa_decrement_exception_refcount__deps: ['$EXCEPTIONS'],
  __cxa_decrement_exception_refcount: function(ptr) {
    EXCEPTIONS.decRef(EXCEPTIONS.deAdjust(ptr));
  },
  // Here, we throw an exception after recording a couple of values that we need to remember
  // We also remember that it was the last exception thrown as we need to know that later.
  __cxa_throw__sig: 'viii',
  __cxa_throw__deps: ['_ZSt18uncaught_exceptionv', '__cxa_find_matching_catch', '$EXCEPTIONS'],
  __cxa_throw: function(ptr, type, destructor) {
#if EXCEPTION_DEBUG
    Module.printErr('Compiled code throwing an exception, ' + [ptr,type,destructor]);
#endif
    EXCEPTIONS.infos[ptr] = {
      ptr: ptr,
      adjusted: ptr,
      type: type,
      destructor: destructor,
      refcount: 0,
      caught: false,
      rethrown: false
    };
    EXCEPTIONS.last = ptr;
    if (!("uncaught_exception" in __ZSt18uncaught_exceptionv)) {
      __ZSt18uncaught_exceptionv.uncaught_exception = 1;
    } else {
      __ZSt18uncaught_exceptionv.uncaught_exception++;
    }
    {{{ makeThrow('ptr') }}}
  },
  // This exception will be caught twice, but while begin_catch runs twice,
  // we early-exit from end_catch when the exception has been rethrown, so
  // pop that here from the caught exceptions.
  __cxa_rethrow__deps: ['__cxa_end_catch', '$EXCEPTIONS'],
  __cxa_rethrow: function() {
    var ptr = EXCEPTIONS.caught.pop();
    if (!EXCEPTIONS.infos[ptr].rethrown) {
      // Only pop if the corresponding push was through rethrow_primary_exception
      EXCEPTIONS.caught.push(ptr)
      EXCEPTIONS.infos[ptr].rethrown = true;
    }
#if EXCEPTION_DEBUG
    Module.printErr('Compiled code RE-throwing an exception, popped ' + [ptr, EXCEPTIONS.last, 'stack', EXCEPTIONS.caught]);
#endif
    EXCEPTIONS.last = ptr;
    {{{ makeThrow('ptr') }}}
  },
  llvm_eh_exception__deps: ['$EXCEPTIONS'],
  llvm_eh_exception: function() {
    return EXCEPTIONS.last;
  },
  llvm_eh_selector__jsargs: true,
  llvm_eh_selector__deps: ['$EXCEPTIONS'],
  llvm_eh_selector: function(unused_exception_value, personality/*, varargs*/) {
    var type = EXCEPTIONS.last;
    for (var i = 2; i < arguments.length; i++) {
      if (arguments[i] ==  type) return type;
    }
    return 0;
  },
  llvm_eh_typeid_for: function(type) {
    return type;
  },
  __cxa_begin_catch__deps: ['_ZSt18uncaught_exceptionv', '$EXCEPTIONS'],
  __cxa_begin_catch: function(ptr) {
    var info = EXCEPTIONS.infos[ptr];
    if (info && !info.caught) {
      info.caught = true;
      __ZSt18uncaught_exceptionv.uncaught_exception--;
    }
    if (info) info.rethrown = false;
    EXCEPTIONS.caught.push(ptr);
#if EXCEPTION_DEBUG
		Module.printErr('cxa_begin_catch ' + [ptr, 'stack', EXCEPTIONS.caught]);
#endif
    EXCEPTIONS.addRef(EXCEPTIONS.deAdjust(ptr));
    return ptr;
  },
  // We're done with a catch. Now, we can run the destructor if there is one
  // and free the exception. Note that if the dynCall on the destructor fails
  // due to calling apply on undefined, that means that the destructor is
  // an invalid index into the FUNCTION_TABLE, so something has gone wrong.
  __cxa_end_catch__deps: ['__cxa_free_exception', '$EXCEPTIONS'],
  __cxa_end_catch: function() {
    // Clear state flag.
    Module['setThrew'](0);
    // Call destructor if one is registered then clear it.
    var ptr = EXCEPTIONS.caught.pop();
#if EXCEPTION_DEBUG
    Module.printErr('cxa_end_catch popped ' + [ptr, EXCEPTIONS.last, 'stack', EXCEPTIONS.caught]);
#endif
    if (ptr) {
      EXCEPTIONS.decRef(EXCEPTIONS.deAdjust(ptr));
      EXCEPTIONS.last = 0; // XXX in decRef?
    }
  },
  __cxa_get_exception_ptr: function(ptr) {
#if EXCEPTION_DEBUG
    Module.printErr('cxa_get_exception_ptr ' + ptr);
#endif
    // TODO: use info.adjusted?
    return ptr;
  },
  _ZSt18uncaught_exceptionv: function() { // std::uncaught_exception()
    return !!__ZSt18uncaught_exceptionv.uncaught_exception;
  },
  __cxa_uncaught_exception__deps: ['_ZSt18uncaught_exceptionv'],
  __cxa_uncaught_exception: function() {
    return !!__ZSt18uncaught_exceptionv.uncaught_exception;
  },

  __cxa_call_unexpected: function(exception) {
    Module.printErr('Unexpected exception thrown, this is not properly supported - aborting');
    ABORT = true;
    throw exception;
  },

  __cxa_current_primary_exception: function() {
    var ret = EXCEPTIONS.caught[EXCEPTIONS.caught.length-1] || 0;
    if (ret) EXCEPTIONS.addRef(EXCEPTIONS.deAdjust(ret));
    return ret;
  },

  __cxa_rethrow_primary_exception__deps: ['__cxa_rethrow'],
  __cxa_rethrow_primary_exception: function(ptr) {
    if (!ptr) return;
    EXCEPTIONS.caught.push(ptr);
    EXCEPTIONS.infos[ptr].rethrown = true;
    ___cxa_rethrow();
  },

  terminate: '__cxa_call_unexpected',

  __gxx_personality_v0__deps: ['_ZSt18uncaught_exceptionv', '__cxa_find_matching_catch'],
  __gxx_personality_v0: function() {
  },

  __gcc_personality_v0: function() {
  },

  // Finds a suitable catch clause for when an exception is thrown.
  // In normal compilers, this functionality is handled by the C++
  // 'personality' routine. This is passed a fairly complex structure
  // relating to the context of the exception and makes judgements
  // about how to handle it. Some of it is about matching a suitable
  // catch clause, and some of it is about unwinding. We already handle
  // unwinding using 'if' blocks around each function, so the remaining
  // functionality boils down to picking a suitable 'catch' block.
  // We'll do that here, instead, to keep things simpler.

  __cxa_find_matching_catch__deps: ['__resumeException', '$EXCEPTIONS'],
  __cxa_find_matching_catch: function() {
    var thrown = EXCEPTIONS.last;
    if (!thrown) {
      // just pass through the null ptr
      {{{ makeStructuralReturn([0, 0]) }}};
    }
    var info = EXCEPTIONS.infos[thrown];
    var throwntype = info.type;
    if (!throwntype) {
      // just pass through the thrown ptr
      {{{ makeStructuralReturn(['thrown', 0]) }}};
    }
    var typeArray = Array.prototype.slice.call(arguments);

    var pointer = Module['___cxa_is_pointer_type'](throwntype);
    // can_catch receives a **, add indirection
    if (!___cxa_find_matching_catch.buffer) ___cxa_find_matching_catch.buffer = _malloc(4);
#if EXCEPTION_DEBUG
    Module.print("can_catch on " + [thrown]);
#endif
    {{{ makeSetValue('___cxa_find_matching_catch.buffer', '0', 'thrown', '*') }}};
    thrown = ___cxa_find_matching_catch.buffer;
    // The different catch blocks are denoted by different types.
    // Due to inheritance, those types may not precisely match the
    // type of the thrown object. Find one which matches, and
    // return the type of the catch block which should be called.
    for (var i = 0; i < typeArray.length; i++) {
      if (typeArray[i] && Module['___cxa_can_catch'](typeArray[i], throwntype, thrown)) {
        thrown = {{{ makeGetValue('thrown', '0', '*') }}}; // undo indirection
        info.adjusted = thrown;
#if EXCEPTION_DEBUG
        Module.print("  can_catch found " + [thrown, typeArray[i]]);
#endif
        {{{ makeStructuralReturn(['thrown', 'typeArray[i]']) }}};
      }
    }
    // Shouldn't happen unless we have bogus data in typeArray
    // or encounter a type for which emscripten doesn't have suitable
    // typeinfo defined. Best-efforts match just in case.
    thrown = {{{ makeGetValue('thrown', '0', '*') }}}; // undo indirection
    {{{ makeStructuralReturn(['thrown', 'throwntype']) }}};
  },

  __resumeException__deps: ['$EXCEPTIONS', function() { Functions.libraryFunctions['___resumeException'] = 1 }], // will be called directly from compiled code
  __resumeException: function(ptr) {
#if EXCEPTION_DEBUG
    Module.print("Resuming exception " + [ptr, EXCEPTIONS.last]);
#endif
    if (!EXCEPTIONS.last) { EXCEPTIONS.last = ptr; }
    {{{ makeThrow('ptr') }}}
  },

  llvm_stacksave: function() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
      self.LLVM_SAVEDSTACKS = [];
    }
    self.LLVM_SAVEDSTACKS.push(Runtime.stackSave());
    return self.LLVM_SAVEDSTACKS.length-1;
  },
  llvm_stackrestore: function(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    Runtime.stackRestore(ret);
  },

  __cxa_pure_virtual: function() {
    ABORT = true;
    throw 'Pure virtual function called!';
  },

  llvm_flt_rounds: function() {
    return -1; // 'indeterminable' for FLT_ROUNDS
  },

  llvm_expect_i32__inline: function(val, expected) {
    return '(' + val + ')';
  },

  llvm_lifetime_start: function() {},
  llvm_lifetime_end: function() {},

  llvm_invariant_start: function() {},
  llvm_invariant_end: function() {},

  llvm_objectsize_i32: function() { return -1 }, // TODO: support this

  llvm_dbg_declare__inline: function() { throw 'llvm_debug_declare' }, // avoid warning

  llvm_bitreverse_i32__asm: true,
  llvm_bitreverse_i32__sig: 'ii',
  llvm_bitreverse_i32: function(x) {
    x = x|0;
    x = ((x & 0xaaaaaaaa) >>> 1) | ((x & 0x55555555) << 1);
    x = ((x & 0xcccccccc) >>> 2) | ((x & 0x33333333) << 2);
    x = ((x & 0xf0f0f0f0) >>> 4) | ((x & 0x0f0f0f0f) << 4);
    x = ((x & 0xff00ff00) >>> 8) | ((x & 0x00ff00ff) << 8);
    return (x >>> 16) | (x << 16);
  },

  // llvm-nacl

  llvm_nacl_atomic_store_i32__inline: true,

  llvm_nacl_atomic_cmpxchg_i8__inline: true,
  llvm_nacl_atomic_cmpxchg_i16__inline: true,
  llvm_nacl_atomic_cmpxchg_i32__inline: true,

  // ==========================================================================
  // llvm-mono integration
  // ==========================================================================

  llvm_mono_load_i8_p0i8: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i8') }}};
  },

  llvm_mono_store_i8_p0i8: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i8') }}};
  },

  llvm_mono_load_i16_p0i16: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i16') }}};
  },

  llvm_mono_store_i16_p0i16: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i16') }}};
  },

  llvm_mono_load_i32_p0i32: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i32') }}};
  },

  llvm_mono_store_i32_p0i32: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i32') }}};
  },

  // ==========================================================================
  // math.h
  // ==========================================================================

  cos: 'Math_cos',
  cosf: 'Math_cos',
  cosl: 'Math_cos',
  sin: 'Math_sin',
  sinf: 'Math_sin',
  sinl: 'Math_sin',
  tan: 'Math_tan',
  tanf: 'Math_tan',
  tanl: 'Math_tan',
  acos: 'Math_acos',
  acosf: 'Math_acos',
  acosl: 'Math_acos',
  asin: 'Math_asin',
  asinf: 'Math_asin',
  asinl: 'Math_asin',
  atan: 'Math_atan',
  atanf: 'Math_atan',
  atanl: 'Math_atan',
  atan2: 'Math_atan2',
  atan2f: 'Math_atan2',
  atan2l: 'Math_atan2',
  exp: 'Math_exp',
  expf: 'Math_exp',
  expl: 'Math_exp',
  log: 'Math_log',
  logf: 'Math_log',
  logl: 'Math_log',
  sqrt: 'Math_sqrt',
  sqrtf: 'Math_sqrt',
  sqrtl: 'Math_sqrt',
  fabs: 'Math_abs',
  fabsf: 'Math_abs',
  fabsl: 'Math_abs',
  llvm_fabs_f32: 'Math_abs',
  llvm_fabs_f64: 'Math_abs',
  ceil: 'Math_ceil',
  ceilf: 'Math_ceil',
  ceill: 'Math_ceil',
  floor: 'Math_floor',
  floorf: 'Math_floor',
  floorl: 'Math_floor',
  pow: 'Math_pow',
  powf: 'Math_pow',
  powl: 'Math_pow',
  llvm_sqrt_f32: 'Math_sqrt',
  llvm_sqrt_f64: 'Math_sqrt',
  llvm_pow_f32: 'Math_pow',
  llvm_pow_f64: 'Math_pow',
  llvm_powi_f32: 'Math_pow',
  llvm_powi_f64: 'Math_pow',
  llvm_log_f32: 'Math_log',
  llvm_log_f64: 'Math_log',
  llvm_exp_f32: 'Math_exp',
  llvm_exp_f64: 'Math_exp',
  llvm_cos_f32: 'Math_cos',
  llvm_cos_f64: 'Math_cos',
  llvm_sin_f32: 'Math_sin',
  llvm_sin_f64: 'Math_sin',
  llvm_trunc_f32: 'Math_trunc',
  llvm_trunc_f64: 'Math_trunc',
  llvm_ceil_f32: 'Math_ceil',
  llvm_ceil_f64: 'Math_ceil',
  llvm_floor_f32: 'Math_floor',
  llvm_floor_f64: 'Math_floor',
  llvm_round_f32: 'Math_round',
  llvm_round_f64: 'Math_round',

  llvm_exp2_f32: function(x) {
    return Math.pow(2, x);
  },
  llvm_exp2_f64: 'llvm_exp2_f32',

  llvm_log2_f32: function(x) {
    return Math.log(x) / Math.LN2; // TODO: Math.log2, when browser support is there
  },
  llvm_log2_f64: 'llvm_log2_f32',

  llvm_log10_f32: function(x) {
    return Math.log(x) / Math.LN10; // TODO: Math.log10, when browser support is there
  },
  llvm_log10_f64: 'llvm_log10_f32',

  llvm_copysign_f32: function(x, y) {
    return y < 0 || (y === 0 && 1/y < 0) ? -Math_abs(x) : Math_abs(x);
  },

  llvm_copysign_f64: function(x, y) {
    return y < 0 || (y === 0 && 1/y < 0) ? -Math_abs(x) : Math_abs(x);
  },

  round__asm: true,
  round__sig: 'dd',
  round: function(d) {
    d = +d;
    return d >= +0 ? +Math_floor(d + +0.5) : +Math_ceil(d - +0.5);
  },

  roundf__asm: true,
  roundf__sig: 'dd',
  roundf: function(f) {
    f = +f;
    return f >= +0 ? +Math_floor(f + +0.5) : +Math_ceil(f - +0.5); // TODO: use fround?
  },

  _reallyNegative: function(x) {
    return x < 0 || (x === 0 && (1/x) === -Infinity);
  },

  // ==========================================================================
  // dlfcn.h - Dynamic library loading
  //
  // Some limitations:
  //
  //  * Minification on each file separately may not work, as they will
  //    have different shortened names. You can in theory combine them, then
  //    minify, then split... perhaps.
  //
  //  * LLVM optimizations may fail. If the child wants to access a function
  //    in the parent, LLVM opts may remove it from the parent when it is
  //    being compiled. Not sure how to tell LLVM to not do so.
  // ==========================================================================

  $DLFCN: {
    error: null,
    errorMsg: null,
    loadedLibs: {}, // handle -> [refcount, name, lib_object]
    loadedLibNames: {}, // name -> handle
  },
  // void* dlopen(const char* filename, int flag);
  dlopen__deps: ['$DLFCN', '$FS', '$ENV'],
  dlopen: function(filename, flag) {
#if MAIN_MODULE == 0
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/kripken/emscripten/wiki/Linking");
#endif
    // void *dlopen(const char *file, int mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlopen.html
    var searchpaths = [];
    if (filename === 0) {
      filename = '__self__';
    } else {
      var strfilename = Pointer_stringify(filename);
      var isValidFile = function (filename) {
        var target = FS.findObject(filename);
        return target && !target.isFolder && !target.isDevice;
      };

      if (isValidFile(strfilename)) {
        filename = strfilename;
      } else {
        if (ENV['LD_LIBRARY_PATH']) {
          searchpaths = ENV['LD_LIBRARY_PATH'].split(':');
        }

        for (var ident in searchpaths) {
          var searchfile = PATH.join2(searchpaths[ident],strfilename);
          if (isValidFile(searchfile)) {
            filename = searchfile;
            break;
          }
        }
      }
    }

    if (DLFCN.loadedLibNames[filename]) {
      // Already loaded; increment ref count and return.
      var handle = DLFCN.loadedLibNames[filename];
      DLFCN.loadedLibs[handle].refcount++;
      return handle;
    }

    if (filename === '__self__') {
      var handle = -1;
      var lib_module = Module;
      var cached_functions = {};
    } else {
      var target = FS.findObject(filename);
      if (!target || target.isFolder || target.isDevice) {
        DLFCN.errorMsg = 'Could not find dynamic lib: ' + filename;
        return 0;
      }
      FS.forceLoadFile(target);

      var lib_module;
      try {
#if BINARYEN
        // the shared library is a shared wasm library (see tools/shared.py WebAssembly.make_shared_library)
        var lib_data = FS.readFile(filename, { encoding: 'binary' });
        if (!(lib_data instanceof Uint8Array)) lib_data = new Uint8Array(lib_data);
        //Module.printErr('libfile ' + filename + ' size: ' + lib_data.length);
        lib_module = Runtime.loadWebAssemblyModule(lib_data);
#else
        // the shared library is a JS file, which we eval
        var lib_data = FS.readFile(filename, { encoding: 'utf8' });
        lib_module = eval(lib_data)(
          Runtime.alignFunctionTables(),
          Module
        );
#endif
      } catch (e) {
#if ASSERTIONS
        Module.printErr('Error in loading dynamic library: ' + e);
#endif
        DLFCN.errorMsg = 'Could not evaluate dynamic lib: ' + filename + '\n' + e;
        return 0;
      }

      // Not all browsers support Object.keys().
      var handle = 1;
      for (var key in DLFCN.loadedLibs) {
        if (DLFCN.loadedLibs.hasOwnProperty(key)) handle++;
      }

      // We don't care about RTLD_NOW and RTLD_LAZY.
      if (flag & 256) { // RTLD_GLOBAL
        for (var ident in lib_module) {
          if (lib_module.hasOwnProperty(ident)) {
            // When RTLD_GLOBAL is enable, the symbols defined by this shared object will be made
            // available for symbol resolution of subsequently loaded shared objects.
            //
            // We should copy the symbols (which include methods and variables) from SIDE_MODULE to MAIN_MODULE.
            //
            // Module of SIDE_MODULE has not only the symbols (which should be copied)
            // but also others (print*, asmGlobal*, FUNCTION_TABLE_**, NAMED_GLOBALS, and so on).
            //
            // When the symbol (which should be copied) is method, Module._* 's type becomes function.
            // When the symbol (which should be copied) is variable, Module._* 's type becomes number.
            //
            // Except for the symbol prefix (_), there is no difference in the symbols (which should be copied) and others.
            // So this just copies over compiled symbols (which start with _).
            if (ident[0] == '_') {
              Module[ident] = lib_module[ident];
            }
          }
        }
      }

      var cached_functions = {};
    }
    DLFCN.loadedLibs[handle] = {
      refcount: 1,
      name: filename,
      module: lib_module,
      cached_functions: cached_functions
    };
    DLFCN.loadedLibNames[filename] = handle;

    return handle;
  },
  // int dlclose(void* handle);
  dlclose__deps: ['$DLFCN'],
  dlclose: function(handle) {
    // int dlclose(void *handle);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlclose.html
    if (!DLFCN.loadedLibs[handle]) {
      DLFCN.errorMsg = 'Tried to dlclose() unopened handle: ' + handle;
      return 1;
    } else {
      var lib_record = DLFCN.loadedLibs[handle];
      if (--lib_record.refcount == 0) {
        if (lib_record.module.cleanups) {
          lib_record.module.cleanups.forEach(function(cleanup) { cleanup() });
        }
        delete DLFCN.loadedLibNames[lib_record.name];
        delete DLFCN.loadedLibs[handle];
      }
      return 0;
    }
  },
  // void* dlsym(void* handle, const char* symbol);
  dlsym__deps: ['$DLFCN'],
  dlsym: function(handle, symbol) {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = Pointer_stringify(symbol);

    if (!DLFCN.loadedLibs[handle]) {
      DLFCN.errorMsg = 'Tried to dlsym() from an unopened handle: ' + handle;
      return 0;
    } else {
      var lib = DLFCN.loadedLibs[handle];
      symbol = '_' + symbol;
      if (lib.cached_functions.hasOwnProperty(symbol)) {
        return lib.cached_functions[symbol];
      }
      if (!lib.module.hasOwnProperty(symbol)) {
        DLFCN.errorMsg = ('Tried to lookup unknown symbol "' + symbol +
                               '" in dynamic lib: ' + lib.name);
        return 0;
      } else {
        var result = lib.module[symbol];
        if (typeof result == 'function') {
          result = Runtime.addFunction(result);
          //Module.printErr('adding function dlsym result for ' + symbol + ' => ' + result);
          lib.cached_functions = result;
        }
        return result;
      }
    }
  },
  // char* dlerror(void);
  dlerror__deps: ['$DLFCN'],
  dlerror: function() {
    // char *dlerror(void);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlerror.html
    if (DLFCN.errorMsg === null) {
      return 0;
    } else {
      if (DLFCN.error) _free(DLFCN.error);
      var msgArr = intArrayFromString(DLFCN.errorMsg);
      DLFCN.error = allocate(msgArr, 'i8', ALLOC_NORMAL);
      DLFCN.errorMsg = null;
      return DLFCN.error;
    }
  },

  dladdr: function(addr, info) {
    // report all function pointers as coming from this program itself XXX not really correct in any way
    var fname = allocate(intArrayFromString(Module['thisProgram'] || './this.program'), 'i8', ALLOC_NORMAL); // XXX leak
    {{{ makeSetValue('addr', 0, 'fname', 'i32') }}};
    {{{ makeSetValue('addr', QUANTUM_SIZE, '0', 'i32') }}};
    {{{ makeSetValue('addr', QUANTUM_SIZE*2, '0', 'i32') }}};
    {{{ makeSetValue('addr', QUANTUM_SIZE*3, '0', 'i32') }}};
    return 1;
  },

  // ==========================================================================
  // pwd.h
  // ==========================================================================

  // TODO: Implement.
  // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/pwd.h.html
  getpwuid: function(uid) {
    return 0; // NULL
  },


  // ==========================================================================
  // time.h
  // ==========================================================================

  clock: function() {
    if (_clock.start === undefined) _clock.start = Date.now();
    return ((Date.now() - _clock.start) * ({{{ cDefine('CLOCKS_PER_SEC') }}} / 1000))|0;
  },

  time: function(ptr) {
    var ret = (Date.now()/1000)|0;
    if (ptr) {
      {{{ makeSetValue('ptr', 0, 'ret', 'i32') }}};
    }
    return ret;
  },

  difftime: function(time1, time0) {
    return time1 - time0;
  },

  // Statically allocated time struct.
#if USE_PTHREADS
  __tm_current: '; if (ENVIRONMENT_IS_PTHREAD) ___tm_current = PthreadWorkerInit.___tm_current; else PthreadWorkerInit.___tm_current = ___tm_current = allocate({{{ C_STRUCTS.tm.__size__ }}}, "i8", ALLOC_STATIC)',
  __tm_timezone: '; if (ENVIRONMENT_IS_PTHREAD) ___tm_timezone = PthreadWorkerInit.___tm_timezone; else PthreadWorkerInit.___tm_timezone = ___tm_timezone = allocate(intArrayFromString("GMT"), "i8", ALLOC_STATIC)',
  __tm_formatted: '; if (ENVIRONMENT_IS_PTHREAD) ___tm_formatted = PthreadWorkerInit.___tm_formatted; else PthreadWorkerInit.___tm_formatted = ___tm_formatted = allocate({{{ C_STRUCTS.tm.__size__ }}}, "i8", ALLOC_STATIC)',
#else
  __tm_current: '{{{ makeStaticAlloc(C_STRUCTS.tm.__size__) }}}',
  // Statically allocated copy of the string "GMT" for gmtime() to point to
  __tm_timezone: 'allocate(intArrayFromString("GMT"), "i8", ALLOC_STATIC)',
  // Statically allocated time strings.
  __tm_formatted: '{{{ makeStaticAlloc(C_STRUCTS.tm.__size__) }}}',
#endif
  mktime__deps: ['tzset'],
  mktime: function(tmPtr) {
    _tzset();
    var date = new Date({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);

    // There's an ambiguous hour when the time goes back; the tm_isdst field is
    // used to disambiguate it.  Date() basically guesses, so we fix it up if it
    // guessed wrong, or fill in tm_isdst with the guess if it's -1.
    var dst = {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'i32') }}};
    var guessedOffset = date.getTimezoneOffset();
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(2000, 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
    if (dst < 0) {
      {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'Number(dstOffset == guessedOffset)', 'i32') }}};
    } else if ((dst > 0) != (dstOffset == guessedOffset)) {
      var nonDstOffset = Math.max(winterOffset, summerOffset);
      var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
      // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
      date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
    }

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};
    var yday = ((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};

    return (date.getTime() / 1000)|0;
  },
  timelocal: 'mktime',

  gmtime__deps: ['__tm_current', 'gmtime_r'],
  gmtime: function(time) {
    return _gmtime_r(time, ___tm_current);
  },

  gmtime_r__deps: ['__tm_timezone'],
  gmtime_r: function(time, tmPtr) {
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getUTCSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getUTCMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getUTCHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getUTCDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getUTCMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getUTCFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '0', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, '0', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_zone, '___tm_timezone', 'i32') }}};

    return tmPtr;
  },
  timegm__deps: ['tzset'],
  timegm: function(tmPtr) {
    _tzset();
    var time = Date.UTC({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);
    var date = new Date(time);

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};

    return (date.getTime() / 1000)|0;
  },

  localtime__deps: ['__tm_current', 'localtime_r'],
  localtime: function(time) {
    return _localtime_r(time, ___tm_current);
  },

  localtime_r__deps: ['__tm_timezone', 'tzset'],
  localtime_r: function(time, tmPtr) {
    _tzset();
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = ((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '-(date.getTimezoneOffset() * 60)', 'i32') }}};

    // DST is in December in South
    var summerOffset = new Date(2000, 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dst = (date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'dst', 'i32') }}};

    var zonePtr = {{{ makeGetValue(makeGlobalUse('_tzname'), 'dst ? Runtime.QUANTUM_SIZE : 0', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_zone, 'zonePtr', 'i32') }}};

    return tmPtr;
  },

  asctime__deps: ['__tm_formatted', 'asctime_r'],
  asctime: function(tmPtr) {
    return _asctime_r(tmPtr, ___tm_formatted);
  },

  asctime_r__deps: ['__tm_formatted', 'mktime'],
  asctime_r: function(tmPtr, buf) {
    var date = {
      tm_sec: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'i32') }}}
    };
    var days = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
    var months = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];
    var s = days[date.tm_wday] + ' ' + months[date.tm_mon] +
        (date.tm_mday < 10 ? '  ' : ' ') + date.tm_mday +
        (date.tm_hour < 10 ? ' 0' : ' ') + date.tm_hour +
        (date.tm_min < 10 ? ':0' : ':') + date.tm_min +
        (date.tm_sec < 10 ? ':0' : ':') + date.tm_sec +
        ' ' + (1900 + date.tm_year) + "\n";

    // asctime_r is specced to behave in an undefined manner if the algorithm would attempt
    // to write out more than 26 bytes (including the null terminator).
    // See http://pubs.opengroup.org/onlinepubs/9699919799/functions/asctime.html
    // Our undefined behavior is to truncate the write to at most 26 bytes, including null terminator.
    stringToUTF8(s, buf, 26);
    return buf;
  },

  ctime__deps: ['__tm_current', 'ctime_r'],
  ctime: function(timer) {
    return _ctime_r(timer, ___tm_current);
  },

  ctime_r__deps: ['localtime_r', 'asctime_r'],
  ctime_r: function(time, buf) {
    var stack = Runtime.stackSave();
    var rv = _asctime_r(_localtime_r(time, Runtime.stackAlloc({{{ C_STRUCTS.tm.__size__ }}})), buf);
    Runtime.stackRestore(stack);
    return rv;
  },

  dysize: function(year) {
    var leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
    return leap ? 366 : 365;
  },

  // TODO: Initialize these to defaults on startup from system settings.
  // Note: glibc has one fewer underscore for all of these. Also used in other related functions (timegm)
#if USE_PTHREADS
  tzname: '; if (ENVIRONMENT_IS_PTHREAD) _tzname = PthreadWorkerInit._tzname; else PthreadWorkerInit._tzname = _tzname = allocate({{{ 2*Runtime.QUANTUM_SIZE }}}, "i32*", ALLOC_STATIC)',
  daylight: '; if (ENVIRONMENT_IS_PTHREAD) _daylight = PthreadWorkerInit._daylight; else PthreadWorkerInit._daylight = _daylight = allocate(1, "i32*", ALLOC_STATIC)',
  timezone: '; if (ENVIRONMENT_IS_PTHREAD) _timezone = PthreadWorkerInit._timezone; else PthreadWorkerInit._timezone = _timezone = allocate(1, "i32*", ALLOC_STATIC)',
#else
  tzname: '{{{ makeStaticAlloc(2*Runtime.QUANTUM_SIZE) }}}',
  daylight: '{{{ makeStaticAlloc(1) }}}',
  timezone: '{{{ makeStaticAlloc(1) }}}',
#endif
  tzset__deps: ['tzname', 'daylight', 'timezone'],
  tzset: function() {
#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) return _emscripten_sync_run_in_main_thread_0({{{ cDefine('EM_PROXIED_TZSET') }}});
#endif
    // TODO: Use (malleable) environment variables instead of system settings.
    if (_tzset.called) return;
    _tzset.called = true;

    {{{ makeSetValue(makeGlobalUse('_timezone'), '0', '-(new Date()).getTimezoneOffset() * 60', 'i32') }}};

    var winter = new Date(2000, 0, 1);
    var summer = new Date(2000, 6, 1);
    {{{ makeSetValue(makeGlobalUse('_daylight'), '0', 'Number(winter.getTimezoneOffset() != summer.getTimezoneOffset())', 'i32') }}};

    function extractZone(date) {
      var match = date.toTimeString().match(/\(([A-Za-z ]+)\)$/);
      return match ? match[1] : "GMT";
    };
    var winterName = extractZone(winter);
    var summerName = extractZone(summer);
    var winterNamePtr = allocate(intArrayFromString(winterName), 'i8', ALLOC_NORMAL);
    var summerNamePtr = allocate(intArrayFromString(summerName), 'i8', ALLOC_NORMAL);
    if (summer.getTimezoneOffset() < winter.getTimezoneOffset()) {
      // Northern hemisphere
      {{{ makeSetValue(makeGlobalUse('_tzname'), '0', 'winterNamePtr', 'i32') }}};
      {{{ makeSetValue(makeGlobalUse('_tzname'), Runtime.QUANTUM_SIZE, 'summerNamePtr', 'i32') }}};
    } else {
      {{{ makeSetValue(makeGlobalUse('_tzname'), '0', 'summerNamePtr', 'i32') }}};
      {{{ makeSetValue(makeGlobalUse('_tzname'), Runtime.QUANTUM_SIZE, 'winterNamePtr', 'i32') }}};
    }
  },

  stime__deps: ['$ERRNO_CODES', '__setErrNo'],
  stime: function(when) {
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  __map_file__deps: ['$ERRNO_CODES', '__setErrNo'],
  __map_file: function(pathname, size) {
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  _MONTH_DAYS_REGULAR: [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  _MONTH_DAYS_LEAP: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],

  _isLeapYear: function(year) {
      return year%4 === 0 && (year%100 !== 0 || year%400 === 0);
  },

  _arraySum: function(array, index) {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]);
    return sum;
  },

  _addDays__deps: ['_isLeapYear', '_MONTH_DAYS_LEAP', '_MONTH_DAYS_REGULAR'],
  _addDays: function(date, days) {
    var newDate = new Date(date.getTime());
    while(days > 0) {
      var leap = __isLeapYear(newDate.getFullYear());
      var currentMonth = newDate.getMonth();
      var daysInCurrentMonth = (leap ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[currentMonth];

      if (days > daysInCurrentMonth-newDate.getDate()) {
        // we spill over to next month
        days -= (daysInCurrentMonth-newDate.getDate()+1);
        newDate.setDate(1);
        if (currentMonth < 11) {
          newDate.setMonth(currentMonth+1)
        } else {
          newDate.setMonth(0);
          newDate.setFullYear(newDate.getFullYear()+1);
        }
      } else {
        // we stay in current month 
        newDate.setDate(newDate.getDate()+days);
        return newDate;
      }
    }

    return newDate;
  },

  strftime__deps: ['_isLeapYear', '_arraySum', '_addDays', '_MONTH_DAYS_REGULAR', '_MONTH_DAYS_LEAP'],
  strftime: function(s, maxsize, format, tm) {
    // size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strftime.html

    var tm_zone = {{{ makeGetValue('tm', C_STRUCTS.tm.tm_zone, 'i32') }}};

    var date = {
      tm_sec: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_wday, 'i32') }}},
      tm_yday: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_yday, 'i32') }}},
      tm_isdst: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_isdst, 'i32') }}},
      tm_gmtoff: {{{ makeGetValue('tm', C_STRUCTS.tm.tm_gmtoff, 'i32') }}},
      tm_zone: tm_zone ? Pointer_stringify(tm_zone) : ''
    };

    var pattern = Pointer_stringify(format);

    // expand format
    var EXPANSION_RULES_1 = {
      '%c': '%a %b %d %H:%M:%S %Y',     // Replaced by the locale's appropriate date and time representation - e.g., Mon Aug  3 14:02:01 2013
      '%D': '%m/%d/%y',                 // Equivalent to %m / %d / %y
      '%F': '%Y-%m-%d',                 // Equivalent to %Y - %m - %d
      '%h': '%b',                       // Equivalent to %b
      '%r': '%I:%M:%S %p',              // Replaced by the time in a.m. and p.m. notation
      '%R': '%H:%M',                    // Replaced by the time in 24-hour notation
      '%T': '%H:%M:%S',                 // Replaced by the time
      '%x': '%m/%d/%y',                 // Replaced by the locale's appropriate date representation
      '%X': '%H:%M:%S'                  // Replaced by the locale's appropriate date representation
    };
    for (var rule in EXPANSION_RULES_1) {
      pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_1[rule]);
    }

    var WEEKDAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    var MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];

    function leadingSomething(value, digits, character) {
      var str = typeof value === 'number' ? value.toString() : (value || '');
      while (str.length < digits) {
        str = character[0]+str;
      }
      return str;
    };

    function leadingNulls(value, digits) {
      return leadingSomething(value, digits, '0');
    };

    function compareByDay(date1, date2) {
      function sgn(value) {
        return value < 0 ? -1 : (value > 0 ? 1 : 0);
      };

      var compare;
      if ((compare = sgn(date1.getFullYear()-date2.getFullYear())) === 0) {
        if ((compare = sgn(date1.getMonth()-date2.getMonth())) === 0) {
          compare = sgn(date1.getDate()-date2.getDate());
        }
      }
      return compare;
    };

    function getFirstWeekStartDate(janFourth) {
        switch (janFourth.getDay()) {
          case 0: // Sunday
            return new Date(janFourth.getFullYear()-1, 11, 29);
          case 1: // Monday
            return janFourth;
          case 2: // Tuesday
            return new Date(janFourth.getFullYear(), 0, 3);
          case 3: // Wednesday
            return new Date(janFourth.getFullYear(), 0, 2);
          case 4: // Thursday
            return new Date(janFourth.getFullYear(), 0, 1);
          case 5: // Friday
            return new Date(janFourth.getFullYear()-1, 11, 31);
          case 6: // Saturday
            return new Date(janFourth.getFullYear()-1, 11, 30);
        }
    };

    function getWeekBasedYear(date) {
        var thisDate = __addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);

        var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
        var janFourthNextYear = new Date(thisDate.getFullYear()+1, 0, 4);

        var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
        var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);

        if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
          // this date is after the start of the first week of this year
          if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
            return thisDate.getFullYear()+1;
          } else {
            return thisDate.getFullYear();
          }
        } else { 
          return thisDate.getFullYear()-1;
        }
    };

    var EXPANSION_RULES_2 = {
      '%a': function(date) {
        return WEEKDAYS[date.tm_wday].substring(0,3);
      },
      '%A': function(date) {
        return WEEKDAYS[date.tm_wday];
      },
      '%b': function(date) {
        return MONTHS[date.tm_mon].substring(0,3);
      },
      '%B': function(date) {
        return MONTHS[date.tm_mon];
      },
      '%C': function(date) {
        var year = date.tm_year+1900;
        return leadingNulls((year/100)|0,2);
      },
      '%d': function(date) {
        return leadingNulls(date.tm_mday, 2);
      },
      '%e': function(date) {
        return leadingSomething(date.tm_mday, 2, ' ');
      },
      '%g': function(date) {
        // %g, %G, and %V give values according to the ISO 8601:2000 standard week-based year. 
        // In this system, weeks begin on a Monday and week 1 of the year is the week that includes 
        // January 4th, which is also the week that includes the first Thursday of the year, and 
        // is also the first week that contains at least four days in the year. 
        // If the first Monday of January is the 2nd, 3rd, or 4th, the preceding days are part of 
        // the last week of the preceding year; thus, for Saturday 2nd January 1999, 
        // %G is replaced by 1998 and %V is replaced by 53. If December 29th, 30th, 
        // or 31st is a Monday, it and any following days are part of week 1 of the following year. 
        // Thus, for Tuesday 30th December 1997, %G is replaced by 1998 and %V is replaced by 01.
        
        return getWeekBasedYear(date).toString().substring(2);
      },
      '%G': function(date) {
        return getWeekBasedYear(date);
      },
      '%H': function(date) {
        return leadingNulls(date.tm_hour, 2);
      },
      '%I': function(date) {
        var twelveHour = date.tm_hour;
        if (twelveHour == 0) twelveHour = 12;
        else if (twelveHour > 12) twelveHour -= 12;
        return leadingNulls(twelveHour, 2);
      },
      '%j': function(date) {
        // Day of the year (001-366)
        return leadingNulls(date.tm_mday+__arraySum(__isLeapYear(date.tm_year+1900) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, date.tm_mon-1), 3);
      },
      '%m': function(date) {
        return leadingNulls(date.tm_mon+1, 2);
      },
      '%M': function(date) {
        return leadingNulls(date.tm_min, 2);
      },
      '%n': function() {
        return '\n';
      },
      '%p': function(date) {
        if (date.tm_hour >= 0 && date.tm_hour < 12) {
          return 'AM';
        } else {
          return 'PM';
        }
      },
      '%S': function(date) {
        return leadingNulls(date.tm_sec, 2);
      },
      '%t': function() {
        return '\t';
      },
      '%u': function(date) {
        var day = new Date(date.tm_year+1900, date.tm_mon+1, date.tm_mday, 0, 0, 0, 0);
        return day.getDay() || 7;
      },
      '%U': function(date) {
        // Replaced by the week number of the year as a decimal number [00,53]. 
        // The first Sunday of January is the first day of week 1; 
        // days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
        var janFirst = new Date(date.tm_year+1900, 0, 1);
        var firstSunday = janFirst.getDay() === 0 ? janFirst : __addDays(janFirst, 7-janFirst.getDay());
        var endDate = new Date(date.tm_year+1900, date.tm_mon, date.tm_mday);
        
        // is target date after the first Sunday?
        if (compareByDay(firstSunday, endDate) < 0) {
          // calculate difference in days between first Sunday and endDate
          var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth()-1)-31;
          var firstSundayUntilEndJanuary = 31-firstSunday.getDate();
          var days = firstSundayUntilEndJanuary+februaryFirstUntilEndMonth+endDate.getDate();
          return leadingNulls(Math.ceil(days/7), 2);
        }

        return compareByDay(firstSunday, janFirst) === 0 ? '01': '00';
      },
      '%V': function(date) {
        // Replaced by the week number of the year (Monday as the first day of the week) 
        // as a decimal number [01,53]. If the week containing 1 January has four 
        // or more days in the new year, then it is considered week 1. 
        // Otherwise, it is the last week of the previous year, and the next week is week 1. 
        // Both January 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
        var janFourthThisYear = new Date(date.tm_year+1900, 0, 4);
        var janFourthNextYear = new Date(date.tm_year+1901, 0, 4);

        var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
        var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);

        var endDate = __addDays(new Date(date.tm_year+1900, 0, 1), date.tm_yday);

        if (compareByDay(endDate, firstWeekStartThisYear) < 0) {
          // if given date is before this years first week, then it belongs to the 53rd week of last year
          return '53';
        } 

        if (compareByDay(firstWeekStartNextYear, endDate) <= 0) {
          // if given date is after next years first week, then it belongs to the 01th week of next year
          return '01';
        }

        // given date is in between CW 01..53 of this calendar year
        var daysDifference;
        if (firstWeekStartThisYear.getFullYear() < date.tm_year+1900) {
          // first CW of this year starts last year
          daysDifference = date.tm_yday+32-firstWeekStartThisYear.getDate()
        } else {
          // first CW of this year starts this year
          daysDifference = date.tm_yday+1-firstWeekStartThisYear.getDate();
        }
        return leadingNulls(Math.ceil(daysDifference/7), 2);
      },
      '%w': function(date) {
        var day = new Date(date.tm_year+1900, date.tm_mon+1, date.tm_mday, 0, 0, 0, 0);
        return day.getDay();
      },
      '%W': function(date) {
        // Replaced by the week number of the year as a decimal number [00,53]. 
        // The first Monday of January is the first day of week 1; 
        // days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
        var janFirst = new Date(date.tm_year, 0, 1);
        var firstMonday = janFirst.getDay() === 1 ? janFirst : __addDays(janFirst, janFirst.getDay() === 0 ? 1 : 7-janFirst.getDay()+1);
        var endDate = new Date(date.tm_year+1900, date.tm_mon, date.tm_mday);

        // is target date after the first Monday?
        if (compareByDay(firstMonday, endDate) < 0) {
          var februaryFirstUntilEndMonth = __arraySum(__isLeapYear(endDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, endDate.getMonth()-1)-31;
          var firstMondayUntilEndJanuary = 31-firstMonday.getDate();
          var days = firstMondayUntilEndJanuary+februaryFirstUntilEndMonth+endDate.getDate();
          return leadingNulls(Math.ceil(days/7), 2);
        }
        return compareByDay(firstMonday, janFirst) === 0 ? '01': '00';
      },
      '%y': function(date) {
        // Replaced by the last two digits of the year as a decimal number [00,99]. [ tm_year]
        return (date.tm_year+1900).toString().substring(2);
      },
      '%Y': function(date) {
        // Replaced by the year as a decimal number (for example, 1997). [ tm_year]
        return date.tm_year+1900;
      },
      '%z': function(date) {
        // Replaced by the offset from UTC in the ISO 8601:2000 standard format ( +hhmm or -hhmm ).
        // For example, "-0430" means 4 hours 30 minutes behind UTC (west of Greenwich).
        var off = date.tm_gmtoff;
        var ahead = off >= 0;
        off = Math.abs(off) / 60;
        // convert from minutes into hhmm format (which means 60 minutes = 100 units)
        off = (off / 60)*100 + (off % 60);
        return (ahead ? '+' : '-') + String("0000" + off).slice(-4);
      },
      '%Z': function(date) {
        return date.tm_zone;
      },
      '%%': function() {
        return '%';
      }
    };
    for (var rule in EXPANSION_RULES_2) {
      if (pattern.indexOf(rule) >= 0) {
        pattern = pattern.replace(new RegExp(rule, 'g'), EXPANSION_RULES_2[rule](date));
      }
    }

    var bytes = intArrayFromString(pattern, false);
    if (bytes.length > maxsize) {
      return 0;
    } 

    writeArrayToMemory(bytes, s);
    return bytes.length-1;
  },
  strftime_l__deps: ['strftime'],
  strftime_l: function(s, maxsize, format, tm) {
    return _strftime(s, maxsize, format, tm); // no locale support yet
  },

  strptime__deps: ['_isLeapYear', '_arraySum', '_addDays', '_MONTH_DAYS_REGULAR', '_MONTH_DAYS_LEAP'],
  strptime: function(buf, format, tm) {
    // char *strptime(const char *restrict buf, const char *restrict format, struct tm *restrict tm);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strptime.html
    var pattern = Pointer_stringify(format);

    // escape special characters
    // TODO: not sure we really need to escape all of these in JS regexps
    var SPECIAL_CHARS = '\\!@#$^&*()+=-[]/{}|:<>?,.';
    for (var i=0, ii=SPECIAL_CHARS.length; i<ii; ++i) {
      pattern = pattern.replace(new RegExp('\\'+SPECIAL_CHARS[i], 'g'), '\\'+SPECIAL_CHARS[i]);
    }

    // reduce number of matchers
    var EQUIVALENT_MATCHERS = {
      '%A':  '%a',
      '%B':  '%b',
      '%c':  '%x\\s+%X',
      '%D':  '%m\\/%d\\/%y',
      '%e':  '%d',
      '%h':  '%b',
      '%R':  '%H\\:%M',
      '%r':  '%I\\:%M\\:%S\\s%p',
      '%T':  '%H\\:%M\\:%S',
      '%x':  '%m\\/%d\\/(?:%y|%Y)',
      '%X':  '%H\\:%M\\:%S'
    };
    for (var matcher in EQUIVALENT_MATCHERS) {
      pattern = pattern.replace(matcher, EQUIVALENT_MATCHERS[matcher]);
    }
    
    // TODO: take care of locale

    var DATE_PATTERNS = {
      /* weeday name */     '%a': '(?:Sun(?:day)?)|(?:Mon(?:day)?)|(?:Tue(?:sday)?)|(?:Wed(?:nesday)?)|(?:Thu(?:rsday)?)|(?:Fri(?:day)?)|(?:Sat(?:urday)?)',
      /* month name */      '%b': '(?:Jan(?:uary)?)|(?:Feb(?:ruary)?)|(?:Mar(?:ch)?)|(?:Apr(?:il)?)|May|(?:Jun(?:e)?)|(?:Jul(?:y)?)|(?:Aug(?:ust)?)|(?:Sep(?:tember)?)|(?:Oct(?:ober)?)|(?:Nov(?:ember)?)|(?:Dec(?:ember)?)',
      /* century */         '%C': '\\d\\d',
      /* day of month */    '%d': '0[1-9]|[1-9](?!\\d)|1\\d|2\\d|30|31',
      /* hour (24hr) */     '%H': '\\d(?!\\d)|[0,1]\\d|20|21|22|23',
      /* hour (12hr) */     '%I': '\\d(?!\\d)|0\\d|10|11|12',
      /* day of year */     '%j': '00[1-9]|0?[1-9](?!\\d)|0?[1-9]\\d(?!\\d)|[1,2]\\d\\d|3[0-6]\\d',
      /* month */           '%m': '0[1-9]|[1-9](?!\\d)|10|11|12',
      /* minutes */         '%M': '0\\d|\\d(?!\\d)|[1-5]\\d',
      /* whitespace */      '%n': '\\s',
      /* AM/PM */           '%p': 'AM|am|PM|pm|A\\.M\\.|a\\.m\\.|P\\.M\\.|p\\.m\\.',
      /* seconds */         '%S': '0\\d|\\d(?!\\d)|[1-5]\\d|60',
      /* week number */     '%U': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* week number */     '%W': '0\\d|\\d(?!\\d)|[1-4]\\d|50|51|52|53',
      /* weekday number */  '%w': '[0-6]',
      /* 2-digit year */    '%y': '\\d\\d',
      /* 4-digit year */    '%Y': '\\d\\d\\d\\d',
      /* % */               '%%': '%',
      /* whitespace */      '%t': '\\s',
    };

    var MONTH_NUMBERS = {JAN: 0, FEB: 1, MAR: 2, APR: 3, MAY: 4, JUN: 5, JUL: 6, AUG: 7, SEP: 8, OCT: 9, NOV: 10, DEC: 11};
    var DAY_NUMBERS_SUN_FIRST = {SUN: 0, MON: 1, TUE: 2, WED: 3, THU: 4, FRI: 5, SAT: 6};
    var DAY_NUMBERS_MON_FIRST = {MON: 0, TUE: 1, WED: 2, THU: 3, FRI: 4, SAT: 5, SUN: 6};

    for (var datePattern in DATE_PATTERNS) {
      pattern = pattern.replace(datePattern, '('+datePattern+DATE_PATTERNS[datePattern]+')');    
    }

    // take care of capturing groups
    var capture = [];
    for (var i=pattern.indexOf('%'); i>=0; i=pattern.indexOf('%')) {
      capture.push(pattern[i+1]);
      pattern = pattern.replace(new RegExp('\\%'+pattern[i+1], 'g'), '');
    }

    var matches = new RegExp('^'+pattern, "i").exec(Pointer_stringify(buf))
    // Module['print'](Pointer_stringify(buf)+ ' is matched by '+((new RegExp('^'+pattern)).source)+' into: '+JSON.stringify(matches));

    function initDate() {
      function fixup(value, min, max) {
        return (typeof value !== 'number' || isNaN(value)) ? min : (value>=min ? (value<=max ? value: max): min);
      };
      return {
        year: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_year, 'i32', 0, 0, 1) }}} + 1900 , 1970, 9999),
        month: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mon, 'i32', 0, 0, 1) }}}, 0, 11),
        day: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_mday, 'i32', 0, 0, 1) }}}, 1, 31),
        hour: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_hour, 'i32', 0, 0, 1) }}}, 0, 23),
        min: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_min, 'i32', 0, 0, 1) }}}, 0, 59),
        sec: fixup({{{ makeGetValue('tm', C_STRUCTS.tm.tm_sec, 'i32', 0, 0, 1) }}}, 0, 59)
      };
    };

    if (matches) {
      var date = initDate();
      var value;

      function getMatch(symbol) {
        var pos = capture.indexOf(symbol);
        // check if symbol appears in regexp
        if (pos >= 0) {
          // return matched value or null (falsy!) for non-matches
          return matches[pos+1];
        }
        return;
      }

      // seconds
      if ((value=getMatch('S'))) {
        date.sec = parseInt(value);
      }

      // minutes
      if ((value=getMatch('M'))) {
        date.min = parseInt(value);
      }

      // hours
      if ((value=getMatch('H'))) {
        // 24h clock
        date.hour = parseInt(value);
      } else if ((value = getMatch('I'))) {
        // AM/PM clock
        var hour = parseInt(value);
        if ((value=getMatch('p'))) {
          hour += value.toUpperCase()[0] === 'P' ? 12 : 0;
        }
        date.hour = hour;
      }

      // year
      if ((value=getMatch('Y'))) {
        // parse from four-digit year
        date.year = parseInt(value);
      } else if ((value=getMatch('y'))) {
        // parse from two-digit year...
        var year = parseInt(value);
        if ((value=getMatch('C'))) {
          // ...and century
          year += parseInt(value)*100;
        } else {
          // ...and rule-of-thumb
          year += year<69 ? 2000 : 1900;
        }
        date.year = year;
      }

      // month
      if ((value=getMatch('m'))) {
        // parse from month number
        date.month = parseInt(value)-1;
      } else if ((value=getMatch('b'))) {
        // parse from month name
        date.month = MONTH_NUMBERS[value.substring(0,3).toUpperCase()] || 0;
        // TODO: derive month from day in year+year, week number+day of week+year 
      }

      // day
      if ((value=getMatch('d'))) {
        // get day of month directly
        date.day = parseInt(value);
      } else if ((value=getMatch('j'))) {
        // get day of month from day of year ...
        var day = parseInt(value);
        var leapYear = __isLeapYear(date.year);
        for (var month=0; month<12; ++month) {
          var daysUntilMonth = __arraySum(leapYear ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, month-1);
          if (day<=daysUntilMonth+(leapYear ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[month]) {
            date.day = day-daysUntilMonth;
          }
        }
      } else if ((value=getMatch('a'))) {
        // get day of month from weekday ...
        var weekDay = value.substring(0,3).toUpperCase();
        if ((value=getMatch('U'))) {
          // ... and week number (Sunday being first day of week)
          // Week number of the year (Sunday as the first day of the week) as a decimal number [00,53]. 
          // All days in a new year preceding the first Sunday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_SUN_FIRST[weekDay];
          var weekNumber = parseInt(value);

          // January 1st 
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay() === 0) {
            // Jan 1st is a Sunday, and, hence in the 1st CW
            endDate = __addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Sunday, and, hence still in the 0th CW
            endDate = __addDays(janFirst, 7-janFirst.getDay()+weekDayNumber+7*(weekNumber-1));
          }
          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        } else if ((value=getMatch('W'))) {
          // ... and week number (Monday being first day of week)
          // Week number of the year (Monday as the first day of the week) as a decimal number [00,53]. 
          // All days in a new year preceding the first Monday are considered to be in week 0.
          var weekDayNumber = DAY_NUMBERS_MON_FIRST[weekDay];
          var weekNumber = parseInt(value);

          // January 1st 
          var janFirst = new Date(date.year, 0, 1);
          var endDate;
          if (janFirst.getDay()===1) {
            // Jan 1st is a Monday, and, hence in the 1st CW
             endDate = __addDays(janFirst, weekDayNumber+7*(weekNumber-1));
          } else {
            // Jan 1st is not a Monday, and, hence still in the 0th CW
            endDate = __addDays(janFirst, 7-janFirst.getDay()+1+weekDayNumber+7*(weekNumber-1));
          }

          date.day = endDate.getDate();
          date.month = endDate.getMonth();
        }
      }

      /*
      tm_sec  int seconds after the minute  0-61*
      tm_min  int minutes after the hour  0-59
      tm_hour int hours since midnight  0-23
      tm_mday int day of the month  1-31
      tm_mon  int months since January  0-11
      tm_year int years since 1900  
      tm_wday int days since Sunday 0-6
      tm_yday int days since January 1  0-365
      tm_isdst  int Daylight Saving Time flag 
      */

      var fullDate = new Date(date.year, date.month, date.day, date.hour, date.min, date.sec, 0);
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_sec, 'fullDate.getSeconds()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_min, 'fullDate.getMinutes()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_hour, 'fullDate.getHours()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mday, 'fullDate.getDate()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_mon, 'fullDate.getMonth()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_year, 'fullDate.getFullYear()-1900', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_wday, 'fullDate.getDay()', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_yday, '__arraySum(__isLeapYear(fullDate.getFullYear()) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, fullDate.getMonth()-1)+fullDate.getDate()-1', 'i32') }}};
      {{{ makeSetValue('tm', C_STRUCTS.tm.tm_isdst, '0', 'i32') }}};

      // we need to convert the matched sequence into an integer array to take care of UTF-8 characters > 0x7F
      // TODO: not sure that intArrayFromString handles all unicode characters correctly
      return buf+intArrayFromString(matches[0]).length-1;
    } 

    return 0;
  },
  strptime_l__deps: ['strptime'],
  strptime_l: function(buf, format, tm) {
    return _strptime(buf, format, tm); // no locale support yet
  },

  getdate: function(string) {
    // struct tm *getdate(const char *string);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/getdate.html
    // TODO: Implement.
    return 0;
  },

  // ==========================================================================
  // sys/time.h
  // ==========================================================================

  clock_gettime__deps: ['emscripten_get_now', 'emscripten_get_now_is_monotonic', '$ERRNO_CODES', '__setErrNo'],
  clock_gettime: function(clk_id, tp) {
    // int clock_gettime(clockid_t clk_id, struct timespec *tp);
    var now;
    if (clk_id === {{{ cDefine('CLOCK_REALTIME') }}}) {
      now = Date.now();
    } else if (clk_id === {{{ cDefine('CLOCK_MONOTONIC') }}} && _emscripten_get_now_is_monotonic()) {
      now = _emscripten_get_now();
    } else {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    {{{ makeSetValue('tp', C_STRUCTS.timespec.tv_sec, '(now/1000)|0', 'i32') }}}; // seconds
    {{{ makeSetValue('tp', C_STRUCTS.timespec.tv_nsec, '((now % 1000)*1000*1000)|0', 'i32') }}}; // nanoseconds
    return 0;
  },
  __clock_gettime: 'clock_gettime', // musl internal alias
  clock_settime__deps: ['$ERRNO_CODES', '__setErrNo'],
  clock_settime: function(clk_id, tp) {
    // int clock_settime(clockid_t clk_id, const struct timespec *tp);
    // Nothing.
    ___setErrNo(clk_id === {{{ cDefine('CLOCK_REALTIME') }}} ? ERRNO_CODES.EPERM
                                                             : ERRNO_CODES.EINVAL);
    return -1;
  },
  clock_getres__deps: ['emscripten_get_now_res', 'emscripten_get_now_is_monotonic', '$ERRNO_CODES', '__setErrNo'],
  clock_getres: function(clk_id, res) {
    // int clock_getres(clockid_t clk_id, struct timespec *res);
    var nsec;
    if (clk_id === {{{ cDefine('CLOCK_REALTIME') }}}) {
      nsec = 1000 * 1000; // educated guess that it's milliseconds
    } else if (clk_id === {{{ cDefine('CLOCK_MONOTONIC') }}} && _emscripten_get_now_is_monotonic()) {
      nsec = _emscripten_get_now_res();
    } else {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    {{{ makeSetValue('res', C_STRUCTS.timespec.tv_sec, '(nsec/1000000000)|0', 'i32') }}};
    {{{ makeSetValue('res', C_STRUCTS.timespec.tv_nsec, 'nsec', 'i32') }}} // resolution is nanoseconds
    return 0;
  },
  clock_getcpuclockid__deps: ['$PROCINFO'],
  clock_getcpuclockid: function(pid, clk_id) {
    if (pid < 0) return ERRNO_CODES.ESRCH;
    if (pid !== 0 && pid !== PROCINFO.pid) return ERRNO_CODES.ENOSYS;
    if (clk_id) {{{ makeSetValue('clk_id', 0, 2/*CLOCK_PROCESS_CPUTIME_ID*/, 'i32') }}};
    return 0;
  },
  // http://pubs.opengroup.org/onlinepubs/000095399/basedefs/sys/time.h.html
  gettimeofday: function(ptr) {
    var now = Date.now();
    {{{ makeSetValue('ptr', C_STRUCTS.timeval.tv_sec, '(now/1000)|0', 'i32') }}}; // seconds
    {{{ makeSetValue('ptr', C_STRUCTS.timeval.tv_usec, '((now % 1000)*1000)|0', 'i32') }}}; // microseconds
    return 0;
  },

  // ==========================================================================
  // sys/timeb.h
  // ==========================================================================
  
  ftime: function(p) {
    var millis = Date.now();
    {{{ makeSetValue('p', C_STRUCTS.timeb.time, '(millis/1000)|0', 'i32') }}};
    {{{ makeSetValue('p', C_STRUCTS.timeb.millitm, 'millis % 1000', 'i16') }}};
    {{{ makeSetValue('p', C_STRUCTS.timeb.timezone, '0', 'i16') }}}; // Obsolete field
    {{{ makeSetValue('p', C_STRUCTS.timeb.dstflag, '0', 'i16') }}}; // Obsolete field
    return 0;
  },

  // ==========================================================================
  // sys/times.h
  // ==========================================================================

  times__deps: ['memset'],
  times: function(buffer) {
    // clock_t times(struct tms *buffer);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/times.html
    // NOTE: This is fake, since we can't calculate real CPU time usage in JS.
    if (buffer !== 0) {
      _memset(buffer, 0, {{{ C_STRUCTS.tms.__size__ }}});
    }
    return 0;
  },

  // ==========================================================================
  // sys/types.h
  // ==========================================================================
  // http://www.kernel.org/doc/man-pages/online/pages/man3/minor.3.html
  makedev: function(maj, min) {
    return ((maj) << 8 | (min));
  },
  gnu_dev_makedev: 'makedev',
  major: function(dev) {
    return ((dev) >> 8);
  },
  gnu_dev_major: 'major',
  minor: function(dev) {
    return ((dev) & 0xff);
  },
  gnu_dev_minor: 'minor',

  // ==========================================================================
  // setjmp.h
  // ==========================================================================

  // asm.js-style setjmp/longjmp support for wasm binaryen backend.
  // In asm.js compilation, various variables including setjmpId will be
  // generated within 'var asm' in emscripten.py, while in wasm compilation,
  // wasm side is considered as 'asm' so they are not generated. But
  // saveSetjmp() needs setjmpId and no other functions in wasm side needs it.
  // So we declare it here if WASM_BACKEND=1.
#if WASM_BACKEND == 1
  $setjmpId: 0,
#endif

  saveSetjmp__asm: true,
  saveSetjmp__sig: 'iii',
#if WASM_BACKEND == 1
  saveSetjmp__deps: ['realloc', '$setjmpId'],
#else
  saveSetjmp__deps: ['realloc'],
#endif
  saveSetjmp: function(env, label, table, size) {
    // Not particularly fast: slow table lookup of setjmpId to label. But setjmp
    // prevents relooping anyhow, so slowness is to be expected. And typical case
    // is 1 setjmp per invocation, or less.
    env = env|0;
    label = label|0;
    table = table|0;
    size = size|0;
    var i = 0;
    setjmpId = (setjmpId+1)|0;
    {{{ makeSetValueAsm('env', '0', 'setjmpId', 'i32') }}};
    while ((i|0) < (size|0)) {
      if ({{{ makeGetValueAsm('table', '(i<<3)', 'i32') }}} == 0) {
        {{{ makeSetValueAsm('table', '(i<<3)', 'setjmpId', 'i32') }}};
        {{{ makeSetValueAsm('table', '(i<<3)+4', 'label', 'i32') }}};
        // prepare next slot
        {{{ makeSetValueAsm('table', '(i<<3)+8', '0', 'i32') }}};
        {{{ makeSetTempRet0('size') }}};
        return table | 0;
      }
      i = i+1|0;
    }
    // grow the table
    size = (size*2)|0;
    table = _realloc(table|0, 8*(size+1|0)|0) | 0;
    table = _saveSetjmp(env|0, label|0, table|0, size|0) | 0;
    {{{ makeSetTempRet0('size') }}};
    return table | 0;
  },

  testSetjmp__asm: true,
  testSetjmp__sig: 'iii',
  testSetjmp: function(id, table, size) {
    id = id|0;
    table = table|0;
    size = size|0;
    var i = 0, curr = 0;
    while ((i|0) < (size|0)) {
      curr = {{{ makeGetValueAsm('table', '(i<<3)', 'i32') }}};
      if ((curr|0) == 0) break;
      if ((curr|0) == (id|0)) {
        return {{{ makeGetValueAsm('table', '(i<<3)+4', 'i32') }}};
      }
      i = i+1|0;
    }
    return 0;
  },

  setjmp__deps: ['saveSetjmp', 'testSetjmp'],
  setjmp__inline: function(env) {
    // Save the label
    return '_saveSetjmp(' + env + ', label, setjmpTable)|0';
  },

  longjmp__deps: ['saveSetjmp', 'testSetjmp'],
  longjmp: function(env, value) {
    Module['setThrew'](env, value || 1);
    throw 'longjmp';
  },
  emscripten_longjmp__deps: ['longjmp'],
  emscripten_longjmp: function(env, value) {
    _longjmp(env, value);
  },

  // ==========================================================================
  // sys/wait.h
  // ==========================================================================

  wait__deps: ['$ERRNO_CODES', '__setErrNo'],
  wait: function(stat_loc) {
    // pid_t wait(int *stat_loc);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/wait.html
    // Makes no sense in a single-process environment.
    ___setErrNo(ERRNO_CODES.ECHILD);
    return -1;
  },
  // NOTE: These aren't really the same, but we use the same stub for them all.
  waitid: 'wait',
  waitpid: 'wait',
  wait3: 'wait',
  wait4: 'wait',

  // ==========================================================================
  // errno.h
  // ==========================================================================

  $ERRNO_CODES: {
    EPERM: {{{ cDefine('EPERM') }}},
    ENOENT: {{{ cDefine('ENOENT') }}},
    ESRCH: {{{ cDefine('ESRCH') }}},
    EINTR: {{{ cDefine('EINTR') }}},
    EIO: {{{ cDefine('EIO') }}},
    ENXIO: {{{ cDefine('ENXIO') }}},
    E2BIG: {{{ cDefine('E2BIG') }}},
    ENOEXEC: {{{ cDefine('ENOEXEC') }}},
    EBADF: {{{ cDefine('EBADF') }}},
    ECHILD: {{{ cDefine('ECHILD') }}},
    EAGAIN: {{{ cDefine('EAGAIN') }}},
    EWOULDBLOCK: {{{ cDefine('EWOULDBLOCK') }}},
    ENOMEM: {{{ cDefine('ENOMEM') }}},
    EACCES: {{{ cDefine('EACCES') }}},
    EFAULT: {{{ cDefine('EFAULT') }}},
    ENOTBLK: {{{ cDefine('ENOTBLK') }}},
    EBUSY: {{{ cDefine('EBUSY') }}},
    EEXIST: {{{ cDefine('EEXIST') }}},
    EXDEV: {{{ cDefine('EXDEV') }}},
    ENODEV: {{{ cDefine('ENODEV') }}},
    ENOTDIR: {{{ cDefine('ENOTDIR') }}},
    EISDIR: {{{ cDefine('EISDIR') }}},
    EINVAL: {{{ cDefine('EINVAL') }}},
    ENFILE: {{{ cDefine('ENFILE') }}},
    EMFILE: {{{ cDefine('EMFILE') }}},
    ENOTTY: {{{ cDefine('ENOTTY') }}},
    ETXTBSY: {{{ cDefine('ETXTBSY') }}},
    EFBIG: {{{ cDefine('EFBIG') }}},
    ENOSPC: {{{ cDefine('ENOSPC') }}},
    ESPIPE: {{{ cDefine('ESPIPE') }}},
    EROFS: {{{ cDefine('EROFS') }}},
    EMLINK: {{{ cDefine('EMLINK') }}},
    EPIPE: {{{ cDefine('EPIPE') }}},
    EDOM: {{{ cDefine('EDOM') }}},
    ERANGE: {{{ cDefine('ERANGE') }}},
    ENOMSG: {{{ cDefine('ENOMSG') }}},
    EIDRM: {{{ cDefine('EIDRM') }}},
    ECHRNG: {{{ cDefine('ECHRNG') }}},
    EL2NSYNC: {{{ cDefine('EL2NSYNC') }}},
    EL3HLT: {{{ cDefine('EL3HLT') }}},
    EL3RST: {{{ cDefine('EL3RST') }}},
    ELNRNG: {{{ cDefine('ELNRNG') }}},
    EUNATCH: {{{ cDefine('EUNATCH') }}},
    ENOCSI: {{{ cDefine('ENOCSI') }}},
    EL2HLT: {{{ cDefine('EL2HLT') }}},
    EDEADLK: {{{ cDefine('EDEADLK') }}},
    ENOLCK: {{{ cDefine('ENOLCK') }}},
    EBADE: {{{ cDefine('EBADE') }}},
    EBADR: {{{ cDefine('EBADR') }}},
    EXFULL: {{{ cDefine('EXFULL') }}},
    ENOANO: {{{ cDefine('ENOANO') }}},
    EBADRQC: {{{ cDefine('EBADRQC') }}},
    EBADSLT: {{{ cDefine('EBADSLT') }}},
    EDEADLOCK: {{{ cDefine('EDEADLOCK') }}},
    EBFONT: {{{ cDefine('EBFONT') }}},
    ENOSTR: {{{ cDefine('ENOSTR') }}},
    ENODATA: {{{ cDefine('ENODATA') }}},
    ETIME: {{{ cDefine('ETIME') }}},
    ENOSR: {{{ cDefine('ENOSR') }}},
    ENONET: {{{ cDefine('ENONET') }}},
    ENOPKG: {{{ cDefine('ENOPKG') }}},
    EREMOTE: {{{ cDefine('EREMOTE') }}},
    ENOLINK: {{{ cDefine('ENOLINK') }}},
    EADV: {{{ cDefine('EADV') }}},
    ESRMNT: {{{ cDefine('ESRMNT') }}},
    ECOMM: {{{ cDefine('ECOMM') }}},
    EPROTO: {{{ cDefine('EPROTO') }}},
    EMULTIHOP: {{{ cDefine('EMULTIHOP') }}},
    EDOTDOT: {{{ cDefine('EDOTDOT') }}},
    EBADMSG: {{{ cDefine('EBADMSG') }}},
    ENOTUNIQ: {{{ cDefine('ENOTUNIQ') }}},
    EBADFD: {{{ cDefine('EBADFD') }}},
    EREMCHG: {{{ cDefine('EREMCHG') }}},
    ELIBACC: {{{ cDefine('ELIBACC') }}},
    ELIBBAD: {{{ cDefine('ELIBBAD') }}},
    ELIBSCN: {{{ cDefine('ELIBSCN') }}},
    ELIBMAX: {{{ cDefine('ELIBMAX') }}},
    ELIBEXEC: {{{ cDefine('ELIBEXEC') }}},
    ENOSYS: {{{ cDefine('ENOSYS') }}},
    ENOTEMPTY: {{{ cDefine('ENOTEMPTY') }}},
    ENAMETOOLONG: {{{ cDefine('ENAMETOOLONG') }}},
    ELOOP: {{{ cDefine('ELOOP') }}},
    EOPNOTSUPP: {{{ cDefine('EOPNOTSUPP') }}},
    EPFNOSUPPORT: {{{ cDefine('EPFNOSUPPORT') }}},
    ECONNRESET: {{{ cDefine('ECONNRESET') }}},
    ENOBUFS: {{{ cDefine('ENOBUFS') }}},
    EAFNOSUPPORT: {{{ cDefine('EAFNOSUPPORT') }}},
    EPROTOTYPE: {{{ cDefine('EPROTOTYPE') }}},
    ENOTSOCK: {{{ cDefine('ENOTSOCK') }}},
    ENOPROTOOPT: {{{ cDefine('ENOPROTOOPT') }}},
    ESHUTDOWN: {{{ cDefine('ESHUTDOWN') }}},
    ECONNREFUSED: {{{ cDefine('ECONNREFUSED') }}},
    EADDRINUSE: {{{ cDefine('EADDRINUSE') }}},
    ECONNABORTED: {{{ cDefine('ECONNABORTED') }}},
    ENETUNREACH: {{{ cDefine('ENETUNREACH') }}},
    ENETDOWN: {{{ cDefine('ENETDOWN') }}},
    ETIMEDOUT: {{{ cDefine('ETIMEDOUT') }}},
    EHOSTDOWN: {{{ cDefine('EHOSTDOWN') }}},
    EHOSTUNREACH: {{{ cDefine('EHOSTUNREACH') }}},
    EINPROGRESS: {{{ cDefine('EINPROGRESS') }}},
    EALREADY: {{{ cDefine('EALREADY') }}},
    EDESTADDRREQ: {{{ cDefine('EDESTADDRREQ') }}},
    EMSGSIZE: {{{ cDefine('EMSGSIZE') }}},
    EPROTONOSUPPORT: {{{ cDefine('EPROTONOSUPPORT') }}},
    ESOCKTNOSUPPORT: {{{ cDefine('ESOCKTNOSUPPORT') }}},
    EADDRNOTAVAIL: {{{ cDefine('EADDRNOTAVAIL') }}},
    ENETRESET: {{{ cDefine('ENETRESET') }}},
    EISCONN: {{{ cDefine('EISCONN') }}},
    ENOTCONN: {{{ cDefine('ENOTCONN') }}},
    ETOOMANYREFS: {{{ cDefine('ETOOMANYREFS') }}},
    EUSERS: {{{ cDefine('EUSERS') }}},
    EDQUOT: {{{ cDefine('EDQUOT') }}},
    ESTALE: {{{ cDefine('ESTALE') }}},
    ENOTSUP: {{{ cDefine('ENOTSUP') }}},
    ENOMEDIUM: {{{ cDefine('ENOMEDIUM') }}},
    EILSEQ: {{{ cDefine('EILSEQ') }}},
    EOVERFLOW: {{{ cDefine('EOVERFLOW') }}},
    ECANCELED: {{{ cDefine('ECANCELED') }}},
    ENOTRECOVERABLE: {{{ cDefine('ENOTRECOVERABLE') }}},
    EOWNERDEAD: {{{ cDefine('EOWNERDEAD') }}},
    ESTRPIPE: {{{ cDefine('ESTRPIPE') }}},
  },
  $ERRNO_MESSAGES: {
    0: 'Success',
    {{{ cDefine('EPERM') }}}: 'Not super-user',
    {{{ cDefine('ENOENT') }}}: 'No such file or directory',
    {{{ cDefine('ESRCH') }}}: 'No such process',
    {{{ cDefine('EINTR') }}}: 'Interrupted system call',
    {{{ cDefine('EIO') }}}: 'I/O error',
    {{{ cDefine('ENXIO') }}}: 'No such device or address',
    {{{ cDefine('E2BIG') }}}: 'Arg list too long',
    {{{ cDefine('ENOEXEC') }}}: 'Exec format error',
    {{{ cDefine('EBADF') }}}: 'Bad file number',
    {{{ cDefine('ECHILD') }}}: 'No children',
    {{{ cDefine('EWOULDBLOCK') }}}: 'No more processes',
    {{{ cDefine('ENOMEM') }}}: 'Not enough core',
    {{{ cDefine('EACCES') }}}: 'Permission denied',
    {{{ cDefine('EFAULT') }}}: 'Bad address',
    {{{ cDefine('ENOTBLK') }}}: 'Block device required',
    {{{ cDefine('EBUSY') }}}: 'Mount device busy',
    {{{ cDefine('EEXIST') }}}: 'File exists',
    {{{ cDefine('EXDEV') }}}: 'Cross-device link',
    {{{ cDefine('ENODEV') }}}: 'No such device',
    {{{ cDefine('ENOTDIR') }}}: 'Not a directory',
    {{{ cDefine('EISDIR') }}}: 'Is a directory',
    {{{ cDefine('EINVAL') }}}: 'Invalid argument',
    {{{ cDefine('ENFILE') }}}: 'Too many open files in system',
    {{{ cDefine('EMFILE') }}}: 'Too many open files',
    {{{ cDefine('ENOTTY') }}}: 'Not a typewriter',
    {{{ cDefine('ETXTBSY') }}}: 'Text file busy',
    {{{ cDefine('EFBIG') }}}: 'File too large',
    {{{ cDefine('ENOSPC') }}}: 'No space left on device',
    {{{ cDefine('ESPIPE') }}}: 'Illegal seek',
    {{{ cDefine('EROFS') }}}: 'Read only file system',
    {{{ cDefine('EMLINK') }}}: 'Too many links',
    {{{ cDefine('EPIPE') }}}: 'Broken pipe',
    {{{ cDefine('EDOM') }}}: 'Math arg out of domain of func',
    {{{ cDefine('ERANGE') }}}: 'Math result not representable',
    {{{ cDefine('ENOMSG') }}}: 'No message of desired type',
    {{{ cDefine('EIDRM') }}}: 'Identifier removed',
    {{{ cDefine('ECHRNG') }}}: 'Channel number out of range',
    {{{ cDefine('EL2NSYNC') }}}: 'Level 2 not synchronized',
    {{{ cDefine('EL3HLT') }}}: 'Level 3 halted',
    {{{ cDefine('EL3RST') }}}: 'Level 3 reset',
    {{{ cDefine('ELNRNG') }}}: 'Link number out of range',
    {{{ cDefine('EUNATCH') }}}: 'Protocol driver not attached',
    {{{ cDefine('ENOCSI') }}}: 'No CSI structure available',
    {{{ cDefine('EL2HLT') }}}: 'Level 2 halted',
    {{{ cDefine('EDEADLK') }}}: 'Deadlock condition',
    {{{ cDefine('ENOLCK') }}}: 'No record locks available',
    {{{ cDefine('EBADE') }}}: 'Invalid exchange',
    {{{ cDefine('EBADR') }}}: 'Invalid request descriptor',
    {{{ cDefine('EXFULL') }}}: 'Exchange full',
    {{{ cDefine('ENOANO') }}}: 'No anode',
    {{{ cDefine('EBADRQC') }}}: 'Invalid request code',
    {{{ cDefine('EBADSLT') }}}: 'Invalid slot',
    {{{ cDefine('EDEADLOCK') }}}: 'File locking deadlock error',
    {{{ cDefine('EBFONT') }}}: 'Bad font file fmt',
    {{{ cDefine('ENOSTR') }}}: 'Device not a stream',
    {{{ cDefine('ENODATA') }}}: 'No data (for no delay io)',
    {{{ cDefine('ETIME') }}}: 'Timer expired',
    {{{ cDefine('ENOSR') }}}: 'Out of streams resources',
    {{{ cDefine('ENONET') }}}: 'Machine is not on the network',
    {{{ cDefine('ENOPKG') }}}: 'Package not installed',
    {{{ cDefine('EREMOTE') }}}: 'The object is remote',
    {{{ cDefine('ENOLINK') }}}: 'The link has been severed',
    {{{ cDefine('EADV') }}}: 'Advertise error',
    {{{ cDefine('ESRMNT') }}}: 'Srmount error',
    {{{ cDefine('ECOMM') }}}: 'Communication error on send',
    {{{ cDefine('EPROTO') }}}: 'Protocol error',
    {{{ cDefine('EMULTIHOP') }}}: 'Multihop attempted',
    {{{ cDefine('EDOTDOT') }}}: 'Cross mount point (not really error)',
    {{{ cDefine('EBADMSG') }}}: 'Trying to read unreadable message',
    {{{ cDefine('ENOTUNIQ') }}}: 'Given log. name not unique',
    {{{ cDefine('EBADFD') }}}: 'f.d. invalid for this operation',
    {{{ cDefine('EREMCHG') }}}: 'Remote address changed',
    {{{ cDefine('ELIBACC') }}}: 'Can   access a needed shared lib',
    {{{ cDefine('ELIBBAD') }}}: 'Accessing a corrupted shared lib',
    {{{ cDefine('ELIBSCN') }}}: '.lib section in a.out corrupted',
    {{{ cDefine('ELIBMAX') }}}: 'Attempting to link in too many libs',
    {{{ cDefine('ELIBEXEC') }}}: 'Attempting to exec a shared library',
    {{{ cDefine('ENOSYS') }}}: 'Function not implemented',
    {{{ cDefine('ENOTEMPTY') }}}: 'Directory not empty',
    {{{ cDefine('ENAMETOOLONG') }}}: 'File or path name too long',
    {{{ cDefine('ELOOP') }}}: 'Too many symbolic links',
    {{{ cDefine('EOPNOTSUPP') }}}: 'Operation not supported on transport endpoint',
    {{{ cDefine('EPFNOSUPPORT') }}}: 'Protocol family not supported',
    {{{ cDefine('ECONNRESET') }}}: 'Connection reset by peer',
    {{{ cDefine('ENOBUFS') }}}: 'No buffer space available',
    {{{ cDefine('EAFNOSUPPORT') }}}: 'Address family not supported by protocol family',
    {{{ cDefine('EPROTOTYPE') }}}: 'Protocol wrong type for socket',
    {{{ cDefine('ENOTSOCK') }}}: 'Socket operation on non-socket',
    {{{ cDefine('ENOPROTOOPT') }}}: 'Protocol not available',
    {{{ cDefine('ESHUTDOWN') }}}: 'Can\'t send after socket shutdown',
    {{{ cDefine('ECONNREFUSED') }}}: 'Connection refused',
    {{{ cDefine('EADDRINUSE') }}}: 'Address already in use',
    {{{ cDefine('ECONNABORTED') }}}: 'Connection aborted',
    {{{ cDefine('ENETUNREACH') }}}: 'Network is unreachable',
    {{{ cDefine('ENETDOWN') }}}: 'Network interface is not configured',
    {{{ cDefine('ETIMEDOUT') }}}: 'Connection timed out',
    {{{ cDefine('EHOSTDOWN') }}}: 'Host is down',
    {{{ cDefine('EHOSTUNREACH') }}}: 'Host is unreachable',
    {{{ cDefine('EINPROGRESS') }}}: 'Connection already in progress',
    {{{ cDefine('EALREADY') }}}: 'Socket already connected',
    {{{ cDefine('EDESTADDRREQ') }}}: 'Destination address required',
    {{{ cDefine('EMSGSIZE') }}}: 'Message too long',
    {{{ cDefine('EPROTONOSUPPORT') }}}: 'Unknown protocol',
    {{{ cDefine('ESOCKTNOSUPPORT') }}}: 'Socket type not supported',
    {{{ cDefine('EADDRNOTAVAIL') }}}: 'Address not available',
    {{{ cDefine('ENETRESET') }}}: 'Connection reset by network',
    {{{ cDefine('EISCONN') }}}: 'Socket is already connected',
    {{{ cDefine('ENOTCONN') }}}: 'Socket is not connected',
    {{{ cDefine('ETOOMANYREFS') }}}: 'Too many references',
    {{{ cDefine('EUSERS') }}}: 'Too many users',
    {{{ cDefine('EDQUOT') }}}: 'Quota exceeded',
    {{{ cDefine('ESTALE') }}}: 'Stale file handle',
    {{{ cDefine('ENOTSUP') }}}: 'Not supported',
    {{{ cDefine('ENOMEDIUM') }}}: 'No medium (in tape drive)',
    {{{ cDefine('EILSEQ') }}}: 'Illegal byte sequence',
    {{{ cDefine('EOVERFLOW') }}}: 'Value too large for defined data type',
    {{{ cDefine('ECANCELED') }}}: 'Operation canceled',
    {{{ cDefine('ENOTRECOVERABLE') }}}: 'State not recoverable',
    {{{ cDefine('EOWNERDEAD') }}}: 'Previous owner died',
    {{{ cDefine('ESTRPIPE') }}}: 'Streams pipe error',
  },
  __setErrNo: function(value) {
    if (Module['___errno_location']) {{{ makeSetValue("Module['___errno_location']()", 0, 'value', 'i32') }}};
#if ASSERTIONS
    else Module.printErr('failed to set errno from JS');
#endif
    return value;
  },

  // ==========================================================================
  // sched.h (stubs only - no thread support yet!)
  // ==========================================================================
  sched_yield: function() {
    return 0;
  },

  // ==========================================================================
  // arpa/inet.h
  // ==========================================================================

  // old ipv4 only functions
  inet_addr__deps: ['_inet_pton4_raw'],
  inet_addr: function(ptr) {
    var addr = __inet_pton4_raw(Pointer_stringify(ptr));
    if (addr === null) {
      return -1;
    }
    return addr;
  },

  // ==========================================================================
  // netinet/in.h
  // ==========================================================================

#if USE_PTHREADS
  in6addr_any: '; if (ENVIRONMENT_IS_PTHREAD) _in6addr_any = PthreadWorkerInit._in6addr_any; else PthreadWorkerInit._in6addr_any = _in6addr_any = allocate([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], "i8", ALLOC_STATIC)',
  in6addr_loopback: '; if (ENVIRONMENT_IS_PTHREAD) _in6addr_loopback = PthreadWorkerInit._in6addr_loopback; else PthreadWorkerInit._in6addr_loopback = _in6addr_loopback = allocate([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1], "i8", ALLOC_STATIC)',
#else
  in6addr_any:
    'allocate([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], "i8", ALLOC_STATIC)',
  in6addr_loopback:
    'allocate([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1], "i8", ALLOC_STATIC)',
#endif

  // ==========================================================================
  // netdb.h
  // ==========================================================================

  _inet_pton4_raw: function(str) {
    var b = str.split('.');
    for (var i = 0; i < 4; i++) {
      var tmp = Number(b[i]);
      if (isNaN(tmp)) return null;
      b[i] = tmp;
    }
    return (b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)) >>> 0;
  },
  _inet_ntop4_raw: function(addr) {
    return (addr & 0xff) + '.' + ((addr >> 8) & 0xff) + '.' + ((addr >> 16) & 0xff) + '.' + ((addr >> 24) & 0xff)
  },
  _inet_pton6_raw__deps: ['htons'],
  _inet_pton6_raw: function(str) {
    var words;
    var w, offset, z, i;
    /* http://home.deds.nl/~aeron/regex/ */
    var valid6regx = /^((?=.*::)(?!.*::.+::)(::)?([\dA-F]{1,4}:(:|\b)|){5}|([\dA-F]{1,4}:){6})((([\dA-F]{1,4}((?!\3)::|:\b|$))|(?!\2\3)){2}|(((2[0-4]|1\d|[1-9])?\d|25[0-5])\.?\b){4})$/i
    var parts = [];
    if (!valid6regx.test(str)) {
      return null;
    }
    if (str === "::") {
      return [0, 0, 0, 0, 0, 0, 0, 0];
    }
    // Z placeholder to keep track of zeros when splitting the string on ":"
    if (str.indexOf("::") === 0) {
      str = str.replace("::", "Z:"); // leading zeros case
    } else {
      str = str.replace("::", ":Z:");
    }

    if (str.indexOf(".") > 0) {
      // parse IPv4 embedded stress
      str = str.replace(new RegExp('[.]', 'g'), ":");
      words = str.split(":");
      words[words.length-4] = parseInt(words[words.length-4]) + parseInt(words[words.length-3])*256;
      words[words.length-3] = parseInt(words[words.length-2]) + parseInt(words[words.length-1])*256;
      words = words.slice(0, words.length-2);
    } else {
      words = str.split(":");
    }

    offset = 0; z = 0;
    for (w=0; w < words.length; w++) {
      if (typeof words[w] === 'string') {
        if (words[w] === 'Z') {
          // compressed zeros - write appropriate number of zero words
          for (z = 0; z < (8 - words.length+1); z++) {
            parts[w+z] = 0;
          }
          offset = z-1;
        } else {
          // parse hex to field to 16-bit value and write it in network byte-order
          parts[w+offset] = _htons(parseInt(words[w],16));
        }
      } else {
        // parsed IPv4 words
        parts[w+offset] = words[w];
      }
    }
    return [
      (parts[1] << 16) | parts[0],
      (parts[3] << 16) | parts[2],
      (parts[5] << 16) | parts[4],
      (parts[7] << 16) | parts[6]
    ];
  },
  _inet_pton6__deps: ['_inet_pton6_raw'],
  _inet_pton6: function(src, dst) {
    var ints = __inet_pton6_raw(Pointer_stringify(src));
    if (ints === null) {
      return 0;
    }
    for (var i = 0; i < 4; i++) {
      {{{ makeSetValue('dst', 'i*4', 'ints[i]', 'i32') }}};
    }
    return 1;
  },
  _inet_ntop6_raw__deps: ['_inet_ntop4_raw'],
  _inet_ntop6_raw: function(ints) {
    //  ref:  http://www.ietf.org/rfc/rfc2373.txt - section 2.5.4
    //  Format for IPv4 compatible and mapped  128-bit IPv6 Addresses
    //  128-bits are split into eight 16-bit words
    //  stored in network byte order (big-endian)
    //  |                80 bits               | 16 |      32 bits        |
    //  +-----------------------------------------------------------------+
    //  |               10 bytes               |  2 |      4 bytes        |
    //  +--------------------------------------+--------------------------+
    //  +               5 words                |  1 |      2 words        |
    //  +--------------------------------------+--------------------------+
    //  |0000..............................0000|0000|    IPv4 ADDRESS     | (compatible)
    //  +--------------------------------------+----+---------------------+
    //  |0000..............................0000|FFFF|    IPv4 ADDRESS     | (mapped)
    //  +--------------------------------------+----+---------------------+
    var str = "";
    var word = 0;
    var longest = 0;
    var lastzero = 0;
    var zstart = 0;
    var len = 0;
    var i = 0;
    var parts = [
      ints[0] & 0xffff,
      (ints[0] >> 16),
      ints[1] & 0xffff,
      (ints[1] >> 16),
      ints[2] & 0xffff,
      (ints[2] >> 16),
      ints[3] & 0xffff,
      (ints[3] >> 16)
    ];

    // Handle IPv4-compatible, IPv4-mapped, loopback and any/unspecified addresses

    var hasipv4 = true;
    var v4part = "";
    // check if the 10 high-order bytes are all zeros (first 5 words)
    for (i = 0; i < 5; i++) {
      if (parts[i] !== 0) { hasipv4 = false; break; }
    }

    if (hasipv4) {
      // low-order 32-bits store an IPv4 address (bytes 13 to 16) (last 2 words)
      v4part = __inet_ntop4_raw(parts[6] | (parts[7] << 16));
      // IPv4-mapped IPv6 address if 16-bit value (bytes 11 and 12) == 0xFFFF (6th word)
      if (parts[5] === -1) {
        str = "::ffff:";
        str += v4part;
        return str;
      }
      // IPv4-compatible IPv6 address if 16-bit value (bytes 11 and 12) == 0x0000 (6th word)
      if (parts[5] === 0) {
        str = "::";
        //special case IPv6 addresses
        if(v4part === "0.0.0.0") v4part = ""; // any/unspecified address
        if(v4part === "0.0.0.1") v4part = "1";// loopback address
        str += v4part;
        return str;
      }
    }

    // Handle all other IPv6 addresses

    // first run to find the longest contiguous zero words
    for (word = 0; word < 8; word++) {
      if (parts[word] === 0) {
        if (word - lastzero > 1) {
          len = 0;
        }
        lastzero = word;
        len++;
      }
      if (len > longest) {
        longest = len;
        zstart = word - longest + 1;
      }
    }

    for (word = 0; word < 8; word++) {
      if (longest > 1) {
        // compress contiguous zeros - to produce "::"
        if (parts[word] === 0 && word >= zstart && word < (zstart + longest) ) {
          if (word === zstart) {
            str += ":";
            if (zstart === 0) str += ":"; //leading zeros case
          }
          continue;
        }
      }
      // converts 16-bit words from big-endian to little-endian before converting to hex string
      str += Number(_ntohs(parts[word] & 0xffff)).toString(16);
      str += word < 7 ? ":" : "";
    }
    return str;
  },

  _read_sockaddr__deps: ['$Sockets', '_inet_ntop4_raw', '_inet_ntop6_raw'],
  _read_sockaddr: function (sa, salen) {
    // family / port offsets are common to both sockaddr_in and sockaddr_in6
    var family = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'i16') }}};
    var port = _ntohs({{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_port, 'i16') }}});
    var addr;

    switch (family) {
      case {{{ cDefine('AF_INET') }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in.__size__ }}}) {
          return { errno: ERRNO_CODES.EINVAL };
        }
        addr = {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'i32') }}};
        addr = __inet_ntop4_raw(addr);
        break;
      case {{{ cDefine('AF_INET6') }}}:
        if (salen !== {{{ C_STRUCTS.sockaddr_in6.__size__ }}}) {
          return { errno: ERRNO_CODES.EINVAL };
        }
        addr = [
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+0, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+4, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+8, 'i32') }}},
          {{{ makeGetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+12, 'i32') }}}
        ];
        addr = __inet_ntop6_raw(addr);
        break;
      default:
        return { errno: ERRNO_CODES.EAFNOSUPPORT };
    }

    return { family: family, addr: addr, port: port };
  },
  _write_sockaddr__deps: ['$Sockets', '_inet_pton4_raw', '_inet_pton6_raw'],
  _write_sockaddr: function (sa, family, addr, port) {
    switch (family) {
      case {{{ cDefine('AF_INET') }}}:
        addr = __inet_pton4_raw(addr);
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_family, 'family', 'i16') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_addr.s_addr, 'addr', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in.sin_port, '_htons(port)', 'i16') }}};
        break;
      case {{{ cDefine('AF_INET6') }}}:
        addr = __inet_pton6_raw(addr);
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_family, 'family', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+0, 'addr[0]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+4, 'addr[1]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+8, 'addr[2]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_addr.__in6_union.__s6_addr+12, 'addr[3]', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_port, '_htons(port)', 'i16') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_flowinfo, '0', 'i32') }}};
        {{{ makeSetValue('sa', C_STRUCTS.sockaddr_in6.sin6_scope_id, '0', 'i32') }}};
        break;
      default:
        return { errno: ERRNO_CODES.EAFNOSUPPORT };
    }
    // kind of lame, but let's match _read_sockaddr's interface
    return {};
  },

  // We can't actually resolve hostnames in the browser, so instead
  // we're generating fake IP addresses with lookup_name that we can
  // resolve later on with lookup_addr.
  // We do the aliasing in 172.29.*.*, giving us 65536 possibilities.
  $DNS__deps: ['_inet_pton4_raw', '_inet_pton6_raw'],
  $DNS: {
    address_map: {
      id: 1,
      addrs: {},
      names: {}
    },

    lookup_name: function (name) {
      // If the name is already a valid ipv4 / ipv6 address, don't generate a fake one.
      var res = __inet_pton4_raw(name);
      if (res !== null) {
        return name;
      }
      res = __inet_pton6_raw(name);
      if (res !== null) {
        return name;
      }

      // See if this name is already mapped.
      var addr;

      if (DNS.address_map.addrs[name]) {
        addr = DNS.address_map.addrs[name];
      } else {
        var id = DNS.address_map.id++;
        assert(id < 65535, 'exceeded max address mappings of 65535');

        addr = '172.29.' + (id & 0xff) + '.' + (id & 0xff00);

        DNS.address_map.names[addr] = name;
        DNS.address_map.addrs[name] = addr;
      }

      return addr;
    },

    lookup_addr: function (addr) {
      if (DNS.address_map.names[addr]) {
        return DNS.address_map.names[addr];
      }

      return null;
    }
  },

  // note: lots of leaking here!
  gethostbyaddr__deps: ['$DNS', 'gethostbyname', '_inet_ntop4_raw'],
  gethostbyaddr: function (addr, addrlen, type) {
    if (type !== {{{ cDefine('AF_INET') }}}) {
      ___setErrNo(ERRNO_CODES.EAFNOSUPPORT);
      // TODO: set h_errno
      return null;
    }
    addr = {{{ makeGetValue('addr', '0', 'i32') }}}; // addr is in_addr
    var host = __inet_ntop4_raw(addr);
    var lookup = DNS.lookup_addr(host);
    if (lookup) {
      host = lookup;
    }
    var hostp = allocate(intArrayFromString(host), 'i8', ALLOC_STACK);
    return _gethostbyname(hostp);
  },

  gethostbyname__deps: ['$DNS', '_inet_pton4_raw'],
  gethostbyname: function(name) {
    name = Pointer_stringify(name);

    // generate hostent
    var ret = _malloc({{{ C_STRUCTS.hostent.__size__ }}}); // XXX possibly leaked, as are others here
    var nameBuf = _malloc(name.length+1);
    stringToUTF8(name, nameBuf, name.length+1);
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_name, 'nameBuf', 'i8*') }}};
    var aliasesBuf = _malloc(4);
    {{{ makeSetValue('aliasesBuf', '0', '0', 'i8*') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_aliases, 'aliasesBuf', 'i8**') }}};
    var afinet = {{{ cDefine('AF_INET') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_addrtype, 'afinet', 'i32') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_length, '4', 'i32') }}};
    var addrListBuf = _malloc(12);
    {{{ makeSetValue('addrListBuf', '0', 'addrListBuf+8', 'i32*') }}};
    {{{ makeSetValue('addrListBuf', '4', '0', 'i32*') }}};
    {{{ makeSetValue('addrListBuf', '8', '__inet_pton4_raw(DNS.lookup_name(name))', 'i32') }}};
    {{{ makeSetValue('ret', C_STRUCTS.hostent.h_addr_list, 'addrListBuf', 'i8**') }}};
    return ret;
  },

  gethostbyname_r__deps: ['gethostbyname'],
  gethostbyname_r: function(name, ret, buf, buflen, out, err) {
    var data = _gethostbyname(name);
    _memcpy(ret, data, {{{ C_STRUCTS.hostent.__size__ }}});
    _free(data);
    {{{ makeSetValue('err', '0', '0', 'i32') }}};
    {{{ makeSetValue('out', '0', 'ret', '*') }}};
    return 0;
  },

  getaddrinfo__deps: ['$Sockets', '$DNS', '_inet_pton4_raw', '_inet_ntop4_raw', '_inet_pton6_raw', '_inet_ntop6_raw', '_write_sockaddr'],
  getaddrinfo: function(node, service, hint, out) {
    // Note getaddrinfo currently only returns a single addrinfo with ai_next defaulting to NULL. When NULL
    // hints are specified or ai_family set to AF_UNSPEC or ai_socktype or ai_protocol set to 0 then we
    // really should provide a linked list of suitable addrinfo values.
    var addrs = [];
    var canon = null;
    var addr = 0;
    var port = 0;
    var flags = 0;
    var family = {{{ cDefine('AF_UNSPEC') }}};
    var type = 0;
    var proto = 0;
    var ai, last;

    function allocaddrinfo(family, type, proto, canon, addr, port) {
      var sa, salen, ai;
      var res;

      salen = family === {{{ cDefine('AF_INET6') }}} ?
        {{{ C_STRUCTS.sockaddr_in6.__size__ }}} :
        {{{ C_STRUCTS.sockaddr_in.__size__ }}};
      addr = family === {{{ cDefine('AF_INET6') }}} ?
        __inet_ntop6_raw(addr) :
        __inet_ntop4_raw(addr);
      sa = _malloc(salen);
      res = __write_sockaddr(sa, family, addr, port);
      assert(!res.errno);

      ai = _malloc({{{ C_STRUCTS.addrinfo.__size__ }}});
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_family, 'family', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_socktype, 'type', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_protocol, 'proto', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_canonname, 'canon', 'i32') }}};
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addr, 'sa', '*') }}};
      if (family === {{{ cDefine('AF_INET6') }}}) {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in6.__size__, 'i32') }}};
      } else {
        {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_addrlen, C_STRUCTS.sockaddr_in.__size__, 'i32') }}};
      }
      {{{ makeSetValue('ai', C_STRUCTS.addrinfo.ai_next, '0', 'i32') }}};

      return ai;
    }

    if (hint) {
      flags = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}};
      family = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_family, 'i32') }}};
      type = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_socktype, 'i32') }}};
      proto = {{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_protocol, 'i32') }}};
    }
    if (type && !proto) {
      proto = type === {{{ cDefine('SOCK_DGRAM') }}} ? {{{ cDefine('IPPROTO_UDP') }}} : {{{ cDefine('IPPROTO_TCP') }}};
    }
    if (!type && proto) {
      type = proto === {{{ cDefine('IPPROTO_UDP') }}} ? {{{ cDefine('SOCK_DGRAM') }}} : {{{ cDefine('SOCK_STREAM') }}};
    }

    // If type or proto are set to zero in hints we should really be returning multiple addrinfo values, but for
    // now default to a TCP STREAM socket so we can at least return a sensible addrinfo given NULL hints.
    if (proto === 0) {
      proto = {{{ cDefine('IPPROTO_TCP') }}};
    }
    if (type === 0) {
      type = {{{ cDefine('SOCK_STREAM') }}};
    }

    if (!node && !service) {
      return {{{ cDefine('EAI_NONAME') }}};
    }
    if (flags & ~({{{ cDefine('AI_PASSIVE') }}}|{{{ cDefine('AI_CANONNAME') }}}|{{{ cDefine('AI_NUMERICHOST') }}}|
        {{{ cDefine('AI_NUMERICSERV') }}}|{{{ cDefine('AI_V4MAPPED') }}}|{{{ cDefine('AI_ALL') }}}|{{{ cDefine('AI_ADDRCONFIG') }}})) {
      return {{{ cDefine('EAI_BADFLAGS') }}};
    }
    if (hint !== 0 && ({{{ makeGetValue('hint', C_STRUCTS.addrinfo.ai_flags, 'i32') }}} & {{{ cDefine('AI_CANONNAME') }}}) && !node) {
      return {{{ cDefine('EAI_BADFLAGS') }}};
    }
    if (flags & {{{ cDefine('AI_ADDRCONFIG') }}}) {
      // TODO
      return {{{ cDefine('EAI_NONAME') }}};
    }
    if (type !== 0 && type !== {{{ cDefine('SOCK_STREAM') }}} && type !== {{{ cDefine('SOCK_DGRAM') }}}) {
      return {{{ cDefine('EAI_SOCKTYPE') }}};
    }
    if (family !== {{{ cDefine('AF_UNSPEC') }}} && family !== {{{ cDefine('AF_INET') }}} && family !== {{{ cDefine('AF_INET6') }}}) {
      return {{{ cDefine('EAI_FAMILY') }}};
    }

    if (service) {
      service = Pointer_stringify(service);
      port = parseInt(service, 10);

      if (isNaN(port)) {
        if (flags & {{{ cDefine('AI_NUMERICSERV') }}}) {
          return {{{ cDefine('EAI_NONAME') }}};
        }
        // TODO support resolving well-known service names from:
        // http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
        return {{{ cDefine('EAI_SERVICE') }}};
      }
    }

    if (!node) {
      if (family === {{{ cDefine('AF_UNSPEC') }}}) {
        family = {{{ cDefine('AF_INET') }}};
      }
      if ((flags & {{{ cDefine('AI_PASSIVE') }}}) === 0) {
        if (family === {{{ cDefine('AF_INET') }}}) {
          addr = _htonl({{{ cDefine('INADDR_LOOPBACK') }}});
        } else {
          addr = [0, 0, 0, 1];
        }
      }
      ai = allocaddrinfo(family, type, proto, null, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }

    //
    // try as a numeric address
    //
    node = Pointer_stringify(node);
    addr = __inet_pton4_raw(node);
    if (addr !== null) {
      // incoming node is a valid ipv4 address
      if (family === {{{ cDefine('AF_UNSPEC') }}} || family === {{{ cDefine('AF_INET') }}}) {
        family = {{{ cDefine('AF_INET') }}};
      }
      else if (family === {{{ cDefine('AF_INET6') }}} && (flags & {{{ cDefine('AI_V4MAPPED') }}})) {
        addr = [0, 0, _htonl(0xffff), addr];
        family = {{{ cDefine('AF_INET6') }}};
      } else {
        return {{{ cDefine('EAI_NONAME') }}};
      }
    } else {
      addr = __inet_pton6_raw(node);
      if (addr !== null) {
        // incoming node is a valid ipv6 address
        if (family === {{{ cDefine('AF_UNSPEC') }}} || family === {{{ cDefine('AF_INET6') }}}) {
          family = {{{ cDefine('AF_INET6') }}};
        } else {
          return {{{ cDefine('EAI_NONAME') }}};
        }
      }
    }
    if (addr != null) {
      ai = allocaddrinfo(family, type, proto, node, addr, port);
      {{{ makeSetValue('out', '0', 'ai', '*') }}};
      return 0;
    }
    if (flags & {{{ cDefine('AI_NUMERICHOST') }}}) {
      return {{{ cDefine('EAI_NONAME') }}};
    }

    //
    // try as a hostname
    //
    // resolve the hostname to a temporary fake address
    node = DNS.lookup_name(node);
    addr = __inet_pton4_raw(node);
    if (family === {{{ cDefine('AF_UNSPEC') }}}) {
      family = {{{ cDefine('AF_INET') }}};
    } else if (family === {{{ cDefine('AF_INET6') }}}) {
      addr = [0, 0, _htonl(0xffff), addr];
    }
    ai = allocaddrinfo(family, type, proto, null, addr, port);
    {{{ makeSetValue('out', '0', 'ai', '*') }}};
    return 0;
  },

  getnameinfo__deps: ['$Sockets', '$DNS', '_read_sockaddr'],
  getnameinfo: function (sa, salen, node, nodelen, serv, servlen, flags) {
    var info = __read_sockaddr(sa, salen);
    if (info.errno) {
      return {{{ cDefine('EAI_FAMILY') }}};
    }
    var port = info.port;
    var addr = info.addr;

    var overflowed = false;

    if (node && nodelen) {
      var lookup;
      if ((flags & {{{ cDefine('NI_NUMERICHOST') }}}) || !(lookup = DNS.lookup_addr(addr))) {
        if (flags & {{{ cDefine('NI_NAMEREQD') }}}) {
          return {{{ cDefine('EAI_NONAME') }}};
        }
      } else {
        addr = lookup;
      }
      var numBytesWrittenExclNull = stringToUTF8(addr, node, nodelen);

      if (numBytesWrittenExclNull+1 >= nodelen) {
        overflowed = true;
      }
    }

    if (serv && servlen) {
      port = '' + port;
      var numBytesWrittenExclNull = stringToUTF8(port, serv, servlen);

      if (numBytesWrittenExclNull+1 >= servlen) {
        overflowed = true;
      }
    }

    if (overflowed) {
      // Note: even when we overflow, getnameinfo() is specced to write out the truncated results.
      return {{{ cDefine('EAI_OVERFLOW') }}};
    }

    return 0;
  },
  // Can't use a literal for $GAI_ERRNO_MESSAGES as was done for $ERRNO_MESSAGES as the keys (e.g. EAI_BADFLAGS)
  // are actually negative numbers and you can't have expressions as keys in JavaScript literals.
  $GAI_ERRNO_MESSAGES: {},

  gai_strerror__deps: ['$GAI_ERRNO_MESSAGES'],
  gai_strerror: function(val) {
    var buflen = 256;

    // On first call to gai_strerror we initialise the buffer and populate the error messages.
    if (!_gai_strerror.buffer) {
        _gai_strerror.buffer = _malloc(buflen);

        GAI_ERRNO_MESSAGES['0'] = 'Success';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_BADFLAGS') }}}] = 'Invalid value for \'ai_flags\' field';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_NONAME') }}}] = 'NAME or SERVICE is unknown';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_AGAIN') }}}] = 'Temporary failure in name resolution';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_FAIL') }}}] = 'Non-recoverable failure in name res';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_FAMILY') }}}] = '\'ai_family\' not supported';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_SOCKTYPE') }}}] = '\'ai_socktype\' not supported';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_SERVICE') }}}] = 'SERVICE not supported for \'ai_socktype\'';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_MEMORY') }}}] = 'Memory allocation failure';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_SYSTEM') }}}] = 'System error returned in \'errno\'';
        GAI_ERRNO_MESSAGES['' + {{{ cDefine('EAI_OVERFLOW') }}}] = 'Argument buffer overflow';
    }

    var msg = 'Unknown error';

    if (val in GAI_ERRNO_MESSAGES) {
      if (GAI_ERRNO_MESSAGES[val].length > buflen - 1) {
        msg = 'Message too long'; // EMSGSIZE message. This should never occur given the GAI_ERRNO_MESSAGES above. 
      } else {
        msg = GAI_ERRNO_MESSAGES[val];
      }
    }

    writeAsciiToMemory(msg, _gai_strerror.buffer);
    return _gai_strerror.buffer;
  },

  // Implement netdb.h protocol entry (getprotoent, getprotobyname, getprotobynumber, setprotoent, endprotoent)
  // http://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobyname.html
  // The Protocols object holds our 'fake' protocols 'database'.
  $Protocols: {
    list: [],
    map: {}
  },
  setprotoent__deps: ['$Protocols'],
  setprotoent: function(stayopen) {
    // void setprotoent(int stayopen);

    // Allocate and populate a protoent structure given a name, protocol number and array of aliases
    function allocprotoent(name, proto, aliases) {
      // write name into buffer
      var nameBuf = _malloc(name.length + 1);
      writeAsciiToMemory(name, nameBuf);

      // write aliases into buffer
      var j = 0;
      var length = aliases.length;
      var aliasListBuf = _malloc((length + 1) * 4); // Use length + 1 so we have space for the terminating NULL ptr.

      for (var i = 0; i < length; i++, j += 4) {
        var alias = aliases[i];
        var aliasBuf = _malloc(alias.length + 1);
        writeAsciiToMemory(alias, aliasBuf);
        {{{ makeSetValue('aliasListBuf', 'j', 'aliasBuf', 'i8*') }}};
      }
      {{{ makeSetValue('aliasListBuf', 'j', '0', 'i8*') }}}; // Terminating NULL pointer.

      // generate protoent
      var pe = _malloc({{{ C_STRUCTS.protoent.__size__ }}});
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_name, 'nameBuf', 'i8*') }}};
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_aliases, 'aliasListBuf', 'i8**') }}};
      {{{ makeSetValue('pe', C_STRUCTS.protoent.p_proto, 'proto', 'i32') }}};
      return pe;
    };

    // Populate the protocol 'database'. The entries are limited to tcp and udp, though it is fairly trivial
    // to add extra entries from /etc/protocols if desired - though not sure if that'd actually be useful.
    var list = Protocols.list;
    var map  = Protocols.map;
    if (list.length === 0) {
        var entry = allocprotoent('tcp', 6, ['TCP']);
        list.push(entry);
        map['tcp'] = map['6'] = entry;
        entry = allocprotoent('udp', 17, ['UDP']);
        list.push(entry);
        map['udp'] = map['17'] = entry;
    }

    _setprotoent.index = 0;
  },

  endprotoent: function() {
    // void endprotoent(void);
    // We're not using a real protocol database so we don't do a real close.
  },

  getprotoent__deps: ['setprotoent', '$Protocols'],
  getprotoent: function(number) {
    // struct protoent *getprotoent(void);
    // reads the  next  entry  from  the  protocols 'database' or return NULL if 'eof'
    if (_setprotoent.index === Protocols.list.length) {
      return 0; 
    } else {
      var result = Protocols.list[_setprotoent.index++];
      return result;
    }
  },

  getprotobyname__deps: ['setprotoent', '$Protocols'],
  getprotobyname: function(name) {
    // struct protoent *getprotobyname(const char *);
    name = Pointer_stringify(name);
    _setprotoent(true);
    var result = Protocols.map[name];
    return result;
  },

  getprotobynumber__deps: ['setprotoent', '$Protocols'],
  getprotobynumber: function(number) {
    // struct protoent *getprotobynumber(int proto);
    _setprotoent(true);
    var result = Protocols.map[number];
    return result;
  },

  // ==========================================================================
  // sockets. Note that the implementation assumes all sockets are always
  // nonblocking
  // ==========================================================================
#if SOCKET_WEBRTC
  $Sockets__deps: ['__setErrNo', '$ERRNO_CODES',
    function() { return 'var SocketIO = ' + read('socket.io.js') + ';\n' },
    function() { return 'var Peer = ' + read('wrtcp.js') + ';\n' }],
#else
  $Sockets__deps: ['__setErrNo', '$ERRNO_CODES'],
#endif
  $Sockets: {
    BUFFER_SIZE: 10*1024, // initial size
    MAX_BUFFER_SIZE: 10*1024*1024, // maximum size we will grow the buffer

    nextFd: 1,
    fds: {},
    nextport: 1,
    maxport: 65535,
    peer: null,
    connections: {},
    portmap: {},
    localAddr: 0xfe00000a, // Local address is always 10.0.0.254
    addrPool: [            0x0200000a, 0x0300000a, 0x0400000a, 0x0500000a,
               0x0600000a, 0x0700000a, 0x0800000a, 0x0900000a, 0x0a00000a,
               0x0b00000a, 0x0c00000a, 0x0d00000a, 0x0e00000a] /* 0x0100000a is reserved */
  },

  // pwd.h

  getpwnam: function() { throw 'getpwnam: TODO' },
  setpwent: function() { throw 'setpwent: TODO' },
  getpwent: function() { throw 'getpwent: TODO' },
  endpwent: function() { throw 'endpwent: TODO' },

  // ==========================================================================
  // emscripten.h
  // ==========================================================================

  emscripten_run_script: function(ptr) {
    {{{ makeEval('eval(Pointer_stringify(ptr));') }}}
  },

  emscripten_run_script_int: function(ptr) {
    {{{ makeEval('return eval(Pointer_stringify(ptr))|0;') }}}
  },

  emscripten_run_script_string: function(ptr) {
    {{{ makeEval("var s = eval(Pointer_stringify(ptr)) + '';") }}}
    var me = _emscripten_run_script_string;
    var len = lengthBytesUTF8(s);
    if (!me.bufferSize || me.bufferSize < len+1) {
      if (me.bufferSize) _free(me.buffer);
      me.bufferSize = len+1;
      me.buffer = _malloc(me.bufferSize);
    }
    stringToUTF8(s, me.buffer, me.bufferSize);
    return me.buffer;
  },

  emscripten_random: function() {
    return Math.random();
  },

  emscripten_get_now: function() { abort() }, // replaced by the postset at startup time
  emscripten_get_now__postset: "if (ENVIRONMENT_IS_NODE) {\n" +
                               "  _emscripten_get_now = function _emscripten_get_now_actual() {\n" +
                               "    var t = process['hrtime']();\n" +
                               "    return t[0] * 1e3 + t[1] / 1e6;\n" +
                               "  };\n" +
                               "} else if (typeof dateNow !== 'undefined') {\n" +
                               "  _emscripten_get_now = dateNow;\n" +
                               "} else if (typeof self === 'object' && self['performance'] && typeof self['performance']['now'] === 'function') {\n" +
                               "  _emscripten_get_now = function() { return self['performance']['now'](); };\n" +
                               "} else if (typeof performance === 'object' && typeof performance['now'] === 'function') {\n" +
                               "  _emscripten_get_now = function() { return performance['now'](); };\n" +
                               "} else {\n" +
                               "  _emscripten_get_now = Date.now;\n" +
                               "}",

  emscripten_get_now_res: function() { // return resolution of get_now, in nanoseconds
    if (ENVIRONMENT_IS_NODE) {
      return 1; // nanoseconds
    } else if (typeof dateNow !== 'undefined' ||
               ((ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && self['performance'] && self['performance']['now'])) {
      return 1000; // microseconds (1/1000 of a millisecond)
    } else {
      return 1000*1000; // milliseconds
    }
  },

  emscripten_get_now_is_monotonic__deps: ['emscripten_get_now'],
  emscripten_get_now_is_monotonic: function() {
    // return whether emscripten_get_now is guaranteed monotonic; the Date.now
    // implementation is not :(
    return ENVIRONMENT_IS_NODE || (typeof dateNow !== 'undefined') ||
        ((ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && self['performance'] && self['performance']['now']);
  },

  // Returns [parentFuncArguments, functionName, paramListName]
  _emscripten_traverse_stack: function(args) {
    if (!args || !args.callee || !args.callee.name) {
      return [null, '', ''];
    }

    var funstr = args.callee.toString();
    var funcname = args.callee.name;
    var str = '(';
    var first = true;
    for(i in args) {
      var a = args[i];
      if (!first) {
        str += ", ";
      }
      first = false;
      if (typeof a === 'number' || typeof a === 'string') {
        str += a;
      } else {
        str += '(' + typeof a + ')';
      }
    }
    str += ')';
    var caller = args.callee.caller;
    args = caller ? caller.arguments : [];
    if (first)
      str = '';
    return [args, funcname, str];
  },

  emscripten_get_callstack_js__deps: ['_emscripten_traverse_stack'],
  emscripten_get_callstack_js: function(flags) {
    var callstack = jsStackTrace();

    // Find the symbols in the callstack that corresponds to the functions that report callstack information, and remove everyhing up to these from the output.
    var iThisFunc = callstack.lastIndexOf('_emscripten_log');
    var iThisFunc2 = callstack.lastIndexOf('_emscripten_get_callstack');
    var iNextLine = callstack.indexOf('\n', Math.max(iThisFunc, iThisFunc2))+1;
    callstack = callstack.slice(iNextLine);

    // If user requested to see the original source stack, but no source map information is available, just fall back to showing the JS stack.
    if (flags & 8/*EM_LOG_C_STACK*/ && typeof emscripten_source_map === 'undefined') {
      Runtime.warnOnce('Source map information is not available, emscripten_log with EM_LOG_C_STACK will be ignored. Build with "--pre-js $EMSCRIPTEN/src/emscripten-source-map.min.js" linker flag to add source map loading to code.');
      flags ^= 8/*EM_LOG_C_STACK*/;
      flags |= 16/*EM_LOG_JS_STACK*/;
    }

    var stack_args = null;
    if (flags & 128 /*EM_LOG_FUNC_PARAMS*/) {
      // To get the actual parameters to the functions, traverse the stack via the unfortunately deprecated 'arguments.callee' method, if it works:
      var stack_args = __emscripten_traverse_stack(arguments);
      while (stack_args[1].indexOf('_emscripten_') >= 0)
        stack_args = __emscripten_traverse_stack(stack_args[0]);
    }
    
    // Process all lines:
    lines = callstack.split('\n');
    callstack = '';
    var newFirefoxRe = new RegExp('\\s*(.*?)@(.*?):([0-9]+):([0-9]+)'); // New FF30 with column info: extract components of form '       Object._main@http://server.com:4324:12'
    var firefoxRe = new RegExp('\\s*(.*?)@(.*):(.*)(:(.*))?'); // Old FF without column info: extract components of form '       Object._main@http://server.com:4324'
    var chromeRe = new RegExp('\\s*at (.*?) \\\((.*):(.*):(.*)\\\)'); // Extract components of form '    at Object._main (http://server.com/file.html:4324:12)'
    
    for(l in lines) {
      var line = lines[l];

      var jsSymbolName = '';
      var file = '';
      var lineno = 0;
      var column = 0;

      var parts = chromeRe.exec(line);
      if (parts && parts.length == 5) {
        jsSymbolName = parts[1];
        file = parts[2];
        lineno = parts[3];
        column = parts[4];
      } else {
        parts = newFirefoxRe.exec(line);
        if (!parts) parts = firefoxRe.exec(line);
        if (parts && parts.length >= 4) {
          jsSymbolName = parts[1];
          file = parts[2];
          lineno = parts[3];
          column = parts[4]|0; // Old Firefox doesn't carry column information, but in new FF30, it is present. See https://bugzilla.mozilla.org/show_bug.cgi?id=762556
        } else {
          // Was not able to extract this line for demangling/sourcemapping purposes. Output it as-is.
          callstack += line + '\n';
          continue;
        }
      }

      // Try to demangle the symbol, but fall back to showing the original JS symbol name if not available.
      var cSymbolName = (flags & 32/*EM_LOG_DEMANGLE*/) ? demangle(jsSymbolName) : jsSymbolName;
      if (!cSymbolName) {
        cSymbolName = jsSymbolName;
      }

      var haveSourceMap = false;

      if (flags & 8/*EM_LOG_C_STACK*/) {
        var orig = emscripten_source_map.originalPositionFor({line: lineno, column: column});
        haveSourceMap = (orig && orig.source);
        if (haveSourceMap) {
          if (flags & 64/*EM_LOG_NO_PATHS*/) {
            orig.source = orig.source.substring(orig.source.replace(/\\/g, "/").lastIndexOf('/')+1);
          }
          callstack += '    at ' + cSymbolName + ' (' + orig.source + ':' + orig.line + ':' + orig.column + ')\n';
        }
      }
      if ((flags & 16/*EM_LOG_JS_STACK*/) || !haveSourceMap) {
        if (flags & 64/*EM_LOG_NO_PATHS*/) {
          file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
        }
        callstack += (haveSourceMap ? ('     = '+jsSymbolName) : ('    at '+cSymbolName)) + ' (' + file + ':' + lineno + ':' + column + ')\n';
      }
      
      // If we are still keeping track with the callstack by traversing via 'arguments.callee', print the function parameters as well.
      if (flags & 128 /*EM_LOG_FUNC_PARAMS*/ && stack_args[0]) {
        if (stack_args[1] == jsSymbolName && stack_args[2].length > 0) {
          callstack = callstack.replace(/\s+$/, '');
          callstack += ' with values: ' + stack_args[1] + stack_args[2] + '\n';
        }
        stack_args = __emscripten_traverse_stack(stack_args[0]);
      }
    }
    // Trim extra whitespace at the end of the output.
    callstack = callstack.replace(/\s+$/, '');
    return callstack;
  },

  emscripten_get_callstack__deps: ['emscripten_get_callstack_js'],
  emscripten_get_callstack: function(flags, str, maxbytes) {
    var callstack = _emscripten_get_callstack_js(flags);
    // User can query the required amount of bytes to hold the callstack.
    if (!str || maxbytes <= 0) {
      return lengthBytesUTF8(callstack)+1;
    }
    // Output callstack string as C string to HEAP.
    var bytesWrittenExcludingNull = stringToUTF8(callstack, str, maxbytes);

    // Return number of bytes written, including null.
    return bytesWrittenExcludingNull+1;
  },

  emscripten_log_js__deps: ['emscripten_get_callstack_js'],
  emscripten_log_js: function(flags, str) {
    if (flags & 24/*EM_LOG_C_STACK | EM_LOG_JS_STACK*/) {
      str = str.replace(/\s+$/, ''); // Ensure the message and the callstack are joined cleanly with exactly one newline.
      str += (str.length > 0 ? '\n' : '') + _emscripten_get_callstack_js(flags);
    }

    if (flags & 1 /*EM_LOG_CONSOLE*/) {
      if (flags & 4 /*EM_LOG_ERROR*/) {
        console.error(str);
      } else if (flags & 2 /*EM_LOG_WARN*/) {
        console.warn(str);
      } else {
        console.log(str);
      }
    } else if (flags & 6 /*EM_LOG_ERROR|EM_LOG_WARN*/) {
      Module.printErr(str);
    } else {
      Module.print(str);
    }
  },

  emscripten_log__deps: ['_formatString', 'emscripten_log_js'],
  emscripten_log: function(flags, varargs) {
    // Extract the (optionally-existing) printf format specifier field from varargs.
    var format = {{{ makeGetValue('varargs', '0', 'i32', undefined, undefined, true) }}};
    varargs += Math.max(Runtime.getNativeFieldSize('i32'), Runtime.getAlignSize('i32', null, true));
    var str = '';
    if (format) {
      var result = __formatString(format, varargs);
      for(var i = 0 ; i < result.length; ++i) {
        str += String.fromCharCode(result[i]);
      }
    }
    _emscripten_log_js(flags, str);
  },

  emscripten_get_compiler_setting: function(name) {
    name = Pointer_stringify(name);

    var ret = Runtime.getCompilerSetting(name);
    if (typeof ret === 'number') return ret;

    if (!_emscripten_get_compiler_setting.cache) _emscripten_get_compiler_setting.cache = {};
    var cache = _emscripten_get_compiler_setting.cache;
    var fullname = name + '__str';
    var fullret = cache[fullname];
    if (fullret) return fullret;
    return cache[fullname] = allocate(intArrayFromString(ret + ''), 'i8', ALLOC_NORMAL);
  },

  emscripten_debugger: function() {
    debugger;
  },

  emscripten_print_double: function(x, to, max) {
    var str = x + '';
    if (to) return stringToUTF8(str, to, max);
    else return lengthBytesUTF8(str);
  },

  //============================
  // i64 math
  //============================

  i64Add__asm: true,
  i64Add__sig: 'iiiii',
  i64Add: function(a, b, c, d) {
    /*
      x = a + b*2^32
      y = c + d*2^32
      result = l + h*2^32
    */
    a = a|0; b = b|0; c = c|0; d = d|0;
    var l = 0, h = 0;
    l = (a + c)>>>0;
    h = (b + d + (((l>>>0) < (a>>>0))|0))>>>0; // Add carry from low word to high word on overflow.
    {{{ makeStructuralReturn(['l|0', 'h'], true) }}};
  },

  i64Subtract__asm: true,
  i64Subtract__sig: 'iiiii',
  i64Subtract: function(a, b, c, d) {
    a = a|0; b = b|0; c = c|0; d = d|0;
    var l = 0, h = 0;
    l = (a - c)>>>0;
    h = (b - d)>>>0;
    h = (b - d - (((c>>>0) > (a>>>0))|0))>>>0; // Borrow one from high word to low word on underflow.
    {{{ makeStructuralReturn(['l|0', 'h'], true) }}};
  },

  bitshift64Shl__asm: true,
  bitshift64Shl__sig: 'iiii',
  bitshift64Shl: function(low, high, bits) {
    low = low|0; high = high|0; bits = bits|0;
    var ander = 0;
    if ((bits|0) < 32) {
      ander = ((1 << bits) - 1)|0;
      {{{ makeSetTempRet0('(high << bits) | ((low&(ander << (32 - bits))) >>> (32 - bits))') }}};
      return low << bits;
    }
    {{{ makeSetTempRet0('low << (bits - 32)') }}};
    return 0;
  },
  bitshift64Ashr__asm: true,
  bitshift64Ashr__sig: 'iiii',
  bitshift64Ashr: function(low, high, bits) {
    low = low|0; high = high|0; bits = bits|0;
    var ander = 0;
    if ((bits|0) < 32) {
      ander = ((1 << bits) - 1)|0;
      {{{ makeSetTempRet0('high >> bits') }}};
      return (low >>> bits) | ((high&ander) << (32 - bits));
    }
    {{{ makeSetTempRet0('(high|0) < 0 ? -1 : 0') }}};
    return (high >> (bits - 32))|0;
  },
  bitshift64Lshr__asm: true,
  bitshift64Lshr__sig: 'iiii',
  bitshift64Lshr: function(low, high, bits) {
    low = low|0; high = high|0; bits = bits|0;
    var ander = 0;
    if ((bits|0) < 32) {
      ander = ((1 << bits) - 1)|0;
      {{{ makeSetTempRet0('high >>> bits') }}};
      return (low >>> bits) | ((high&ander) << (32 - bits));
    }
    {{{ makeSetTempRet0('0') }}};
    return (high >>> (bits - 32))|0;
  },

  // misc shims for musl
  __lock: function() {},
  __unlock: function() {},
  __lockfile: function() { return 1 },
  __unlockfile: function(){},

  // ubsan (undefined behavior sanitizer) support
  __ubsan_handle_float_cast_overflow: function(id, post) {
    abort('Undefined behavior! ubsan_handle_float_cast_overflow: ' + [id, post]);
  },

  // USE_FULL_LIBRARY hacks
  realloc: function() { throw 'bad' },

  // libunwind

  _Unwind_Backtrace__deps: ['emscripten_get_callstack_js'],
  _Unwind_Backtrace: function(func, arg) {
    var trace = _emscripten_get_callstack_js();
    var parts = trace.split('\n');
    for (var i = 0; i < parts.length; i++) {
      var ret = Module['dynCall_iii'](func, 0, arg);
      if (ret !== 0) return;
    }
  },

  _Unwind_GetIPInfo: function() {
    abort('Unwind_GetIPInfo');
  },

  _Unwind_FindEnclosingFunction: function() {
    return 0; // we cannot succeed
  },

  _Unwind_RaiseException__deps: ['__cxa_throw'],
  _Unwind_RaiseException: function(ex) {
    Module.printErr('Warning: _Unwind_RaiseException is not correctly implemented');
    return ___cxa_throw(ex, 0, 0);
  },

  _Unwind_DeleteException: function(ex) {
    Module.printErr('TODO: Unwind_DeleteException');
  },

  // autodebugging

  emscripten_autodebug_i64: function(line, valuel, valueh) {
    Module.print('AD:' + [line, valuel, valueh]);
  },
  emscripten_autodebug_i32: function(line, value) {
    Module.print('AD:' + [line, value]);
  },
  emscripten_autodebug_i16: function(line, value) {
    Module.print('AD:' + [line, value]);
  },
  emscripten_autodebug_i8: function(line, value) {
    Module.print('AD:' + [line, value]);
  },
  emscripten_autodebug_float: function(line, value) {
    Module.print('AD:' + [line, value]);
  },
  emscripten_autodebug_double: function(line, value) {
    Module.print('AD:' + [line, value]);
  },

  // misc definitions to avoid unnecessary unresolved symbols from fastcomp
  emscripten_prep_setjmp: true,
  emscripten_cleanup_setjmp: true,
  emscripten_check_longjmp: true,
  emscripten_get_longjmp_result: true,
  emscripten_setjmp: true,
  emscripten_preinvoke: true,
  emscripten_postinvoke: true,
  emscripten_resume: true,
  emscripten_landingpad: true,
  getHigh32: true,
  setHigh32: true,
  FtoILow: true,
  FtoIHigh: true,
  DtoILow: true,
  DtoIHigh: true,
  BDtoILow: true,
  BDtoIHigh: true,
  SItoF: true,
  UItoF: true,
  SItoD: true,
  UItoD: true,
  BItoD: true,
  llvm_dbg_value: true,
  llvm_debugtrap: true,
  llvm_ctlz_i32: true,
  llvm_maxnum_f32: true,
  llvm_maxnum_f64: true,
  emscripten_asm_const: true,
  emscripten_asm_const_int: true,
  emscripten_asm_const_double: true,

  // ======== compiled code from system/lib/compiler-rt , see readme therein
  __muldsi3__asm: true,
  __muldsi3__sig: 'iii',
  __muldsi3: function($a, $b) {
    $a = $a | 0;
    $b = $b | 0;
    var $1 = 0, $2 = 0, $3 = 0, $6 = 0, $8 = 0, $11 = 0, $12 = 0;
    $1 = $a & 65535;
    $2 = $b & 65535;
    $3 = Math_imul($2, $1) | 0;
    $6 = $a >>> 16;
    $8 = ($3 >>> 16) + (Math_imul($2, $6) | 0) | 0;
    $11 = $b >>> 16;
    $12 = Math_imul($11, $1) | 0;
    return ({{{ makeSetTempRet0('(($8 >>> 16) + (Math_imul($11, $6) | 0) | 0) + ((($8 & 65535) + $12 | 0) >>> 16) | 0') }}}, 0 | ($8 + $12 << 16 | $3 & 65535)) | 0;
  },
  __divdi3__sig: 'iiiii',
  __divdi3__asm: true,
  __divdi3__deps: ['__udivmoddi4', 'i64Subtract'],
  __divdi3: function($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $1$0 = 0, $1$1 = 0, $2$0 = 0, $2$1 = 0, $4$0 = 0, $4$1 = 0, $6$0 = 0, $7$0 = 0, $7$1 = 0, $8$0 = 0, $10$0 = 0;
    $1$0 = $a$1 >> 31 | (($a$1 | 0) < 0 ? -1 : 0) << 1;
    $1$1 = (($a$1 | 0) < 0 ? -1 : 0) >> 31 | (($a$1 | 0) < 0 ? -1 : 0) << 1;
    $2$0 = $b$1 >> 31 | (($b$1 | 0) < 0 ? -1 : 0) << 1;
    $2$1 = (($b$1 | 0) < 0 ? -1 : 0) >> 31 | (($b$1 | 0) < 0 ? -1 : 0) << 1;
    $4$0 = _i64Subtract($1$0 ^ $a$0 | 0, $1$1 ^ $a$1 | 0, $1$0 | 0, $1$1 | 0) | 0;
    $4$1 = {{{ makeGetTempRet0() }}};
    $6$0 = _i64Subtract($2$0 ^ $b$0 | 0, $2$1 ^ $b$1 | 0, $2$0 | 0, $2$1 | 0) | 0;
    $7$0 = $2$0 ^ $1$0;
    $7$1 = $2$1 ^ $1$1;
    $8$0 = ___udivmoddi4($4$0, $4$1, $6$0, {{{ makeGetTempRet0() }}}, 0) | 0;
    $10$0 = _i64Subtract($8$0 ^ $7$0 | 0, {{{ makeGetTempRet0() }}} ^ $7$1 | 0, $7$0 | 0, $7$1 | 0) | 0;
    return $10$0 | 0;
  },
  __remdi3__sig: 'iiiii',
  __remdi3__asm: true,
  __remdi3__deps: ['__udivmoddi4', 'i64Subtract'],
  __remdi3: function($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $rem = 0, $1$0 = 0, $1$1 = 0, $2$0 = 0, $2$1 = 0, $4$0 = 0, $4$1 = 0, $6$0 = 0, $10$0 = 0, $10$1 = 0, __stackBase__ = 0;
    __stackBase__ = STACKTOP;
    STACKTOP = STACKTOP + 16 | 0;
    $rem = __stackBase__ | 0;
    $1$0 = $a$1 >> 31 | (($a$1 | 0) < 0 ? -1 : 0) << 1;
    $1$1 = (($a$1 | 0) < 0 ? -1 : 0) >> 31 | (($a$1 | 0) < 0 ? -1 : 0) << 1;
    $2$0 = $b$1 >> 31 | (($b$1 | 0) < 0 ? -1 : 0) << 1;
    $2$1 = (($b$1 | 0) < 0 ? -1 : 0) >> 31 | (($b$1 | 0) < 0 ? -1 : 0) << 1;
    $4$0 = _i64Subtract($1$0 ^ $a$0 | 0, $1$1 ^ $a$1 | 0, $1$0 | 0, $1$1 | 0) | 0;
    $4$1 = {{{ makeGetTempRet0() }}};
    $6$0 = _i64Subtract($2$0 ^ $b$0 | 0, $2$1 ^ $b$1 | 0, $2$0 | 0, $2$1 | 0) | 0;
    ___udivmoddi4($4$0, $4$1, $6$0, {{{ makeGetTempRet0() }}}, $rem) | 0;
    $10$0 = _i64Subtract(HEAP32[$rem >> 2] ^ $1$0 | 0, HEAP32[$rem + 4 >> 2] ^ $1$1 | 0, $1$0 | 0, $1$1 | 0) | 0;
    $10$1 = {{{ makeGetTempRet0() }}};
    STACKTOP = __stackBase__;
    return ({{{ makeSetTempRet0('$10$1') }}}, $10$0) | 0;
  },
  __muldi3__sig: 'iiiii',
  __muldi3__asm: true,
  __muldi3__deps: ['__muldsi3'],
  __muldi3: function($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $x_sroa_0_0_extract_trunc = 0, $y_sroa_0_0_extract_trunc = 0, $1$0 = 0, $1$1 = 0, $2 = 0;
    $x_sroa_0_0_extract_trunc = $a$0;
    $y_sroa_0_0_extract_trunc = $b$0;
    $1$0 = ___muldsi3($x_sroa_0_0_extract_trunc, $y_sroa_0_0_extract_trunc) | 0;
    $1$1 = {{{ makeGetTempRet0() }}};
    $2 = Math_imul($a$1, $y_sroa_0_0_extract_trunc) | 0;
    return ({{{ makeSetTempRet0('((Math_imul($b$1, $x_sroa_0_0_extract_trunc) | 0) + $2 | 0) + $1$1 | $1$1 & 0') }}}, 0 | $1$0 & -1) | 0;
  },
  __udivdi3__sig: 'iiiii',
  __udivdi3__asm: true,
  __udivdi3__deps: ['__udivmoddi4'],
  __udivdi3: function($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $1$0 = 0;
    $1$0 = ___udivmoddi4($a$0, $a$1, $b$0, $b$1, 0) | 0;
    return $1$0 | 0;
  },
  __uremdi3__sig: 'iiiii',
  __uremdi3__asm: true,
  __uremdi3__deps: ['__udivmoddi4'],
  __uremdi3: function($a$0, $a$1, $b$0, $b$1) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    var $rem = 0, __stackBase__ = 0;
    __stackBase__ = STACKTOP;
    STACKTOP = STACKTOP + 16 | 0;
    $rem = __stackBase__ | 0;
    ___udivmoddi4($a$0, $a$1, $b$0, $b$1, $rem) | 0;
    STACKTOP = __stackBase__;
    return ({{{ makeSetTempRet0('HEAP32[$rem + 4 >> 2] | 0') }}}, HEAP32[$rem >> 2] | 0) | 0;
  },
  __udivmoddi4__sig: 'iiiiii',
  __udivmoddi4__asm: true,
  __udivmoddi4__deps: ['i64Add', 'i64Subtract', 'llvm_cttz_i32'],
  __udivmoddi4: function($a$0, $a$1, $b$0, $b$1, $rem) {
    $a$0 = $a$0 | 0;
    $a$1 = $a$1 | 0;
    $b$0 = $b$0 | 0;
    $b$1 = $b$1 | 0;
    $rem = $rem | 0;
    var $n_sroa_0_0_extract_trunc = 0, $n_sroa_1_4_extract_shift$0 = 0, $n_sroa_1_4_extract_trunc = 0, $d_sroa_0_0_extract_trunc = 0, $d_sroa_1_4_extract_shift$0 = 0, $d_sroa_1_4_extract_trunc = 0, $4 = 0, $17 = 0, $37 = 0, $49 = 0, $51 = 0, $57 = 0, $58 = 0, $66 = 0, $78 = 0, $86 = 0, $88 = 0, $89 = 0, $91 = 0, $92 = 0, $95 = 0, $105 = 0, $117 = 0, $119 = 0, $125 = 0, $126 = 0, $130 = 0, $q_sroa_1_1_ph = 0, $q_sroa_0_1_ph = 0, $r_sroa_1_1_ph = 0, $r_sroa_0_1_ph = 0, $sr_1_ph = 0, $d_sroa_0_0_insert_insert99$0 = 0, $d_sroa_0_0_insert_insert99$1 = 0, $137$0 = 0, $137$1 = 0, $carry_0203 = 0, $sr_1202 = 0, $r_sroa_0_1201 = 0, $r_sroa_1_1200 = 0, $q_sroa_0_1199 = 0, $q_sroa_1_1198 = 0, $147 = 0, $149 = 0, $r_sroa_0_0_insert_insert42$0 = 0, $r_sroa_0_0_insert_insert42$1 = 0, $150$1 = 0, $151$0 = 0, $152 = 0, $154$0 = 0, $r_sroa_0_0_extract_trunc = 0, $r_sroa_1_4_extract_trunc = 0, $155 = 0, $carry_0_lcssa$0 = 0, $carry_0_lcssa$1 = 0, $r_sroa_0_1_lcssa = 0, $r_sroa_1_1_lcssa = 0, $q_sroa_0_1_lcssa = 0, $q_sroa_1_1_lcssa = 0, $q_sroa_0_0_insert_ext75$0 = 0, $q_sroa_0_0_insert_ext75$1 = 0, $q_sroa_0_0_insert_insert77$1 = 0, $_0$0 = 0, $_0$1 = 0;
    $n_sroa_0_0_extract_trunc = $a$0;
    $n_sroa_1_4_extract_shift$0 = $a$1;
    $n_sroa_1_4_extract_trunc = $n_sroa_1_4_extract_shift$0;
    $d_sroa_0_0_extract_trunc = $b$0;
    $d_sroa_1_4_extract_shift$0 = $b$1;
    $d_sroa_1_4_extract_trunc = $d_sroa_1_4_extract_shift$0;
    if (($n_sroa_1_4_extract_trunc | 0) == 0) {
      $4 = ($rem | 0) != 0;
      if (($d_sroa_1_4_extract_trunc | 0) == 0) {
        if ($4) {
          HEAP32[$rem >> 2] = ($n_sroa_0_0_extract_trunc >>> 0) % ($d_sroa_0_0_extract_trunc >>> 0);
          HEAP32[$rem + 4 >> 2] = 0;
        }
        $_0$1 = 0;
        $_0$0 = ($n_sroa_0_0_extract_trunc >>> 0) / ($d_sroa_0_0_extract_trunc >>> 0) >>> 0;
        return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
      } else {
        if (!$4) {
          $_0$1 = 0;
          $_0$0 = 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        HEAP32[$rem >> 2] = $a$0 & -1;
        HEAP32[$rem + 4 >> 2] = $a$1 & 0;
        $_0$1 = 0;
        $_0$0 = 0;
        return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
      }
    }
    $17 = ($d_sroa_1_4_extract_trunc | 0) == 0;
    do {
      if (($d_sroa_0_0_extract_trunc | 0) == 0) {
        if ($17) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = ($n_sroa_1_4_extract_trunc >>> 0) % ($d_sroa_0_0_extract_trunc >>> 0);
            HEAP32[$rem + 4 >> 2] = 0;
          }
          $_0$1 = 0;
          $_0$0 = ($n_sroa_1_4_extract_trunc >>> 0) / ($d_sroa_0_0_extract_trunc >>> 0) >>> 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        if (($n_sroa_0_0_extract_trunc | 0) == 0) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = 0;
            HEAP32[$rem + 4 >> 2] = ($n_sroa_1_4_extract_trunc >>> 0) % ($d_sroa_1_4_extract_trunc >>> 0);
          }
          $_0$1 = 0;
          $_0$0 = ($n_sroa_1_4_extract_trunc >>> 0) / ($d_sroa_1_4_extract_trunc >>> 0) >>> 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        $37 = $d_sroa_1_4_extract_trunc - 1 | 0;
        if (($37 & $d_sroa_1_4_extract_trunc | 0) == 0) {
          if (($rem | 0) != 0) {
            HEAP32[$rem >> 2] = 0 | $a$0 & -1;
            HEAP32[$rem + 4 >> 2] = $37 & $n_sroa_1_4_extract_trunc | $a$1 & 0;
          }
          $_0$1 = 0;
          $_0$0 = $n_sroa_1_4_extract_trunc >>> ((_llvm_cttz_i32($d_sroa_1_4_extract_trunc | 0) | 0) >>> 0);
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        $49 = Math_clz32($d_sroa_1_4_extract_trunc | 0) | 0;
        $51 = $49 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
        if ($51 >>> 0 <= 30) {
          $57 = $51 + 1 | 0;
          $58 = 31 - $51 | 0;
          $sr_1_ph = $57;
          $r_sroa_0_1_ph = $n_sroa_1_4_extract_trunc << $58 | $n_sroa_0_0_extract_trunc >>> ($57 >>> 0);
          $r_sroa_1_1_ph = $n_sroa_1_4_extract_trunc >>> ($57 >>> 0);
          $q_sroa_0_1_ph = 0;
          $q_sroa_1_1_ph = $n_sroa_0_0_extract_trunc << $58;
          break;
        }
        if (($rem | 0) == 0) {
          $_0$1 = 0;
          $_0$0 = 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        HEAP32[$rem >> 2] = 0 | $a$0 & -1;
        HEAP32[$rem + 4 >> 2] = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
        $_0$1 = 0;
        $_0$0 = 0;
        return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
      } else {
        if (!$17) {
          $117 = Math_clz32($d_sroa_1_4_extract_trunc | 0) | 0;
          $119 = $117 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
          if ($119 >>> 0 <= 31) {
            $125 = $119 + 1 | 0;
            $126 = 31 - $119 | 0;
            $130 = $119 - 31 >> 31;
            $sr_1_ph = $125;
            $r_sroa_0_1_ph = $n_sroa_0_0_extract_trunc >>> ($125 >>> 0) & $130 | $n_sroa_1_4_extract_trunc << $126;
            $r_sroa_1_1_ph = $n_sroa_1_4_extract_trunc >>> ($125 >>> 0) & $130;
            $q_sroa_0_1_ph = 0;
            $q_sroa_1_1_ph = $n_sroa_0_0_extract_trunc << $126;
            break;
          }
          if (($rem | 0) == 0) {
            $_0$1 = 0;
            $_0$0 = 0;
            return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
          }
          HEAP32[$rem >> 2] = 0 | $a$0 & -1;
          HEAP32[$rem + 4 >> 2] = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
          $_0$1 = 0;
          $_0$0 = 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
        $66 = $d_sroa_0_0_extract_trunc - 1 | 0;
        if (($66 & $d_sroa_0_0_extract_trunc | 0) != 0) {
          $86 = (Math_clz32($d_sroa_0_0_extract_trunc | 0) | 0) + 33 | 0;
          $88 = $86 - (Math_clz32($n_sroa_1_4_extract_trunc | 0) | 0) | 0;
          $89 = 64 - $88 | 0;
          $91 = 32 - $88 | 0;
          $92 = $91 >> 31;
          $95 = $88 - 32 | 0;
          $105 = $95 >> 31;
          $sr_1_ph = $88;
          $r_sroa_0_1_ph = $91 - 1 >> 31 & $n_sroa_1_4_extract_trunc >>> ($95 >>> 0) | ($n_sroa_1_4_extract_trunc << $91 | $n_sroa_0_0_extract_trunc >>> ($88 >>> 0)) & $105;
          $r_sroa_1_1_ph = $105 & $n_sroa_1_4_extract_trunc >>> ($88 >>> 0);
          $q_sroa_0_1_ph = $n_sroa_0_0_extract_trunc << $89 & $92;
          $q_sroa_1_1_ph = ($n_sroa_1_4_extract_trunc << $89 | $n_sroa_0_0_extract_trunc >>> ($95 >>> 0)) & $92 | $n_sroa_0_0_extract_trunc << $91 & $88 - 33 >> 31;
          break;
        }
        if (($rem | 0) != 0) {
          HEAP32[$rem >> 2] = $66 & $n_sroa_0_0_extract_trunc;
          HEAP32[$rem + 4 >> 2] = 0;
        }
        if (($d_sroa_0_0_extract_trunc | 0) == 1) {
          $_0$1 = $n_sroa_1_4_extract_shift$0 | $a$1 & 0;
          $_0$0 = 0 | $a$0 & -1;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        } else {
          $78 = _llvm_cttz_i32($d_sroa_0_0_extract_trunc | 0) | 0;
          $_0$1 = 0 | $n_sroa_1_4_extract_trunc >>> ($78 >>> 0);
          $_0$0 = $n_sroa_1_4_extract_trunc << 32 - $78 | $n_sroa_0_0_extract_trunc >>> ($78 >>> 0) | 0;
          return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
        }
      }
    } while (0);
    if (($sr_1_ph | 0) == 0) {
      $q_sroa_1_1_lcssa = $q_sroa_1_1_ph;
      $q_sroa_0_1_lcssa = $q_sroa_0_1_ph;
      $r_sroa_1_1_lcssa = $r_sroa_1_1_ph;
      $r_sroa_0_1_lcssa = $r_sroa_0_1_ph;
      $carry_0_lcssa$1 = 0;
      $carry_0_lcssa$0 = 0;
    } else {
      $d_sroa_0_0_insert_insert99$0 = 0 | $b$0 & -1;
      $d_sroa_0_0_insert_insert99$1 = $d_sroa_1_4_extract_shift$0 | $b$1 & 0;
      $137$0 = _i64Add($d_sroa_0_0_insert_insert99$0 | 0, $d_sroa_0_0_insert_insert99$1 | 0, -1, -1) | 0;
      $137$1 = {{{ makeGetTempRet0() }}};
      $q_sroa_1_1198 = $q_sroa_1_1_ph;
      $q_sroa_0_1199 = $q_sroa_0_1_ph;
      $r_sroa_1_1200 = $r_sroa_1_1_ph;
      $r_sroa_0_1201 = $r_sroa_0_1_ph;
      $sr_1202 = $sr_1_ph;
      $carry_0203 = 0;
      while (1) {
        $147 = $q_sroa_0_1199 >>> 31 | $q_sroa_1_1198 << 1;
        $149 = $carry_0203 | $q_sroa_0_1199 << 1;
        $r_sroa_0_0_insert_insert42$0 = 0 | ($r_sroa_0_1201 << 1 | $q_sroa_1_1198 >>> 31);
        $r_sroa_0_0_insert_insert42$1 = $r_sroa_0_1201 >>> 31 | $r_sroa_1_1200 << 1 | 0;
        _i64Subtract($137$0 | 0, $137$1 | 0, $r_sroa_0_0_insert_insert42$0 | 0, $r_sroa_0_0_insert_insert42$1 | 0) | 0;
        $150$1 = {{{ makeGetTempRet0() }}};
        $151$0 = $150$1 >> 31 | (($150$1 | 0) < 0 ? -1 : 0) << 1;
        $152 = $151$0 & 1;
        $154$0 = _i64Subtract($r_sroa_0_0_insert_insert42$0 | 0, $r_sroa_0_0_insert_insert42$1 | 0, $151$0 & $d_sroa_0_0_insert_insert99$0 | 0, ((($150$1 | 0) < 0 ? -1 : 0) >> 31 | (($150$1 | 0) < 0 ? -1 : 0) << 1) & $d_sroa_0_0_insert_insert99$1 | 0) | 0;
        $r_sroa_0_0_extract_trunc = $154$0;
        $r_sroa_1_4_extract_trunc = {{{ makeGetTempRet0() }}};
        $155 = $sr_1202 - 1 | 0;
        if (($155 | 0) == 0) {
          break;
        } else {
          $q_sroa_1_1198 = $147;
          $q_sroa_0_1199 = $149;
          $r_sroa_1_1200 = $r_sroa_1_4_extract_trunc;
          $r_sroa_0_1201 = $r_sroa_0_0_extract_trunc;
          $sr_1202 = $155;
          $carry_0203 = $152;
        }
      }
      $q_sroa_1_1_lcssa = $147;
      $q_sroa_0_1_lcssa = $149;
      $r_sroa_1_1_lcssa = $r_sroa_1_4_extract_trunc;
      $r_sroa_0_1_lcssa = $r_sroa_0_0_extract_trunc;
      $carry_0_lcssa$1 = 0;
      $carry_0_lcssa$0 = $152;
    }
    $q_sroa_0_0_insert_ext75$0 = $q_sroa_0_1_lcssa;
    $q_sroa_0_0_insert_ext75$1 = 0;
    $q_sroa_0_0_insert_insert77$1 = $q_sroa_1_1_lcssa | $q_sroa_0_0_insert_ext75$1;
    if (($rem | 0) != 0) {
      HEAP32[$rem >> 2] = 0 | $r_sroa_0_1_lcssa;
      HEAP32[$rem + 4 >> 2] = $r_sroa_1_1_lcssa | 0;
    }
    $_0$1 = (0 | $q_sroa_0_0_insert_ext75$0) >>> 31 | $q_sroa_0_0_insert_insert77$1 << 1 | ($q_sroa_0_0_insert_ext75$1 << 1 | $q_sroa_0_0_insert_ext75$0 >>> 31) & 0 | $carry_0_lcssa$1;
    $_0$0 = ($q_sroa_0_0_insert_ext75$0 << 1 | 0 >>> 31) & -2 | $carry_0_lcssa$0;
    return ({{{ makeSetTempRet0('$_0$1') }}}, $_0$0) | 0;
  },
  // =======================================================================

};

function autoAddDeps(object, name) {
  name = [name];
  for (var item in object) {
    if (item.substr(-6) != '__deps') {
      if (!object[item + '__deps']) {
        object[item + '__deps'] = name;
      } else {
        object[item + '__deps'].push(name[0]); // add to existing list
      }
    }
  }
}

