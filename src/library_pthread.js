var LibraryPThread = {
  $PThread: {
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
          } else if (e.data.cmd == 'cancel') {
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

  pthread_create: function(thread, attr, start_routine, arg) {
    if (!HEAPU8.buffer instanceof SharedArrayBuffer) {
      Module['printErr']('Current environment does not support SharedArrayBuffer, pthreads are not available!');
      return 1;
    }
    if (!thread) {
      Module['printErr']('pthread_create called with a null thread pointer!');
      return 1;
    }
    var worker = PThread.getNewWorker();
    if (worker.pthread !== undefined) throw 'Internal error!';
    PThread.runningWorkers.push(worker); // TODO: The list of threads is local to the parent thread, atm only the parent can access the threads it spawned!
    var threadId = PThread.pthreadIdCounter++;
    {{{ makeSetValue('thread', 0, 'threadId', 'i32') }}};

    var stackSize = 0;
    var stackBase = 0;
    if (attr) {
      stackSize = {{{ makeGetValue('attr', 0, 'i32') }}};
      stackBase = {{{ makeGetValue('attr', 8, 'i32') }}};
    }
    stackSize += 81920 /*DEFAULT_STACK_SIZE*/;
    var allocatedOwnStack = !stackBase;
    if (allocatedOwnStack) { 
      stackBase = _malloc(stackSize); // Allocate a stack if the user doesn't want to place the stack in a custom memory area.
    }
    // Allocate memory for thread-local storage and initialize it to zero.
    var tlsMemory = _malloc({{{ cDefine('PTHREAD_KEYS_MAX') }}} * 4);
    for(var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i)
      {{{ makeSetValue('tlsMemory', 'i*4', 0, 'i32') }}};

    var pthread = PThread.pthreads[threadId] = { // Create a pthread info object to represent this thread.
      worker: worker,
      thread: threadId,
      stackBase: stackBase,
      stackSize: stackSize,
      allocatedOwnStack: allocatedOwnStack,
      threadBlock: _malloc({{{ C_STRUCTS.pthread.__size__ }}}) // Info area for this thread in Emscripten HEAP (shared)
    };
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 0); // threadStatus <- 0, meaning not yet exited.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, 0); // threadExitCode <- 0.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.tsd }}} ) >> 2, tlsMemory); // Init thread-local-storage memory array.
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.tsd_used }}} ) >> 2, 0); // Mark initial status to unused.


    worker.pthread = pthread;

    // Ask the worker to start executing its pthread entry point function.
    worker.postMessage({
      cmd: 'run',
      start_routine: start_routine,
      arg: arg,
      threadBlock: pthread.threadBlock,
      selfThreadId: threadId,
      stackBase: stackBase,
      stackSize: stackSize
    });
    return 0;
  },

  pthread_join: function(thread, status) {
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return 1;
    }
    var worker = pthread.worker;
    for(;;) {
      assert(pthread.threadBlock);
      var threadStatus = Atomics.load(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
      if (threadStatus == 1) { // Exited?
        var threadExitCode = Atomics.load(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2);
        if (status) {
          {{{ makeSetValue('status', 0, 'threadExitCode', 'i32') }}};
        }
        PThread.freeThreadData(pthread);
        worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
        PThread.unusedWorkerPool.push(worker);
        PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker.pthread), 1); // Not a running Worker anymore.
        return 0;
      } else if (threadStatus != 0) {
        // Thread was canceled. It is an error to first pthread_cancel() a thread and then later pthread_join() on it.
        return 1; 
      }
    }
  },

  pthread_kill: function(thread, signal) {
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return 1;
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
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return 1;
    }
    assert(pthread.threadBlock);
    Atomics.store(HEAPU32, (pthread.threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 2); // Signal the thread that it needs to cancel itself.
    pthread.worker.postMessage({ cmd: 'cancel' });
    return 0;
  },

  pthread_testcancel: function() {
    if (!ENVIRONMENT_IS_PTHREAD) return;
    assert(threadBlock);
    var canceled = Atomics.load(HEAPU32, (threadBlock + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    if (canceled == 2) throw 'Canceled!';
  },

  pthread_setcancelstate: function(state, oldstate) {
    // TODO
    return 0;
  },

  pthread_setcanceltype: function(type, oldtype) {
    // TODO
    return 0;
  },

  pthread_detach: function(thread) {
    var pthread = PThread.pthreads[thread];
    if (!pthread) {
      Module['printErr']('PThread ' + thread + ' does not exist!');
      return 1;
    }
    // No-op.    
    return 0;
  },

  pthread_exit: function(status) {
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

  pthread_once: function(once_control, init_routine) {
  },

  pthread_getschedparam: function(thread, policy, schedparam) {
    // TODO
    return 0;
  },

  pthread_setschedparam: function(thread, policy, schedparam) {
    // TODO
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
    // TODO
    return 0;
  },

  pthread_setschedprio: function(thread, prio) {
    // no-op: Can this be implemented?
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

  // Compares the given memory address to 'oldVal', and if equal, replaces the contents with 'newVal'.
  // Returns the value that was stored in the memory location before this operation took place.
  emscripten_atomic_cas_u8: function(addr, oldVal, newVal) { return Atomics.compareExchange(HEAPU8, addr, oldVal, newVal); },
  emscripten_atomic_cas_u16: function(addr, oldVal, newVal) { return Atomics.compareExchange(HEAPU16, addr >> 1, oldVal, newVal); },
  emscripten_atomic_cas_u32: function(addr, oldVal, newVal) { return Atomics.compareExchange(HEAPU32, addr >> 2, oldVal, newVal); },
  emscripten_atomic_cas_f32: function(addr, oldVal, newVal) { return Atomics.compareExchange(HEAPF32, addr >> 2, oldVal, newVal); },
  emscripten_atomic_cas_f64: function(addr, oldVal, newVal) { return Atomics.compareExchange(HEAPF64, addr >> 3, oldVal, newVal); },

  emscripten_atomic_load_u8: function(addr) { return Atomics.load(HEAP8, addr); },
  emscripten_atomic_load_u16: function(addr) { return Atomics.load(HEAP16, addr >> 1); },
  emscripten_atomic_load_u32: function(addr) { return Atomics.load(HEAP32, addr >> 2); },
  emscripten_atomic_load_f32: function(addr) { return Atomics.load(HEAPF32, addr >> 2); },
  emscripten_atomic_load_f64: function(addr) { return Atomics.load(HEAPF64, addr >> 3); },

  // Returns the value stored (coerced to the target type).
  emscripten_atomic_store_u8: function(addr, val) { return Atomics.store(HEAU8, addr, val); },
  emscripten_atomic_store_u16: function(addr, val) { return Atomics.store(HEAP16, addr >> 1, val); },
  emscripten_atomic_store_u32: function(addr, val) { return Atomics.store(HEAP32, addr >> 2, val); },
  emscripten_atomic_store_f32: function(addr, val) { return Atomics.store(HEAPF32, addr >> 2, val); },
  emscripten_atomic_store_f64: function(addr, val) { return Atomics.store(HEAPF64, addr >> 3, val); },

  // void return
  emscripten_atomic_fence: function() { Atomics.fence(); },

  // add, sub, and, or and xor return old value in the memory location.
  emscripten_atomic_add_u8: function(addr, val) { return Atomics.add(HEAP8, addr, val); },
  emscripten_atomic_add_u16: function(addr, val) { return Atomics.add(HEAP16, addr >> 1, val); },
  emscripten_atomic_add_u32: function(addr, val) { return Atomics.add(HEAP32, addr >> 2, val); },
  emscripten_atomic_add_f32: function(addr, val) { return Atomics.add(HEAPF32, addr >> 2, val); },
  emscripten_atomic_add_f64: function(addr, val) { return Atomics.add(HEAPF64, addr >> 3, val); },

  emscripten_atomic_sub_u8: function(addr, val) { return Atomics.sub(HEAP8, addr, val); },
  emscripten_atomic_sub_u16: function(addr, val) { return Atomics.sub(HEAP16, addr >> 1, val); },
  emscripten_atomic_sub_u32: function(addr, val) { return Atomics.sub(HEAP32, addr >> 2, val); },
  emscripten_atomic_sub_f32: function(addr, val) { return Atomics.sub(HEAPF32, addr >> 2, val); },
  emscripten_atomic_sub_f64: function(addr, val) { return Atomics.sub(HEAPF64, addr >> 3, val); },

  emscripten_atomic_and_u8: function(addr, val) { return Atomics.and(HEAP8, addr, val); },
  emscripten_atomic_and_u16: function(addr, val) { return Atomics.and(HEAP16, addr >> 1, val); },
  emscripten_atomic_and_u32: function(addr, val) { return Atomics.and(HEAP32, addr >> 2, val); },
  emscripten_atomic_and_f32: function(addr, val) { return Atomics.and(HEAPF32, addr >> 2, val); },
  emscripten_atomic_and_f64: function(addr, val) { return Atomics.and(HEAPF64, addr >> 3, val); },

  emscripten_atomic_or_u8: function(addr, val) { return Atomics.or(HEAP8, addr, val); },
  emscripten_atomic_or_u16: function(addr, val) { return Atomics.or(HEAP16, addr >> 1, val); },
  emscripten_atomic_or_u32: function(addr, val) { return Atomics.or(HEAP32, addr >> 2, val); },
  emscripten_atomic_or_f32: function(addr, val) { return Atomics.or(HEAPF32, addr >> 2, val); },
  emscripten_atomic_or_f64: function(addr, val) { return Atomics.or(HEAPF64, addr >> 3, val); },

  emscripten_atomic_xor_u8: function(addr, val) { return Atomics.xor(HEAP8, addr, val); },
  emscripten_atomic_xor_u16: function(addr, val) { return Atomics.xor(HEAP16, addr >> 1, val); },
  emscripten_atomic_xor_u32: function(addr, val) { return Atomics.xor(HEAP32, addr >> 2, val); },
  emscripten_atomic_xor_f32: function(addr, val) { return Atomics.xor(HEAPF32, addr >> 2, val); },
  emscripten_atomic_xor_f64: function(addr, val) { return Atomics.xor(HEAPF64, addr >> 3, val); },

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
