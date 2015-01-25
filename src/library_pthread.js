var LibraryPThread = {
  $PThread: {
    MAIN_THREAD_ID: 1, // A special constant that identifies the main JS thread ID.
    mainThreadInfo: {
      schedPolicy: 0/*SCHED_OTHER*/,
      schedPrio: 0
    },
    thisThreadCancelState: 0, // 0: PTHREAD_CANCEL_ENABLE is the default for all threads. (1: PTHREAD_CANCEL_DISABLE is the other option)
    thisThreadCancelType: 0, // 0: PTHREAD_CANCEL_DEFERRED is the default for all threads. (1: PTHREAD_CANCEL_ASYNCHRONOUS is the other option)
    // Since creating a new Web Worker is so heavy (it must reload the whole compiled script page!), maintain a pool of such
    // workers that have already parsed and loaded the scripts.
    unusedWorkerPool: [],
    // The currently executing pthreads.
    runningWorkers: [],
    // Maps pthread_t to pthread info objects
    pthreads: {},
    pthreadIdCounter: 2, // 0: invalid thread, 1: main JS UI thread, 2+: IDs for pthreads

    exitHandlers: null, // An array of C functions to run when this thread exits.

    runExitHandlers: function() {
      if (PThread.exitHandlers !== null) {
        while (PThread.exitHandlers.length > 0) {
          PThread.exitHandlers.pop()();
        }
      }
      PThread.exitHandlers = null;

      // Call into the musl function that runs destructors of all thread-specific data.
      if (ENVIRONMENT_IS_PTHREAD && threadBlock) ___pthread_tsd_run_dtors();
    },

    // Called when we are performing a pthread_exit(), either explicitly called by programmer,
    // or implicitly when leaving the thread main function.
    threadExit: function(exitCode) {
      PThread.runExitHandlers();
      // No-op in the main thread. Note: Spec says we should join() all child threads, but since we don't have join,
      // we might at least cancel all threads.
      if (!ENVIRONMENT_IS_PTHREAD) return 0;

      if (threadBlock) { // If we haven't yet exited?
        var tb = threadBlock;
        threadBlock = 0;
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, exitCode);
        // When we publish this, the main thread is free to deallocate the thread object and we are done.
        // Therefore set threadBlock = 0; above to 'release' the object in this worker thread.
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 1);
        postMessage({ cmd: 'exit' });
      }
    },

    threadCancel: function() {
      PThread.runExitHandlers();
      threadBlock = selfThreadId = 0; // Not hosting a pthread anymore in this worker, reset the info structures to null.
      postMessage({ cmd: 'cancelDone' });
    },

    freeThreadData: function(pthread) {
      if (pthread.threadBlock) {
        var tlsMemory = {{{ makeGetValue('pthread.threadBlock', C_STRUCTS.pthread.tsd, 'i32') }}};
        {{{ makeSetValue('pthread.threadBlock', C_STRUCTS.pthread.tsd, 0, 'i32') }}};
        _free(pthread.tlsMemory);
        _free(pthread.threadBlock);
      }
      pthread.threadBlock = 0;
      if (pthread.allocatedOwnStack && pthread.stackBase) _free(pthread.stackBase);
      pthread.stackBase = 0;
      if (pthread.worker) pthread.worker.pthread = null;
    },

    // Allocates a the given amount of new web workers and stores them in the pool of unused workers.
    // onFinishedLoading: A callback function that will be called once all of the workers have been initialized and are
    //                    ready to host pthreads. Optional. This is used to mitigate bug https://bugzilla.mozilla.org/show_bug.cgi?id=1049079
    allocateUnusedWorkers: function(numWorkers, onFinishedLoading) {
      Module['print']('Preallocating ' + numWorkers + ' workers for a pthread spawn pool.');
      // Create a new one.
      // To spawn a web worker, we must give it a URL of the file to run. This means that for now, the new pthread we are spawning will
      // load the same Emscripten-compiled output .js file as the thread starts up.
      var url = window.location.pathname;
      url = url.substr(url.lastIndexOf('/')+1).replace('.html', '.js');

      var numWorkersLoaded = 0;
      for(var i = 0; i < numWorkers; ++i) {
        var worker = new Worker('pthread-main.js');

        worker.onmessage = function(e) {
          if (e.data.cmd == 'loaded') {
            ++numWorkersLoaded;
            if (numWorkersLoaded == numWorkers && onFinishedLoading) {
              onFinishedLoading();
            }
          } else if (e.data.cmd == 'print') {
            Module['print']('Thread ' + e.data.threadId + ': ' + e.data.text);
          } else if (e.data.cmd == 'printErr') {
            Module['printErr']('Thread ' + e.data.threadId + ': ' + e.data.text);
          } else if (e.data.cmd == 'exit') {
            // todo 
          } else if (e.data.cmd == 'cancelDone') {
              PThread.freeThreadData(worker.pthread);
              worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
              PThread.unusedWorkerPool.push(worker);
              // TODO: Free if detached.
              PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker.pthread), 1); // Not a running Worker anymore.
          } else {
            Module['printErr']("worker sent an unknown command " + e.data.cmd);
          }
        };

        worker.onerror = function(e) {
          Module['printErr']('pthread sent an error! ' + e.message);
        };

        // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
        worker.postMessage({ cmd: 'load', url: url, buffer: HEAPU8.buffer }, [HEAPU8.buffer]);
        PThread.unusedWorkerPool.push(worker);
      }
    },

    getNewWorker: function() {
      if (PThread.unusedWorkerPool.length == 0) PThread.allocateUnusedWorkers(1);
      if (PThread.unusedWorkerPool.length > 0) return PThread.unusedWorkerPool.pop();
      else return null;
    },

    busySpinWait: function(msecs) {
      var t = performance.now() + msecs;
      while(performance.now() < t)
        ;
    }
  },

  _spawn_thread: function(thread, threadParams) {
    if (ENVIRONMENT_IS_WORKER || ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! _spawn_thread() should only ever be called from main JS thread!';

    var worker = PThread.getNewWorker();
    if (worker.pthread !== undefined) throw 'Internal error!';
    PThread.runningWorkers.push(worker); // TODO: The list of threads is local to the parent thread, atm only the parent can access the threads it spawned!
    var threadId = PThread.pthreadIdCounter++;
    {{{ makeSetValue('thread', 0, 'threadId', 'i32') }}};

    // Allocate memory for thread-local storage and initialize it to zero.
    var tlsMemory = _malloc({{{ cDefine('PTHREAD_KEYS_MAX') }}} * 4);
    for(var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i) {
      {{{ makeSetValue('tlsMemory', 'i*4', 0, 'i32') }}};
    }

    var pthread = PThread.pthreads[threadId] = { // Create a pthread info object to represent this thread.
      worker: worker,
      thread: threadId,
      stackBase: threadParams.stackBase,
      stackSize: threadParams.stackSize,
      allocatedOwnStack: threadParams.allocatedOwnStack,
      threadBlock: _malloc({{{ C_STRUCTS.pthread.__size__ }}}) // Info area for this thread in Emscripten HEAP (shared)
    };
    for(var i = 0; i < {{{ C_STRUCTS.pthread.__size__ }}}; ++i) HEAPU8[pthread.threadBlock + i] = 0; // zero-initialize thread structure.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 0); // threadStatus <- 0, meaning not yet exited.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, 0); // threadExitCode <- 0.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, threadParams.detached);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.tsd }}} ) >> 2, tlsMemory); // Init thread-local-storage memory array.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.tsd_used }}} ) >> 2, 0); // Mark initial status to unused.

    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.attr }}}) >> 2, threadParams.stackSize);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.attr }}} + 8) >> 2, threadParams.stackBase);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.attr }}} + 12) >> 2, threadParams.detached);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.attr }}} + 20) >> 2, threadParams.schedPolicy);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, threadParams.schedPrio);

    worker.pthread = pthread;

    // Ask the worker to start executing its pthread entry point function.
    worker.postMessage({
      cmd: 'run',
      start_routine: threadParams.startRoutine,
      arg: threadParams.arg,
      threadBlock: pthread.threadBlock,
      selfThreadId: threadId,
      stackBase: threadParams.stackBase,
      stackSize: threadParams.stackSize,
      stdin: _stdin,
      stdout: _stdout,
      stderr: _stderr
    });
  },

  pthread_create__deps: ['_spawn_thread', 'pthread_getschedparam', 'pthread_self'],
  pthread_create: function(thread, attr, start_routine, arg) {
    if (!HEAPU8.buffer instanceof SharedArrayBuffer) {
      Module['printErr']('Current environment does not support SharedArrayBuffer, pthreads are not available!');
      return 1;
    }
    if (!thread) {
      Module['printErr']('pthread_create called with a null thread pointer!');
      return 1;
    }
    var stackSize = 0;
    var stackBase = 0;
    var detached = 0; // Default thread attr is PTHREAD_CREATE_JOINABLE, i.e. start as not detached.
    var schedPolicy = 0; /*SCHED_OTHER*/
    var schedPrio = 0;
    if (attr) {
      stackSize = {{{ makeGetValue('attr', 0, 'i32') }}};
      stackBase = {{{ makeGetValue('attr', 8, 'i32') }}};
      detached = {{{ makeGetValue('attr', 12/*_a_detach*/, 'i32') }}} != 0/*PTHREAD_CREATE_JOINABLE*/;
      var inheritSched = {{{ makeGetValue('attr', 16/*_a_sched*/, 'i32') }}} == 0/*PTHREAD_INHERIT_SCHED*/;
      if (inheritSched) {
        var prevSchedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        var prevSchedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
        _pthread_getschedparam(_pthread_self(), attr + 20, attr + 24);
        schedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        schedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
        {{{ makeSetValue('attr', 20/*_a_policy*/, 'prevSchedPolicy', 'i32') }}};
        {{{ makeSetValue('attr', 24/*_a_prio*/, 'prevSchedPrio', 'i32') }}};
      } else {
        schedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        schedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
      }
    }
    stackSize += 81920 /*DEFAULT_STACK_SIZE*/;
    var allocatedOwnStack = stackBase != 0;
    if (!allocatedOwnStack) {
      stackBase = _malloc(stackSize); // Allocate a stack if the user doesn't want to place the stack in a custom memory area.
    } else {
      // Musl stores the stack base address assuming stack grows downwards, so adjust it to Emscripten convention that the
      // stack grows upwards instead.
      stackBase -= stackSize;
      assert(stackBase > 0);
    }

    var threadParams = {
      stackBase: stackBase,
      stackSize: stackSize,
      allocatedOwnStack: allocatedOwnStack,
      schedPolicy: schedPolicy,
      schedPrio: schedPrio,
      detached: detached,
      startRoutine: start_routine,
      arg: arg,
    };
    __spawn_thread(thread, threadParams);

    return 0;
  },

  pthread_join: function(thread, status) {
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return ERRNO_CODES.ESRCH;
    }
    if (!ENVIRONMENT_IS_PTHREAD && thread == 1) {
      Module['printErr']('Main thread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK; // The main thread is attempting to join itself?
    }
    if (ENVIRONMENT_IS_PTHREAD && selfThreadId == THREAD) {
      Module['printErr']('PThread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK; // A non-main thread is attempting to join itself?
    }
    assert(pthread.threadBlock);
    var detached = Atomics.load(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.detached }}} ) >> 2);
    if (detached) {
      Module['printErr']('Attempted to join thread ' + thread + ', which was already detached!');
      return ERRNO_CODES.EINVAL; // The thread is already detached, can no longer join it!
    }
    var worker = pthread.worker;
    for(;;) {
      var threadStatus = Atomics.load(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
      if (threadStatus == 1) { // Exited?
        var threadExitCode = Atomics.load(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2);
        if (status) {{{ makeSetValue('status', 0, 'threadExitCode', 'i32') }}};
        Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 1); // Mark the thread as detached.
        PThread.freeThreadData(pthread);
        worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
        PThread.unusedWorkerPool.push(worker);
        PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker.pthread), 1); // Not a running Worker anymore.
        return 0;
      } else if (threadStatus != 0) {
        // Thread was canceled, so it should return with exit code PTHREAD_CANCELED.
        if (status) {{{ makeSetValue('status', 0, -1/*PTHREAD_CANCELED*/, 'i32') }}};
        return 0;
      }
    }
  },

  pthread_kill: function(thread, signal) {
    if (thread == PThread.MAIN_THREAD_ID) {
      Module['printErr']('Main thread (id=' + thread + ') cannot be killed with pthread_kill!');
      return ERRNO_CODES.ESRCH;
    }
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return ERRNO_CODES.ESRCH;
    }
    if (signal != 0) {
      pthread.worker.terminate();
      PThread.freeThreadData(pthread);
      // The worker was completely nuked (not just the pthread execution it was hosting), so remove it from running workers
      // but don't put it back to the pool.
      PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(pthread.worker.pthread), 1); // Not a running Worker anymore.
      pthread.worker.pthread = undefined;
    }
    return 0;
  },

  pthread_cancel: function(thread) {
    if (thread == PThread.MAIN_THREAD_ID) {
      Module['printErr']('Main thread (id=' + thread + ') cannot be canceled!');
      return ERRNO_CODES.ESRCH;
    }
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return ERRNO_CODES.ESRCH;
    }
    if (!pthread.threadBlock) return ERRNO_CODES.ESRCH; // Trying to cancel a thread that is no longer running.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 2); // Signal the thread that it needs to cancel itself.
    pthread.worker.postMessage({ cmd: 'cancel' });
    return 0;
  },

  pthread_testcancel: function() {
    if (!ENVIRONMENT_IS_PTHREAD) return;
    if (!threadBlock) return;
    if (PThread.thisThreadCancelState != 0/*PTHREAD_CANCEL_ENABLE*/) return;
    var canceled = Atomics.load(HEAPU32, (threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    if (canceled == 2) throw 'Canceled!';
  },

  pthread_setcancelstate: function(state, oldstate) {
    if (state != 0 && state != 1) return ERRNO_CODES.EINVAL;
    if (oldstate) {{{ makeSetValue('oldstate', 0, 'PThread.thisThreadCancelState', 'i32') }}};
    PThread.thisThreadCancelState = state;

    if (PThread.thisThreadCancelState == 0/*PTHREAD_CANCEL_ENABLE*/ && ENVIRONMENT_IS_PTHREAD) {
      // If we are re-enabling cancellation, immediately test whether this thread has been queued to be cancelled,
      // and if so, do it.
      var canceled = Atomics.load(HEAPU32, (threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
      if (canceled == 2) {
        throw 'Canceled!';
      }
    }
    return 0;
  },

  pthread_setcanceltype: function(type, oldtype) {
    if (type != 0 && type != 1) return ERRNO_CODES.EINVAL;
    if (oldtype) {{{ makeSetValue('oldtype', 0, 'PThread.thisThreadCancelType', 'i32') }}};
    PThread.thisThreadCancelType = type;
    return 0;
  },

  pthread_detach: function(thread) {
    var tb;
    if (ENVIRONMENT_IS_PTHREAD) {
      if (thread != selfThreadId) {
        Module['printErr']('TODO: Currently non-main threads can only detach themselves!');
        return ERRNO_CODES.ESRCH;
      }
      if (!threadBlock) {
        Module['printErr']('PThread ' + thread + ' does not exist!');
        return ERRNO_CODES.ESRCH;
      }
      tb = threadBlock;
    }
    else {
      var pthread = PThread.pthreads[thread];
      if (!pthread) {
        Module['printErr']('PThread ' + thread + ' does not exist!');
        return ERRNO_CODES.ESRCH;
      }
      tb = pthread.threadBlock;
    }
    var threadStatus = Atomics.load(HEAPU32, (tb + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    // Follow musl convention: detached:0 means not detached, 1 means the thread was created as detached, and 2 means that the thread was detached via pthread_detach.
    var wasDetached = Atomics.compareExchange(HEAPU32, (tb + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 0, 2);
    return wasDetached ? (threadStatus == 0/*running*/ ? ERRNO_CODES.EINVAL : ERRNO_CODES.ESRCH) : 0;
  },

  pthread_exit: function(status) {
    if (!ENVIRONMENT_IS_PTHREAD) {
      Module['printErr']('Warning: pthread_exit was called from the main thread that was not spawned via pthread_create(). (TODO)');
      return;
    }
    PThread.threadExit(status);
  },

  // Public pthread_self() function which returns a unique ID for the thread.
  pthread_self: function() {
    if (ENVIRONMENT_IS_PTHREAD) return selfThreadId;
    return 1; // Main JS thread
  },

  // pthread internal self() function which returns a pointer to the C control block for the thread.
  // pthread_self() and __pthread_self() are separate so that we can ensure that each thread gets its unique ID
  // using an incremented running counter, which helps in debugging.
  __pthread_self: function() {
    if (ENVIRONMENT_IS_PTHREAD) return threadBlock;
    return 0; // Main JS thread
  },

  pthread_getschedparam: function(thread, policy, schedparam) {
    if (!policy && !schedparam) return ERRNO_CODES.EINVAL;

    var tb;
    if (ENVIRONMENT_IS_PTHREAD) {
      if (thread != selfThreadId) {
        Module['printErr']('TODO: Currently non-main threads can only pthread_getschedparam themselves!');
        return ERRNO_CODES.ESRCH;
      }
      if (!threadBlock) {
        Module['printErr']('PThread ' + thread + ' does not exist!');
        return ERRNO_CODES.ESRCH;
      }
      tb = threadBlock;
    } else {
      if (thread == PThread.MAIN_THREAD_ID) {
        if (policy) {{{ makeSetValue('policy', 0, 'PThread.mainThreadInfo.schedPolicy', 'i32') }}};
        if (schedparam) {{{ makeSetValue('schedparam', 0, 'PThread.mainThreadInfo.schedPrio', 'i32') }}};
        return 0;
      }
      var threadInfo = PThread.pthreads[thread];
      if (!threadInfo) return ERRNO_CODES.ESRCH;
      tb = threadInfo.threadBlock;
    }

    var schedPolicy = Atomics.load(HEAPU32, (tb + {{{ C_STRUCTS.pthread.attr }}} + 20 ) >> 2);
    var schedPrio = Atomics.load(HEAPU32, (tb + {{{ C_STRUCTS.pthread.attr }}} + 24 ) >> 2);

    if (policy) {{{ makeSetValue('policy', 0, 'schedPolicy', 'i32') }}};
    if (schedparam) {{{ makeSetValue('schedparam', 0, 'schedPrio', 'i32') }}};
    return 0;
  },

  pthread_setschedparam: function(thread, policy, schedparam) {
    if (!schedparam) return ERRNO_CODES.EINVAL;

    var tb;
    if (ENVIRONMENT_IS_PTHREAD) {
      if (thread != selfThreadId) {
        Module['printErr']('TODO: Currently non-main threads can only pthread_setschedparam themselves!');
        return ERRNO_CODES.ESRCH;
      }
      if (!threadBlock) {
        Module['printErr']('PThread ' + thread + ' does not exist!');
        return ERRNO_CODES.ESRCH;
      }
      tb = threadBlock;
    } else {
      if (thread == PThread.MAIN_THREAD_ID) {
        PThread.mainThreadInfo.schedPolicy = policy;
        PThread.mainThreadInfo.schedPrio = {{{ makeGetValue('schedparam', 0, 'i32') }}};
        return 0;
      }
      var threadInfo = PThread.pthreads[thread];
      if (!threadInfo) return ERRNO_CODES.ESRCH;
      tb = threadInfo.threadBlock;
    }

    Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.attr }}} + 20) >> 2, policy);
    Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, {{{ makeGetValue('schedparam', 0, 'i32') }}});
    return 0;
  },

  // Marked as obsolescent in pthreads specification: http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getconcurrency.html
  pthread_getconcurrency: function() {
    return 0;
  },

  // Marked as obsolescent in pthreads specification.
  pthread_setconcurrency: function(new_level) {
    // no-op
    return 0;
  },

  pthread_getcpuclockid: function(thread, clock_id) {
    return ERRNO_CODES.ENOENT; // pthread API recommends returning this error when "Per-thread CPU time clocks are not supported by the system."
  },

  pthread_setschedprio: function(thread, prio) {
    var tb;
    if (ENVIRONMENT_IS_PTHREAD) {
      if (thread != selfThreadId) {
        Module['printErr']('TODO: Currently non-main threads can only pthread_setschedprio themselves!');
        return ERRNO_CODES.ESRCH;
      }
      if (!threadBlock) {
        Module['printErr']('PThread ' + thread + ' does not exist!');
        return ERRNO_CODES.ESRCH;
      }
      tb = threadBlock;
    } else {
      if (thread == PThread.MAIN_THREAD_ID) {
        PThread.mainThreadInfo.schedPrio = {{{ makeGetValue('prio', 0, 'i32') }}};
        return 0;
      }
      var threadInfo = PThread.pthreads[thread];
      if (!threadInfo) return ERRNO_CODES.ESRCH;
      tb = threadInfo.threadBlock;
    }

    Atomics.store(HEAPU32, (threadInfo.threadBlock + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, prio);
    return 0;
  },

  pthread_cleanup_push: function(routine, arg) {
    if (PThread.exitHandlers === null) {
      PThread.exitHandlers = [];
      if (!ENVIRONMENT_IS_PTHREAD) {
        __ATEXIT__.push({ func: function() { PThread.runExitHandlers(); } });
      }
    }
    PThread.exitHandlers.push(function() { Runtime.dynCall('vi', routine, [arg]) });
  },

  pthread_cleanup_pop: function(execute) {
    var routine = PThread.exitHandlers.pop();
    if (execute) routine();
  },

  // pthread_sigmask - examine and change mask of blocked signals
  pthread_sigmask: function(how, set, oldset) {
    // No-op.
    return 0;
  },

  pthread_atfork: function(prepare, parent, child) {
    Module['printErr']('fork() is not supported: pthread_atfork is a no-op.');
    return 0;
  },

  // Futex API
  emscripten_futex_wait: function(addr, val, timeout) {
    assert(addr);
    var ret = Atomics.futexWait(HEAP32, addr >> 2, val, timeout);
    if (ret === Atomics.OK) return 0;
    if (ret === Atomics.NOTEQUAL) return -1;
    if (ret === Atomics.TIMEDOUT) return -2;
    throw 'Atomics.futexWait returned an unexpected value ' + ret;
  },
/*
  emscripten_futex_wait_callback: function(addr, val, timeout, callback) {
    var callback = function(result) {
      var res = -1;
      if (result === Atomics.OK) res = 0;
      else if (result === Atomics.TIMEDOUT) res = 1;
      else throw 'Atomics.futexWaitCallback returned an unexpected value ' + result;
      asm.dynCall_vi(callback, res);
    };
    var ret = Atomics.futexWaitCallback(HEAP32, addr >> 2, val, timeout, callback);
    if (ret === Atomics.OK) return 0;
    if (ret === Atomics.TIMEDOUT) return 1;
    if (ret === Atomics.NOTEQUAL) return 2;
    throw 'Atomics.futexWaitCallback returned an unexpected value ' + ret;
  },
*/
  // Returns the number of threads woken up.
  emscripten_futex_wake: function(addr, count) {
    assert(addr);
    return Atomics.futexWake(HEAP32, addr >> 2, count);
  },
/*
  // Returns the number of threads woken up.
  emscripten_futex_requeue: function(addr1, count, addr2, guardval) {
    assert(addr1);
    assert(addr2);
    return Atomics.futexRequeue(HEAP32, addr1 >> 2, count, addr2 >> 2, guardval);
  }
*/
};

autoAddDeps(LibraryPThread, '$PThread');
mergeInto(LibraryManager.library, LibraryPThread);
