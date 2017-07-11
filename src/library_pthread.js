var LibraryPThread = {
  $PThread__postset: 'if (!ENVIRONMENT_IS_PTHREAD) PThread.initMainThreadBlock();',
  $PThread__deps: ['$PROCINFO', '_register_pthread_ptr', 'emscripten_main_thread_process_queued_calls'],
  $PThread: {
    MAIN_THREAD_ID: 1, // A special constant that identifies the main JS thread ID.
    mainThreadInfo: {
      schedPolicy: 0/*SCHED_OTHER*/,
      schedPrio: 0
    },
    // Since creating a new Web Worker is so heavy (it must reload the whole compiled script page!), maintain a pool of such
    // workers that have already parsed and loaded the scripts.
    unusedWorkerPool: [],
    // The currently executing pthreads.
    runningWorkers: [],
    // Points to a pthread_t structure in the Emscripten main heap, allocated on demand if/when first needed.
    // mainThreadBlock: undefined,
    initMainThreadBlock: function() {
      if (ENVIRONMENT_IS_PTHREAD) return undefined;
      PThread.mainThreadBlock = allocate({{{ C_STRUCTS.pthread.__size__ }}}, "i32*", ALLOC_STATIC);

      for (var i = 0; i < {{{ C_STRUCTS.pthread.__size__ }}}/4; ++i) HEAPU32[PThread.mainThreadBlock/4+i] = 0;

      // The pthread struct has a field that points to itself - this is used as a magic ID to detect whether the pthread_t
      // structure is 'alive'.
      {{{ makeSetValue('PThread.mainThreadBlock', C_STRUCTS.pthread.self, 'PThread.mainThreadBlock', 'i32') }}};

      // pthread struct robust_list head should point to itself.
      var headPtr = PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.robust_list }}};
      {{{ makeSetValue('headPtr', 0, 'headPtr', 'i32') }}};

      // Allocate memory for thread-local storage.
      var tlsMemory = allocate({{{ cDefine('PTHREAD_KEYS_MAX') }}} * 4, "i32*", ALLOC_STATIC);
      for (var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i) HEAPU32[tlsMemory/4+i] = 0;
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.tsd }}} ) >> 2, tlsMemory); // Init thread-local-storage memory array.
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.tid }}} ) >> 2, PThread.mainThreadBlock); // Main thread ID.
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.pid }}} ) >> 2, PROCINFO.pid); // Process ID.

#if PTHREADS_PROFILING
      PThread.createProfilerBlock(PThread.mainThreadBlock);
      PThread.setThreadName(PThread.mainThreadBlock, "main thread");
      PThread.setThreadStatus(PThread.mainThreadBlock, {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}});
#endif
    },
    // Maps pthread_t to pthread info objects
    pthreads: {},
    pthreadIdCounter: 2, // 0: invalid thread, 1: main JS UI thread, 2+: IDs for pthreads

    exitHandlers: null, // An array of C functions to run when this thread exits.

#if PTHREADS_PROFILING
    createProfilerBlock: function(pthreadPtr) {
      var profilerBlock = (pthreadPtr == PThread.mainThreadBlock) ? allocate({{{ C_STRUCTS.thread_profiler_block.__size__ }}}, "i32*", ALLOC_STATIC) : _malloc({{{ C_STRUCTS.thread_profiler_block.__size__ }}});
      Atomics.store(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2, profilerBlock);

      // Zero fill contents at startup.
      for (var i = 0; i < {{{ C_STRUCTS.thread_profiler_block.__size__ }}}; i += 4) Atomics.store(HEAPU32, (profilerBlock + i) >> 2, 0);
      Atomics.store(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.thread_profiler_block.currentStatusStartTime }}} ) >> 2, performance.now());
    },

    // Sets the current thread status, but only if it was in the given expected state before. This is used
    // to allow high-level control flow "override" the thread status before low-level (futex wait) operations set it.
    setThreadStatusConditional: function(pthreadPtr, expectedStatus, newStatus) {
      var profilerBlock = Atomics.load(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
      if (!profilerBlock) return;

      var prevStatus = Atomics.load(HEAPU32, (profilerBlock + {{{ C_STRUCTS.thread_profiler_block.threadStatus }}} ) >> 2);

      if (prevStatus != newStatus && (prevStatus == expectedStatus || expectedStatus == -1)) {
        var now = performance.now();
        var startState = HEAPF64[(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.currentStatusStartTime }}} ) >> 3];
        var duration = now - startState;

        HEAPF64[((profilerBlock + {{{ C_STRUCTS.thread_profiler_block.timeSpentInStatus }}} ) >> 3) + prevStatus] += duration;
        Atomics.store(HEAPU32, (profilerBlock + {{{ C_STRUCTS.thread_profiler_block.threadStatus }}} ) >> 2, newStatus);
        HEAPF64[(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.currentStatusStartTime }}} ) >> 3] = now;
      }
    },

    // Unconditionally sets the thread status.
    setThreadStatus: function(pthreadPtr, newStatus) {
      PThread.setThreadStatusConditional(pthreadPtr, -1, newStatus);
    },

    setThreadName: function(pthreadPtr, name) {
      var profilerBlock = Atomics.load(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
      if (!profilerBlock) return;
      stringToUTF8(name, profilerBlock + {{{ C_STRUCTS.thread_profiler_block.name }}}, 32);
    },

    getThreadName: function(pthreadPtr) {
      var profilerBlock = Atomics.load(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
      if (!profilerBlock) return "";
      return UTF8ToString(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.name }}});
    },

    threadStatusToString: function(threadStatus) {
      switch(threadStatus) {
        case 0: return "not yet started";
        case 1: return "running";
        case 2: return "sleeping";
        case 3: return "waiting for a futex";
        case 4: return "waiting for a mutex";
        case 5: return "waiting for a proxied operation";
        case 6: return "finished execution";
        default: return "unknown (corrupt?!)";
      }
    },

    threadStatusAsString: function(pthreadPtr) {
      var profilerBlock = Atomics.load(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
      var status = (profilerBlock == 0) ? 0 : Atomics.load(HEAPU32, (profilerBlock + {{{ C_STRUCTS.thread_profiler_block.threadStatus }}} ) >> 2);
      return PThread.threadStatusToString(status);
    },
#else
    setThreadStatus: function() {},
#endif

    runExitHandlers: function() {
      if (PThread.exitHandlers !== null) {
        while (PThread.exitHandlers.length > 0) {
          PThread.exitHandlers.pop()();
        }
        PThread.exitHandlers = null;
      }

      // Call into the musl function that runs destructors of all thread-specific data.
      if (ENVIRONMENT_IS_PTHREAD && threadInfoStruct) ___pthread_tsd_run_dtors();
    },

    // Called when we are performing a pthread_exit(), either explicitly called by programmer,
    // or implicitly when leaving the thread main function.
    threadExit: function(exitCode) {
      var tb = _pthread_self();
      if (tb) { // If we haven't yet exited?
#if PTHREADS_PROFILING
        var profilerBlock = Atomics.load(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
        Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2, 0);
        _free(profilerBlock);
#endif
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, exitCode);
        // When we publish this, the main thread is free to deallocate the thread object and we are done.
        // Therefore set threadInfoStruct = 0; above to 'release' the object in this worker thread.
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 1);

        // Disable all cancellation so that executing the cleanup handlers won't trigger another JS
        // canceled exception to be thrown.
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.canceldisable }}} ) >> 2, 1/*PTHREAD_CANCEL_DISABLE*/);
        Atomics.store(HEAPU32, (tb + {{{ C_STRUCTS.pthread.cancelasync }}} ) >> 2, 0/*PTHREAD_CANCEL_DEFERRED*/);
        PThread.runExitHandlers();

        _emscripten_futex_wake(tb + {{{ C_STRUCTS.pthread.threadStatus }}}, {{{ cDefine('INT_MAX') }}});
        __register_pthread_ptr(0, 0, 0); // Unregister the thread block also inside the asm.js scope.
        threadInfoStruct = 0;
        if (ENVIRONMENT_IS_PTHREAD) {
          // This worker no longer owns any WebGL OffscreenCanvases, so transfer them back to parent thread.
          var transferList = [];

#if OFFSCREENCANVAS_SUPPORT
          var offscreenCanvases = {};
          if (typeof GL !== 'undefined') {
            offscreenCanvases = GL.offscreenCanvases;
            GL.offscreenCanvases = {};
          }
          for (var i in offscreenCanvases) {
            if (offscreenCanvases[i]) transferList.push(offscreenCanvases[i]);
          }
          if (transferList.length > 0) {
            postMessage({
                targetThread: parentThreadId,
                cmd: 'objectTransfer',
                offscreenCanvases: offscreenCanvases,
                moduleCanvasId: Module['canvas'].id, // moduleCanvasId specifies which canvas is denoted via the "#canvas" shorthand.
                transferList: transferList
              }, transferList);
          }
          // And clear the OffscreenCanvases from lingering around in this Worker as well.
          delete Module['canvas'];
#endif

          postMessage({ cmd: 'exit' });
        }
      }
    },

    threadCancel: function() {
      PThread.runExitHandlers();
      Atomics.store(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, -1/*PTHREAD_CANCELED*/);
      Atomics.store(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 1); // Mark the thread as no longer running.
      _emscripten_futex_wake(threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}}, {{{ cDefine('INT_MAX') }}}); // wake all threads
      threadInfoStruct = selfThreadId = 0; // Not hosting a pthread anymore in this worker, reset the info structures to null.
      __register_pthread_ptr(0, 0, 0); // Unregister the thread block also inside the asm.js scope.
      postMessage({ cmd: 'cancelDone' });
    },

    terminateAllThreads: function() {
      for (var t in PThread.pthreads) {
        var pthread = PThread.pthreads[t];
        if (pthread) {
          PThread.freeThreadData(pthread);
          if (pthread.worker) pthread.worker.terminate();
        }
      }
      PThread.pthreads = {};
      for (var t in PThread.unusedWorkerPool) {
        var pthread = PThread.unusedWorkerPool[t];
        if (pthread) {
          PThread.freeThreadData(pthread);
          if (pthread.worker) pthread.worker.terminate();
        }
      }
      PThread.unusedWorkerPool = [];
      for (var t in PThread.runningWorkers) {
        var pthread = PThread.runningWorkers[t];
        if (pthread) {
          PThread.freeThreadData(pthread);
          if (pthread.worker) pthread.worker.terminate();
        }
      }
      PThread.runningWorkers = [];
    },
    freeThreadData: function(pthread) {
      if (!pthread) return;
      if (pthread.threadInfoStruct) {
        var tlsMemory = {{{ makeGetValue('pthread.threadInfoStruct', C_STRUCTS.pthread.tsd, 'i32') }}};
        {{{ makeSetValue('pthread.threadInfoStruct', C_STRUCTS.pthread.tsd, 0, 'i32') }}};
        _free(pthread.tlsMemory);
        _free(pthread.threadInfoStruct);
      }
      pthread.threadInfoStruct = 0;
      if (pthread.allocatedOwnStack && pthread.stackBase) _free(pthread.stackBase);
      pthread.stackBase = 0;
      if (pthread.worker) pthread.worker.pthread = null;
    },

    receiveObjectTransfer: function(data) {
#if OFFSCREENCANVAS_SUPPORT
      if (typeof GL !== 'undefined') {
        for (var i in data.offscreenCanvases) {
          GL.offscreenCanvases[i] = data.offscreenCanvases[i];
          GL.offscreenCanvases[i].id = i; // https://bugzilla.mozilla.org/show_bug.cgi?id=1281909
        }
        if (!Module['canvas']) Module['canvas'] = GL.offscreenCanvases[data.moduleCanvasId];
      }
#endif
    },

    // Allocates the given amount of new web workers and stores them in the pool of unused workers.
    // onFinishedLoading: A callback function that will be called once all of the workers have been initialized and are
    //                    ready to host pthreads. Optional. This is used to mitigate bug https://bugzilla.mozilla.org/show_bug.cgi?id=1049079
    allocateUnusedWorkers: function(numWorkers, onFinishedLoading) {
      if (typeof SharedArrayBuffer === 'undefined') return; // No multithreading support, no-op.
      Module['print']('Preallocating ' + numWorkers + ' workers for a pthread spawn pool.');

      var numWorkersLoaded = 0;
      for (var i = 0; i < numWorkers; ++i) {
        var pthreadMainJs = 'pthread-main.js';
        // Allow HTML module to configure the location where the 'pthread-main.js' file will be loaded from,
        // either via Module.locateFile() function, or via Module.pthreadMainPrefixURL string. If neither
        // of these are passed, then the default URL 'pthread-main.js' relative to the main html file is loaded.
        if (typeof Module['locateFile'] === 'function') pthreadMainJs = Module['locateFile'](pthreadMainJs);
        else if (Module['pthreadMainPrefixURL']) pthreadMainJs = Module['pthreadMainPrefixURL'] + pthreadMainJs;
        var worker = new Worker(pthreadMainJs);

        worker.onmessage = function(e) {
          // If this message is intended to a recipient that is not the main thread, forward it to the target thread.
          if (e.data.targetThread && e.data.targetThread != _pthread_self()) {
            var thread = PThread.pthreads[e.data.targetThread];
            if (thread) {
              thread.worker.postMessage(e.data, e.data.transferList);
            } else {
              console.error('Internal error! Worker sent a message "' + e.data.cmd + '" to target pthread ' + e.data.targetThread + ', but that thread no longer exists!');
            }
            return;
          }

          if (e.data.cmd === 'processQueuedMainThreadWork') {
            // TODO: Must post message to main Emscripten thread in PROXY_TO_WORKER mode.
            _emscripten_main_thread_process_queued_calls();
          } else if (e.data.cmd === 'spawnThread') {
            __spawn_thread(e.data);
          } else if (e.data.cmd === 'cleanupThread') {
            __cleanup_thread(e.data.thread);
          } else if (e.data.cmd === 'killThread') {
            __kill_thread(e.data.thread);
          } else if (e.data.cmd === 'cancelThread') {
            __cancel_thread(e.data.thread);
          } else if (e.data.cmd === 'loaded') {
            ++numWorkersLoaded;
            if (numWorkersLoaded === numWorkers && onFinishedLoading) {
              onFinishedLoading();
            }
          } else if (e.data.cmd === 'print') {
            Module['print']('Thread ' + e.data.threadId + ': ' + e.data.text);
          } else if (e.data.cmd === 'printErr') {
            Module['printErr']('Thread ' + e.data.threadId + ': ' + e.data.text);
          } else if (e.data.cmd === 'alert') {
            alert('Thread ' + e.data.threadId + ': ' + e.data.text);
          } else if (e.data.cmd === 'exit') {
            // currently no-op
          } else if (e.data.cmd === 'cancelDone') {
            PThread.freeThreadData(worker.pthread);
            worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
            PThread.unusedWorkerPool.push(worker);
            // TODO: Free if detached.
            PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker.pthread), 1); // Not a running Worker anymore.
          } else if (e.data.cmd === 'objectTransfer') {
            PThread.receiveObjectTransfer(e.data);
          } else {
            Module['printErr']("worker sent an unknown command " + e.data.cmd);
          }
        };

        worker.onerror = function(e) {
          Module['printErr']('pthread sent an error! ' + e.filename + ':' + e.lineno + ': ' + e.message);
        };

        // Allocate tempDoublePtr for the worker. This is done here on the worker's behalf, since we may need to do this statically
        // if the runtime has not been loaded yet, etc. - so we just use getMemory, which is main-thread only.
        var tempDoublePtr = getMemory(8); // TODO: leaks. Cleanup after worker terminates.

        // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
        worker.postMessage({
            cmd: 'load',
            url: currentScriptUrl,
            buffer: HEAPU8.buffer,
            tempDoublePtr: tempDoublePtr,
            TOTAL_MEMORY: TOTAL_MEMORY,
            STATICTOP: STATICTOP,
            DYNAMIC_BASE: DYNAMIC_BASE,
            DYNAMICTOP_PTR: DYNAMICTOP_PTR,
            PthreadWorkerInit: PthreadWorkerInit
          });
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
      while(performance.now() < t) {
        ;
      }
    }
  },

  _kill_thread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! _kill_thread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in _kill_thread!';
    {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
    var pthread = PThread.pthreads[pthread_ptr];
    pthread.worker.terminate();
    PThread.freeThreadData(pthread);
    // The worker was completely nuked (not just the pthread execution it was hosting), so remove it from running workers
    // but don't put it back to the pool.
    PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(pthread.worker.pthread), 1); // Not a running Worker anymore.
    pthread.worker.pthread = undefined;
  },

  _cleanup_thread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! _cleanup_thread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in _cleanup_thread!';
    {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
    var pthread = PThread.pthreads[pthread_ptr];
    var worker = pthread.worker;
    PThread.freeThreadData(pthread);
    worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
    PThread.unusedWorkerPool.push(worker);
    PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker.pthread), 1); // Not a running Worker anymore.
  },

  _cancel_thread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! _cancel_thread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in _cancel_thread!';
    var pthread = PThread.pthreads[pthread_ptr];
    pthread.worker.postMessage({ cmd: 'cancel' });
  },

  _spawn_thread: function(threadParams) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! _spawn_thread() can only ever be called from main application thread!';

    var worker = PThread.getNewWorker();
    if (worker.pthread !== undefined) throw 'Internal error!';
    if (!threadParams.pthread_ptr) throw 'Internal error, no pthread ptr!';
    PThread.runningWorkers.push(worker);

    // Allocate memory for thread-local storage and initialize it to zero.
    var tlsMemory = _malloc({{{ cDefine('PTHREAD_KEYS_MAX') }}} * 4);
    for (var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i) {
      {{{ makeSetValue('tlsMemory', 'i*4', 0, 'i32') }}};
    }

    var pthread = PThread.pthreads[threadParams.pthread_ptr] = { // Create a pthread info object to represent this thread.
      worker: worker,
      stackBase: threadParams.stackBase,
      stackSize: threadParams.stackSize,
      allocatedOwnStack: threadParams.allocatedOwnStack,
      thread: threadParams.pthread_ptr,
      threadInfoStruct: threadParams.pthread_ptr // Info area for this thread in Emscripten HEAP (shared)
    };
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 0); // threadStatus <- 0, meaning not yet exited.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, 0); // threadExitCode <- 0.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2, 0); // profilerBlock <- 0.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, threadParams.detached);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.tsd }}} ) >> 2, tlsMemory); // Init thread-local-storage memory array.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.tsd_used }}} ) >> 2, 0); // Mark initial status to unused.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.tid }}} ) >> 2, pthread.threadInfoStruct); // Main thread ID.
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.pid }}} ) >> 2, PROCINFO.pid); // Process ID.

    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.attr }}}) >> 2, threadParams.stackSize);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.stack_size }}}) >> 2, threadParams.stackSize);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.stack }}}) >> 2, threadParams.stackBase);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.attr }}} + 8) >> 2, threadParams.stackBase);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.attr }}} + 12) >> 2, threadParams.detached);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.attr }}} + 20) >> 2, threadParams.schedPolicy);
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, threadParams.schedPrio);

    var global_libc = _emscripten_get_global_libc();
    var global_locale = global_libc + {{{ C_STRUCTS.libc.global_locale }}};
    Atomics.store(HEAPU32, (pthread.threadInfoStruct + {{{ C_STRUCTS.pthread.locale }}}) >> 2, global_locale);

#if PTHREADS_PROFILING
    PThread.createProfilerBlock(pthread.threadInfoStruct);
#endif

    worker.pthread = pthread;

    // Ask the worker to start executing its pthread entry point function.
    worker.postMessage({
      cmd: 'run',
      start_routine: threadParams.startRoutine,
      arg: threadParams.arg,
      threadInfoStruct: threadParams.pthread_ptr,
      selfThreadId: threadParams.pthread_ptr, // TODO: Remove this since thread ID is now the same as the thread address.
      parentThreadId: threadParams.parent_pthread_ptr,
      stackBase: threadParams.stackBase,
      stackSize: threadParams.stackSize,
#if OFFSCREENCANVAS_SUPPORT
      moduleCanvasId: threadParams.moduleCanvasId,
      offscreenCanvases: threadParams.offscreenCanvases,
#endif
    }, threadParams.transferList);
  },

#if USE_PTHREADS
  _num_logical_cores__deps: ['emscripten_force_num_logical_cores'],
  _num_logical_cores: '; if (ENVIRONMENT_IS_PTHREAD) __num_logical_cores = PthreadWorkerInit.__num_logical_cores; else { PthreadWorkerInit.__num_logical_cores = __num_logical_cores = allocate(1, "i32*", ALLOC_STATIC); HEAPU32[__num_logical_cores>>2] = navigator["hardwareConcurrency"] || ' + {{{ PTHREAD_HINT_NUM_CORES }}} + '; }',
#else
  _num_logical_cores: '{{{ makeStaticAlloc(1) }}}',
#endif

  emscripten_has_threading_support: function() {
#if USE_PTHREADS
    return typeof SharedArrayBuffer !== 'undefined';
#else
    return 0;
#endif
  },

  emscripten_num_logical_cores__deps: ['_num_logical_cores'],
  emscripten_num_logical_cores: function() {
    return {{{ makeGetValue('__num_logical_cores', 0, 'i32') }}};
  },

  emscripten_force_num_logical_cores: function(cores) {
    {{{ makeSetValue('__num_logical_cores', 0, 'cores', 'i32') }}};
  },

  pthread_create__deps: ['_spawn_thread', 'pthread_getschedparam', 'pthread_self'],
  pthread_create: function(pthread_ptr, attr, start_routine, arg) {
    if (typeof SharedArrayBuffer === 'undefined') {
      Module['printErr']('Current environment does not support SharedArrayBuffer, pthreads are not available!');
      return {{{ cDefine('EAGAIN') }}};
    }
    if (!pthread_ptr) {
      Module['printErr']('pthread_create called with a null thread pointer!');
      return {{{ cDefine('EINVAL') }}};
    }

    var transferList = []; // List of JS objects that will transfer ownership to the Worker hosting the thread

#if OFFSCREENCANVAS_SUPPORT
    // Deduce which WebGL canvases (HTMLCanvasElements or OffscreenCanvases) should be passed over to the
    // Worker that hosts the spawned pthread.
    var transferredCanvasNames = {{{ makeGetValue('attr', 36, 'i32') }}}; // Comma-delimited list of IDs "canvas1, canvas2, ..."
    if (transferredCanvasNames) transferredCanvasNames = Pointer_stringify(transferredCanvasNames).split(',');

    var offscreenCanvases = {}; // Dictionary of OffscreenCanvas objects we'll transfer to the created thread to own
    var moduleCanvasId = Module['canvas'] ? Module['canvas'].id : '';
    for (var i in transferredCanvasNames) {
      var name = transferredCanvasNames[i].trim();
      var offscreenCanvas;
      try {
        if (name == '#canvas') {
          if (!Module['canvas']) {
            Module['printErr']('pthread_create: could not find canvas with ID "' + name + '" to transfer to thread!');
            return {{{ cDefine('EINVAL') }}};
          }
          name = Module['canvas'].id;
        }
        if (GL.offscreenCanvases[name]) {
          offscreenCanvas = GL.offscreenCanvases[name];
          GL.offscreenCanvases[name] = null; // This thread no longer owns this canvas.
          if (Module['canvas'] instanceof OffscreenCanvas && name === Module['canvas'].id) Module['canvas'] = null;
        } else {
          var canvas = (Module['canvas'] && Module['canvas'].id === name) ? Module['canvas'] : document.getElementByID(name);
          if (!canvas) {
            Module['printErr']('pthread_create: could not find canvas with ID "' + name + '" to transfer to thread!');
            return {{{ cDefine('EINVAL') }}};
          }
          if (canvas.controlTransferredOffscreen) {
            Module['printErr']('pthread_create: cannot transfer canvas with ID "' + name + '" to thread, since the current thread does not have control over it!');
            return {{{ cDefine('EPERM') }}}; // Operation not permitted, some other thread is accessing the canvas.
          }
          if (!canvas.transferControlToOffscreen) {
            Module['printErr']('pthread_create: cannot transfer control of canvas "' + name + '" to pthread, because current browser does not support OffscreenCanvas!');
            return {{{ cDefine('ENOSYS') }}}; // Function not implemented, browser doesn't have support for this.
          }
          offscreenCanvas = canvas.transferControlToOffscreen();
          canvas.controlTransferredOffscreen = true;
          offscreenCanvas.id = canvas.id;
        }
        transferList.push(offscreenCanvas);
        offscreenCanvases[offscreenCanvas.id] = offscreenCanvas;
      } catch(e) {
        Module['printErr']('pthread_create: failed to transfer control of canvas "' + name + '" to OffscreenCanvas! Error: ' + e);
        return {{{ cDefine('EINVAL') }}}; // Hitting this might indicate an implementation bug or some other internal error
      }
    }
#endif

    // Synchronously proxy the thread creation to main thread if possible. If we need to transfer ownership of objects, then
    // proxy asynchronously via postMessage.
    if (ENVIRONMENT_IS_PTHREAD && transferList.length == 0) {
      return _emscripten_sync_run_in_main_thread_4({{{ cDefine('EM_PROXIED_PTHREAD_CREATE') }}}, pthread_ptr, attr, start_routine, arg);
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
    var allocatedOwnStack = stackBase == 0; // If allocatedOwnStack == true, then the pthread impl maintains the stack allocation.
    if (allocatedOwnStack) {
      stackBase = _malloc(stackSize); // Allocate a stack if the user doesn't want to place the stack in a custom memory area.
    } else {
      // Musl stores the stack base address assuming stack grows downwards, so adjust it to Emscripten convention that the
      // stack grows upwards instead.
      stackBase -= stackSize;
      assert(stackBase > 0);
    }

    // Allocate thread block (pthread_t structure).
    var threadInfoStruct = _malloc({{{ C_STRUCTS.pthread.__size__ }}});
    for (var i = 0; i < {{{ C_STRUCTS.pthread.__size__ }}} >> 2; ++i) HEAPU32[(threadInfoStruct>>2) + i] = 0; // zero-initialize thread structure.
    {{{ makeSetValue('pthread_ptr', 0, 'threadInfoStruct', 'i32') }}};

    // The pthread struct has a field that points to itself - this is used as a magic ID to detect whether the pthread_t
    // structure is 'alive'.
    {{{ makeSetValue('threadInfoStruct', C_STRUCTS.pthread.self, 'threadInfoStruct', 'i32') }}};

    // pthread struct robust_list head should point to itself.
    var headPtr = threadInfoStruct + {{{ C_STRUCTS.pthread.robust_list }}};
    {{{ makeSetValue('headPtr', 0, 'headPtr', 'i32') }}};

    var threadParams = {
      stackBase: stackBase,
      stackSize: stackSize,
      allocatedOwnStack: allocatedOwnStack,
      schedPolicy: schedPolicy,
      schedPrio: schedPrio,
      detached: detached,
      startRoutine: start_routine,
      pthread_ptr: threadInfoStruct,
      parent_pthread_ptr: _pthread_self(),
      arg: arg,
#if OFFSCREENCANVAS_SUPPORT
      moduleCanvasId: moduleCanvasId,
      offscreenCanvases: offscreenCanvases,
#endif
      transferList: transferList
    };

    if (ENVIRONMENT_IS_PTHREAD) {
      // The prepopulated pool of web workers that can host pthreads is stored in the main JS thread. Therefore if a
      // pthread is attempting to spawn a new thread, the thread creation must be deferred to the main JS thread.
      threadParams.cmd = 'spawnThread';
      postMessage(threadParams, transferList);
    } else {
      // We are the main thread, so we have the pthread warmup pool in this thread and can fire off JS thread creation
      // directly ourselves.
      __spawn_thread(threadParams);
    }

    return 0;
  },

  // TODO HACK! Remove this function, it is a JS side copy of the function pthread_testcancel() in library_pthread.c.
  // Just call pthread_testcancel() everywhere.
  _pthread_testcancel_js: function() {
    if (!ENVIRONMENT_IS_PTHREAD) return;
    if (!threadInfoStruct) return;
    var cancelDisabled = Atomics.load(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.canceldisable }}} ) >> 2);
    if (cancelDisabled) return;
    var canceled = Atomics.load(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    if (canceled == 2) throw 'Canceled!';
  },

  pthread_join__deps: ['_cleanup_thread', '_pthread_testcancel_js', 'emscripten_main_thread_process_queued_calls'],
  pthread_join: function(thread, status) {
    if (!thread) {
      Module['printErr']('pthread_join attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    if (ENVIRONMENT_IS_PTHREAD && selfThreadId == thread) {
      Module['printErr']('PThread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK;
    }
    else if (!ENVIRONMENT_IS_PTHREAD && PThread.mainThreadBlock == thread) {
      Module['printErr']('Main thread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_join attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }

    var detached = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2);
    if (detached) {
      Module['printErr']('Attempted to join thread ' + thread + ', which was already detached!');
      return ERRNO_CODES.EINVAL; // The thread is already detached, can no longer join it!
    }
    for (;;) {
      var threadStatus = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
      if (threadStatus == 1) { // Exited?
        var threadExitCode = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2);
        if (status) {{{ makeSetValue('status', 0, 'threadExitCode', 'i32') }}};
        Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 1); // Mark the thread as detached.

        if (!ENVIRONMENT_IS_PTHREAD) __cleanup_thread(thread);
        else postMessage({ cmd: 'cleanupThread', thread: thread});
        return 0;
      }
      // TODO HACK! Replace the _js variant with just _pthread_testcancel:
      //_pthread_testcancel();
      __pthread_testcancel_js();
      // In main runtime thread (the thread that initialized the Emscripten C runtime and launched main()), assist pthreads in performing operations
      // that they need to access the Emscripten main runtime for.
      if (!ENVIRONMENT_IS_PTHREAD) _emscripten_main_thread_process_queued_calls();
      _emscripten_futex_wait(thread + {{{ C_STRUCTS.pthread.threadStatus }}}, threadStatus, ENVIRONMENT_IS_PTHREAD ? 100 : 1);
    }
  },

  pthread_kill__deps: ['_kill_thread'],
  pthread_kill: function(thread, signal) {
    if (signal < 0 || signal >= 65/*_NSIG*/) return ERRNO_CODES.EINVAL;
    if (thread == PThread.MAIN_THREAD_ID) {
      if (signal == 0) return 0; // signal == 0 is a no-op.
      Module['printErr']('Main thread (id=' + thread + ') cannot be killed with pthread_kill!');
      return ERRNO_CODES.ESRCH;
    }
    if (!thread) {
      Module['printErr']('pthread_kill attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_kill attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    if (signal != 0) {
      if (!ENVIRONMENT_IS_PTHREAD) __kill_thread(thread);
      else postMessage({ cmd: 'killThread', thread: thread});
    }
    return 0;
  },

  pthread_cancel__deps: ['_cancel_thread'],
  pthread_cancel: function(thread) {
    if (thread == PThread.MAIN_THREAD_ID) {
      Module['printErr']('Main thread (id=' + thread + ') cannot be canceled!');
      return ERRNO_CODES.ESRCH;
    }
    if (!thread) {
      Module['printErr']('pthread_cancel attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_cancel attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    Atomics.compareExchange(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 0, 2); // Signal the thread that it needs to cancel itself.
    if (!ENVIRONMENT_IS_PTHREAD) __cancel_thread(thread);
    else postMessage({ cmd: 'cancelThread', thread: thread});
    return 0;
  },

  pthread_detach: function(thread) {
    if (!thread) {
      Module['printErr']('pthread_detach attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_detach attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    var threadStatus = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    // Follow musl convention: detached:0 means not detached, 1 means the thread was created as detached, and 2 means that the thread was detached via pthread_detach.
    var wasDetached = Atomics.compareExchange(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 0, 2);
    return wasDetached ? (threadStatus == 0/*running*/ ? ERRNO_CODES.EINVAL : ERRNO_CODES.ESRCH) : 0;
  },

  pthread_exit__deps: ['exit'],
  pthread_exit: function(status) {
    if (!ENVIRONMENT_IS_PTHREAD) _exit(status);
    else PThread.threadExit(status);
  },

  _pthread_ptr: 0,
  _pthread_is_main_runtime_thread: 0,
  _pthread_is_main_browser_thread: 0,

  _register_pthread_ptr__deps: ['_pthread_ptr', '_pthread_is_main_runtime_thread', '_pthread_is_main_browser_thread'],
  _register_pthread_ptr__asm: true,
  _register_pthread_ptr__sig: 'viii',
  _register_pthread_ptr: function(pthreadPtr, isMainBrowserThread, isMainRuntimeThread) {
    pthreadPtr = pthreadPtr|0;
    isMainBrowserThread = isMainBrowserThread|0;
    isMainRuntimeThread = isMainRuntimeThread|0;
    __pthread_ptr = pthreadPtr;
    __pthread_is_main_browser_thread = isMainBrowserThread;
    __pthread_is_main_runtime_thread = isMainRuntimeThread;
  },

  // Public pthread_self() function which returns a unique ID for the thread.
  pthread_self__deps: ['_pthread_ptr'],
  pthread_self__asm: true,
  pthread_self__sig: 'i',
  pthread_self: function() {
    return __pthread_ptr|0;
  },

  emscripten_is_main_runtime_thread__asm: true,
  emscripten_is_main_runtime_thread__sig: 'i',
  emscripten_is_main_runtime_thread__deps: ['_pthread_is_main_runtime_thread'],
  emscripten_is_main_runtime_thread: function() {
    return __pthread_is_main_runtime_thread|0; // Semantically the same as testing "!ENVIRONMENT_IS_PTHREAD" outside the asm.js scope
  },

  emscripten_is_main_browser_thread__asm: true,
  emscripten_is_main_browser_thread__sig: 'i',
  emscripten_is_main_browser_thread__deps: ['_pthread_is_main_browser_thread'],
  emscripten_is_main_browser_thread: function() {
    return __pthread_is_main_browser_thread|0; // Semantically the same as testing "!ENVIRONMENT_IS_WORKER" outside the asm.js scope
  },

  pthread_getschedparam: function(thread, policy, schedparam) {
    if (!policy && !schedparam) return ERRNO_CODES.EINVAL;

    if (!thread) {
      Module['printErr']('pthread_getschedparam called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_getschedparam attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }

    var schedPolicy = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 20 ) >> 2);
    var schedPrio = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 24 ) >> 2);

    if (policy) {{{ makeSetValue('policy', 0, 'schedPolicy', 'i32') }}};
    if (schedparam) {{{ makeSetValue('schedparam', 0, 'schedPrio', 'i32') }}};
    return 0;
  },

  pthread_setschedparam: function(thread, policy, schedparam) {
    if (!thread) {
      Module['printErr']('pthread_setschedparam called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_setschedparam attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }

    if (!schedparam) return ERRNO_CODES.EINVAL;

    var newSchedPrio = {{{ makeGetValue('schedparam', 0, 'i32') }}};
    if (newSchedPrio < 0) return ERRNO_CODES.EINVAL;
    if (policy == 1/*SCHED_FIFO*/ || policy == 2/*SCHED_RR*/) {
      if (newSchedPrio > 99) return ERRNO_CODES.EINVAL;
    } else {
      if (newSchedPrio > 1) return ERRNO_CODES.EINVAL;
    }

    Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 20) >> 2, policy);
    Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, newSchedPrio);
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

  pthread_mutexattr_getprioceiling: function(attr, prioceiling) {
    // Not supported either in Emscripten or musl, return a faked value.
    if (prioceiling) {{{ makeSetValue('prioceiling', 0, 99, 'i32') }}};
    return 0;
  },

  pthread_mutexattr_setprioceiling: function(attr, prioceiling) {
    // Not supported either in Emscripten or musl, return an error.
    return ERRNO_CODES.EPERM;
  },

  pthread_getcpuclockid: function(thread, clock_id) {
    return ERRNO_CODES.ENOENT; // pthread API recommends returning this error when "Per-thread CPU time clocks are not supported by the system."
  },

  pthread_setschedprio: function(thread, prio) {
    if (!thread) {
      Module['printErr']('pthread_setschedprio called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self != thread) {
      Module['printErr']('pthread_setschedprio attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    if (prio < 0) return ERRNO_CODES.EINVAL;

    var schedPolicy = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 20 ) >> 2);
    if (schedPolicy == 1/*SCHED_FIFO*/ || schedPolicy == 2/*SCHED_RR*/) {
      if (prio > 99) return ERRNO_CODES.EINVAL;
    } else {
      if (prio > 1) return ERRNO_CODES.EINVAL;
    }

    Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.attr }}} + 24) >> 2, prio);
    return 0;
  },

  pthread_cleanup_push: function(routine, arg) {
    if (PThread.exitHandlers === null) {
      PThread.exitHandlers = [];
      if (!ENVIRONMENT_IS_PTHREAD) {
        __ATEXIT__.push(function() { PThread.runExitHandlers(); });
      }
    }
    PThread.exitHandlers.push(function() { Module['dynCall_vi'](routine, arg) });
  },

  pthread_cleanup_pop: function(execute) {
    var routine = PThread.exitHandlers.pop();
    if (execute) routine();
  },

  // pthread_sigmask - examine and change mask of blocked signals
  pthread_sigmask: function(how, set, oldset) {
    Module['printErr']('pthread_sigmask() is not supported: this is a no-op.');
    return 0;
  },

  pthread_atfork: function(prepare, parent, child) {
    Module['printErr']('fork() is not supported: pthread_atfork is a no-op.');
    return 0;
  },

  // Stores the memory address that the main thread is waiting on, if any.
  _main_thread_futex_wait_address: '; if (ENVIRONMENT_IS_PTHREAD) __main_thread_futex_wait_address = PthreadWorkerInit.__main_thread_futex_wait_address; else PthreadWorkerInit.__main_thread_futex_wait_address = __main_thread_futex_wait_address = allocate(1, "i32*", ALLOC_STATIC)',

  // Returns 0 on success, or one of the values -ETIMEDOUT, -EWOULDBLOCK or -EINVAL on error.
  emscripten_futex_wait__deps: ['_main_thread_futex_wait_address', 'emscripten_main_thread_process_queued_calls'],
  emscripten_futex_wait: function(addr, val, timeout) {
    if (addr <= 0 || addr > HEAP8.length || addr&3 != 0) return -{{{ cDefine('EINVAL') }}};
//    dump('futex_wait addr:' + addr + ' by thread: ' + _pthread_self() + (ENVIRONMENT_IS_PTHREAD?'(pthread)':'') + '\n');
    if (ENVIRONMENT_IS_WORKER) {
#if PTHREADS_PROFILING
      PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}}, {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}});
#endif
      var ret = Atomics.wait(HEAP32, addr >> 2, val, timeout);
//    dump('futex_wait done by thread: ' + _pthread_self() + (ENVIRONMENT_IS_PTHREAD?'(pthread)':'') + '\n');
#if PTHREADS_PROFILING
      PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}}, {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}});
#endif
      if (ret === 'timed-out') return -{{{ cDefine('ETIMEDOUT') }}};
      if (ret === 'not-equal') return -{{{ cDefine('EWOULDBLOCK') }}};
      if (ret === 'ok') return 0;
      throw 'Atomics.wait returned an unexpected value ' + ret;
    } else {
      // Atomics.wait is not available in the main browser thread, so simulate it via busy spinning.
      var loadedVal = Atomics.load(HEAP32, addr >> 2);
      if (val != loadedVal) return -{{{ cDefine('EWOULDBLOCK') }}};

      var tNow = performance.now();
      var tEnd = tNow + timeout;

#if PTHREADS_PROFILING
      PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}}, {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}});
#endif

      // Register globally which address the main thread is simulating to be waiting on. When zero, main thread is not waiting on anything,
      // and on nonzero, the contents of address pointed by __main_thread_futex_wait_address tell which address the main thread is simulating its wait on.
      Atomics.store(HEAP32, __main_thread_futex_wait_address >> 2, addr);
      var ourWaitAddress = addr; // We may recursively re-enter this function while processing queued calls, in which case we'll do a spurious wakeup of the older wait operation.
      while (addr == ourWaitAddress) {
        tNow = performance.now();
        if (tNow > tEnd) {
#if PTHREADS_PROFILING
          PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}}, {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}});
#endif
          return -{{{ cDefine('ETIMEDOUT') }}};
        }
        _emscripten_main_thread_process_queued_calls(); // We are performing a blocking loop here, so must pump any pthreads if they want to perform operations that are proxied.
        addr = Atomics.load(HEAP32, __main_thread_futex_wait_address >> 2); // Look for a worker thread waking us up.
      }
#if PTHREADS_PROFILING
      PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}}, {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}});
#endif
      return 0;
    }
  },

  // Returns the number of threads (>= 0) woken up, or the value -EINVAL on error.
  // Pass count == INT_MAX to wake up all threads.
  emscripten_futex_wake__deps: ['_main_thread_futex_wait_address'],
  emscripten_futex_wake: function(addr, count) {
    if (addr <= 0 || addr > HEAP8.length || addr&3 != 0 || count < 0) return -{{{ cDefine('EINVAL') }}};
    if (count == 0) return 0;
//    dump('futex_wake addr:' + addr + ' by thread: ' + _pthread_self() + (ENVIRONMENT_IS_PTHREAD?'(pthread)':'') + '\n');

    // See if main thread is waiting on this address? If so, wake it up by resetting its wake location to zero.
    // Note that this is not a fair procedure, since we always wake main thread first before any workers, so
    // this scheme does not adhere to real queue-based waiting.
    var mainThreadWaitAddress = Atomics.load(HEAP32, __main_thread_futex_wait_address >> 2);
    var mainThreadWoken = 0;
    if (mainThreadWaitAddress == addr) {
      var loadedAddr = Atomics.compareExchange(HEAP32, __main_thread_futex_wait_address >> 2, mainThreadWaitAddress, 0);
      if (loadedAddr == mainThreadWaitAddress) {
        --count;
        mainThreadWoken = 1;
        if (count <= 0) return 1;
      }
    }

    // Wake any workers waiting on this address.
    var ret = Atomics.wake(HEAP32, addr >> 2, count);
    if (ret >= 0) return ret + mainThreadWoken;
    throw 'Atomics.wake returned an unexpected value ' + ret;
  },

  // Returns the number of threads (>= 0) woken up, or one of the values -EINVAL or -EAGAIN on error.
  emscripten_futex_wake_or_requeue__deps: ['_main_thread_futex_wait_address'],
  emscripten_futex_wake_or_requeue: function(addr, count, addr2, cmpValue) {
    if (addr <= 0 || addr2 <= 0 || addr >= HEAP8.length || addr2 >= HEAP8.length || count < 0
      || addr&3 != 0 || addr2&3 != 0) {
      return -{{{ cDefine('EINVAL') }}};
    }

    // See if main thread is waiting on this address? If so, wake it up by resetting its wake location to zero,
    // or move it to wait on addr2. Note that this is not a fair procedure, since we always wake main thread first before
    // any workers, so this scheme does not adhere to real queue-based waiting.
    var mainThreadWaitAddress = Atomics.load(HEAP32, __main_thread_futex_wait_address >> 2);
    var mainThreadWoken = 0;
    if (mainThreadWaitAddress == addr) {
      // Check cmpValue precondition before taking any action.
      var val1 = Atomics.load(HEAP32, addr >> 2);
      if (val1 != cmpValue) return -{{{ cDefine('EAGAIN') }}};

      // If we are actually waking any waiters, then new main thread wait location is reset, otherwise requeue it to wait on addr2.
      var newMainThreadWaitAddress = (count > 0) ? 0 : addr2;
      var loadedAddr = Atomics.compareExchange(HEAP32, __main_thread_futex_wait_address >> 2, mainThreadWaitAddress, newMainThreadWaitAddress);
      if (loadedAddr == mainThreadWaitAddress && count > 0) {
        --count; // Main thread was woken, so one less workers to wake up.
        mainThreadWoken = 1;
      }
    }

    // Wake any workers waiting on this address.
    var ret = Atomics.wakeOrRequeue(HEAP32, addr >> 2, count, addr2 >> 2, cmpValue);
    if (ret == Atomics.NOTEQUAL) return -{{{ cDefine('EAGAIN') }}};
    if (ret >= 0) return ret + mainThreadWoken;
    throw 'Atomics.wakeOrRequeue returned an unexpected value ' + ret;
  },

  __atomic_is_lock_free: function(size, ptr) {
    return size <= 4 && (size & (size-1)) == 0 && (ptr&(size-1)) == 0;
  },

  emscripten_conditional_set_current_thread_status_js: function(expectedStatus, newStatus) {
#if PTHREADS_PROFILING
    PThread.setThreadStatusConditional(_pthread_self(), expectedStatus, newStatus);
#endif
  },

  emscripten_set_current_thread_status_js: function(newStatus) {
#if PTHREADS_PROFILING
    PThread.setThreadStatus(_pthread_self(), newStatus);
#endif
  },

  emscripten_set_thread_name_js: function(threadId, name) {
#if PTHREADS_PROFILING
    PThread.setThreadName(threadId, UTF8ToString(name));
#endif
  },

  // The profiler setters are defined twice, here in asm.js so that they can be #ifdeffed out 
  // without having to pay the impact of a FFI transition for a no-op in non-profiling builds.
  emscripten_conditional_set_current_thread_status__asm: true,
  emscripten_conditional_set_current_thread_status__sig: 'vii',
  emscripten_conditional_set_current_thread_status__deps: ['emscripten_conditional_set_current_thread_status_js'],
  emscripten_conditional_set_current_thread_status: function(expectedStatus, newStatus) {
    expectedStatus = expectedStatus|0;
    newStatus = newStatus|0;
#if PTHREADS_PROFILING
    _emscripten_conditional_set_current_thread_status_js(expectedStatus|0, newStatus|0);
#endif
  },

  emscripten_set_current_thread_status__asm: true,
  emscripten_set_current_thread_status__sig: 'vi',
  emscripten_set_current_thread_status__deps: ['emscripten_set_current_thread_status_js'],
  emscripten_set_current_thread_status: function(newStatus) {
    newStatus = newStatus|0;
#if PTHREADS_PROFILING
    _emscripten_set_current_thread_status_js(newStatus|0);
#endif
  },

  emscripten_set_thread_name__asm: true,
  emscripten_set_thread_name__sig: 'vii',
  emscripten_set_thread_name__deps: ['emscripten_set_thread_name_js'],
  emscripten_set_thread_name: function(threadId, name) {
    threadId = threadId|0;
    name = name|0;
#if PTHREADS_PROFILING
    _emscripten_set_thread_name_js(threadId|0, name|0);
#endif
  }
};

autoAddDeps(LibraryPThread, '$PThread');
mergeInto(LibraryManager.library, LibraryPThread);
