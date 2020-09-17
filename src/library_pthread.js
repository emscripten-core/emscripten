/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryPThread = {
  $PThread__postset: 'if (!ENVIRONMENT_IS_PTHREAD) PThread.initMainThreadBlock(); else PThread.initWorker();',
  $PThread__deps: ['$registerPthreadPtr',
                   '$ERRNO_CODES', 'emscripten_futex_wake', '$killThread',
                   '$cancelThread', '$cleanupThread',
                   '_main_thread_futex_wait_address'
#if USE_ASAN || USE_LSAN
                   , '$withBuiltinMalloc'
#endif
                   ],
  $PThread: {
    MAIN_THREAD_ID: 1, // A special constant that identifies the main JS thread ID.
    mainThreadInfo: {
      schedPolicy: 0/*SCHED_OTHER*/,
      schedPrio: 0
    },
    // Contains all Workers that are idle/unused and not currently hosting an executing pthread.
    // Unused Workers can either be pooled up before page startup, but also when a pthread quits, its hosting
    // Worker is not terminated, but is returned to this pool as an optimization so that starting the next thread is faster.
    unusedWorkers: [],
    // Contains all Workers that are currently hosting an active pthread.
    runningWorkers: [],
    // Points to a pthread_t structure in the Emscripten main heap, allocated on demand if/when first needed.
    // mainThreadBlock: undefined,
    initMainThreadBlock: function() {
#if ASSERTIONS
      assert(!ENVIRONMENT_IS_PTHREAD);
#endif

#if PTHREAD_POOL_SIZE
      var pthreadPoolSize = {{{ PTHREAD_POOL_SIZE }}};
      // Start loading up the Worker pool, if requested.
      for(var i = 0; i < pthreadPoolSize; ++i) {
        PThread.allocateUnusedWorker();
      }
#endif
    },
    initRuntime: function() {
#if USE_ASAN || USE_LSAN
      // When sanitizers are enabled, malloc is normally instrumented to call
      // sanitizer code that checks some things about pthreads. As we are just
      // setting up the main thread here, and are not ready for such calls,
      // call malloc directly.
      withBuiltinMalloc(function () {
#endif

      PThread.mainThreadBlock = _malloc({{{ C_STRUCTS.pthread.__size__ }}});

      for (var i = 0; i < {{{ C_STRUCTS.pthread.__size__ }}}/4; ++i) HEAPU32[PThread.mainThreadBlock/4+i] = 0;

      // The pthread struct has a field that points to itself - this is used as a magic ID to detect whether the pthread_t
      // structure is 'alive'.
      {{{ makeSetValue('PThread.mainThreadBlock', C_STRUCTS.pthread.self, 'PThread.mainThreadBlock', 'i32') }}};

      // pthread struct robust_list head should point to itself.
      var headPtr = PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.robust_list }}};
      {{{ makeSetValue('headPtr', 0, 'headPtr', 'i32') }}};

      // Allocate memory for thread-local storage.
      var tlsMemory = _malloc({{{ cDefine('PTHREAD_KEYS_MAX') * 4 }}});
      for (var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i) HEAPU32[tlsMemory/4+i] = 0;
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.tsd }}} ) >> 2, tlsMemory); // Init thread-local-storage memory array.
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.tid }}} ) >> 2, PThread.mainThreadBlock); // Main thread ID.
      Atomics.store(HEAPU32, (PThread.mainThreadBlock + {{{ C_STRUCTS.pthread.pid }}} ) >> 2, {{{ PROCINFO.pid }}}); // Process ID.

      __main_thread_futex_wait_address = _malloc(4);

#if PTHREADS_PROFILING
      PThread.createProfilerBlock(PThread.mainThreadBlock);
      PThread.setThreadName(PThread.mainThreadBlock, "Browser main thread");
      PThread.setThreadStatus(PThread.mainThreadBlock, {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}});
#endif

#if USE_ASAN || USE_LSAN
      });
#endif

      // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
      // Global constructors trying to access this value will read the wrong value, but that is UB anyway.
      registerPthreadPtr(PThread.mainThreadBlock, /*isMainBrowserThread=*/!ENVIRONMENT_IS_WORKER, /*isMainRuntimeThread=*/1);
      _emscripten_register_main_browser_thread_id(PThread.mainThreadBlock);
    },
    initWorker: function() {
#if USE_CLOSURE_COMPILER
      // worker.js is not compiled together with us, and must access certain
      // things.
      PThread['receiveObjectTransfer'] = PThread.receiveObjectTransfer;
      PThread['setThreadStatus'] = PThread.setThreadStatus;
      PThread['threadCancel'] = PThread.threadCancel;
      PThread['threadExit'] = PThread.threadExit;
#endif
    },
    // Maps pthread_t to pthread info objects
    pthreads: {},
    threadExitHandlers: [], // An array of C functions to run when this thread exits.

#if PTHREADS_PROFILING
    createProfilerBlock: function(pthreadPtr) {
      var profilerBlock = _malloc({{{ C_STRUCTS.thread_profiler_block.__size__ }}});
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
      while (PThread.threadExitHandlers.length > 0) {
        PThread.threadExitHandlers.pop()();
      }

      // Call into the musl function that runs destructors of all thread-specific data.
      if (ENVIRONMENT_IS_PTHREAD && threadInfoStruct) ___pthread_tsd_run_dtors();
    },

    // Called when we are performing a pthread_exit(), either explicitly called by programmer,
    // or implicitly when leaving the thread main function.
    threadExit: function(exitCode) {
      var tb = _pthread_self();
      if (tb) { // If we haven't yet exited?
#if ASSERTIONS
        err('Pthread 0x' + tb.toString(16) + ' exited.');
#endif
#if PTHREADS_PROFILING
        var profilerBlock = Atomics.load(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
        Atomics.store(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2, 0);
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
        registerPthreadPtr(0, 0, 0); // Unregister the thread block also inside the asm.js scope.
        threadInfoStruct = 0;
        if (ENVIRONMENT_IS_PTHREAD) {
          // Note: in theory we would like to return any offscreen canvases back to the main thread,
          // but if we ever fetched a rendering context for them that would not be valid, so we don't try.
          postMessage({ 'cmd': 'exit' });
        }
      }
    },

    threadCancel: function() {
      PThread.runExitHandlers();
      Atomics.store(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2, -1/*PTHREAD_CANCELED*/);
      Atomics.store(HEAPU32, (threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 1); // Mark the thread as no longer running.
      _emscripten_futex_wake(threadInfoStruct + {{{ C_STRUCTS.pthread.threadStatus }}}, {{{ cDefine('INT_MAX') }}}); // wake all threads
      threadInfoStruct = selfThreadId = 0; // Not hosting a pthread anymore in this worker, reset the info structures to null.
      registerPthreadPtr(0, 0, 0); // Unregister the thread block also inside the asm.js scope.
      postMessage({ 'cmd': 'cancelDone' });
    },

    terminateAllThreads: function() {
      for (var t in PThread.pthreads) {
        var pthread = PThread.pthreads[t];
        if (pthread && pthread.worker) {
          PThread.returnWorkerToPool(pthread.worker);
        }
      }
      PThread.pthreads = {};

      for (var i = 0; i < PThread.unusedWorkers.length; ++i) {
        var worker = PThread.unusedWorkers[i];
#if ASSERTIONS
        assert(!worker.pthread); // This Worker should not be hosting a pthread at this time.
#endif
        worker.terminate();
      }
      PThread.unusedWorkers = [];

      for (var i = 0; i < PThread.runningWorkers.length; ++i) {
        var worker = PThread.runningWorkers[i];
        var pthread = worker.pthread;
#if ASSERTIONS
        assert(pthread, 'This Worker should have a pthread it is executing');
#endif
        PThread.freeThreadData(pthread);
        worker.terminate();
      }
      PThread.runningWorkers = [];
    },
    freeThreadData: function(pthread) {
      if (!pthread) return;
      if (pthread.threadInfoStruct) {
        var tlsMemory = {{{ makeGetValue('pthread.threadInfoStruct', C_STRUCTS.pthread.tsd, 'i32') }}};
        {{{ makeSetValue('pthread.threadInfoStruct', C_STRUCTS.pthread.tsd, 0, 'i32') }}};
        _free(tlsMemory);
        _free(pthread.threadInfoStruct);
      }
      pthread.threadInfoStruct = 0;
      if (pthread.allocatedOwnStack && pthread.stackBase) _free(pthread.stackBase);
      pthread.stackBase = 0;
      if (pthread.worker) pthread.worker.pthread = null;
    },
    returnWorkerToPool: function(worker) {
      delete PThread.pthreads[worker.pthread.thread];
      //Note: worker is intentionally not terminated so the pool can dynamically grow.
      PThread.unusedWorkers.push(worker);
      PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker), 1); // Not a running Worker anymore
      PThread.freeThreadData(worker.pthread);
      worker.pthread = undefined; // Detach the worker from the pthread object, and return it to the worker pool as an unused worker.
    },
    receiveObjectTransfer: function(data) {
#if OFFSCREENCANVAS_SUPPORT
      if (typeof GL !== 'undefined') {
        for (var i in data.offscreenCanvases) {
          GL.offscreenCanvases[i] = data.offscreenCanvases[i];
        }
        if (!Module['canvas'] && data.moduleCanvasId && GL.offscreenCanvases[data.moduleCanvasId]) {
          Module['canvas'] = GL.offscreenCanvases[data.moduleCanvasId].offscreenCanvas;
          Module['canvas'].id = data.moduleCanvasId;
        }
      }
#endif
    },

    // Loads the WebAssembly module into the given list of Workers.
    // onFinishedLoading: A callback function that will be called once all of the workers have been initialized and are
    //                    ready to host pthreads.
    loadWasmModuleToWorker: function(worker, onFinishedLoading) {
      worker.onmessage = function(e) {
        var d = e['data'];
        var cmd = d['cmd'];
        // Sometimes we need to backproxy events to the calling thread (e.g. HTML5 DOM events handlers such as emscripten_set_mousemove_callback()), so keep track in a globally accessible variable about the thread that initiated the proxying.
        if (worker.pthread) PThread.currentProxiedOperationCallerThread = worker.pthread.threadInfoStruct;

        // If this message is intended to a recipient that is not the main thread, forward it to the target thread.
        if (d['targetThread'] && d['targetThread'] != _pthread_self()) {
          var thread = PThread.pthreads[d.targetThread];
          if (thread) {
            thread.worker.postMessage(e.data, d['transferList']);
          } else {
            console.error('Internal error! Worker sent a message "' + cmd + '" to target pthread ' + d['targetThread'] + ', but that thread no longer exists!');
          }
          PThread.currentProxiedOperationCallerThread = undefined;
          return;
        }

        if (cmd === 'processQueuedMainThreadWork') {
          // TODO: Must post message to main Emscripten thread in PROXY_TO_WORKER mode.
          _emscripten_main_thread_process_queued_calls();
        } else if (cmd === 'spawnThread') {
          spawnThread(e.data);
        } else if (cmd === 'cleanupThread') {
          cleanupThread(d['thread']);
        } else if (cmd === 'killThread') {
          killThread(d['thread']);
        } else if (cmd === 'cancelThread') {
          cancelThread(d['thread']);
        } else if (cmd === 'loaded') {
          worker.loaded = true;
          if (onFinishedLoading) onFinishedLoading(worker);
          // If this Worker is already pending to start running a thread, launch the thread now
          if (worker.runPthread) {
            worker.runPthread();
            delete worker.runPthread;
          }
        } else if (cmd === 'print') {
          out('Thread ' + d['threadId'] + ': ' + d['text']);
        } else if (cmd === 'printErr') {
          err('Thread ' + d['threadId'] + ': ' + d['text']);
        } else if (cmd === 'alert') {
          alert('Thread ' + d['threadId'] + ': ' + d['text']);
        } else if (cmd === 'exit') {
          var detached = worker.pthread && Atomics.load(HEAPU32, (worker.pthread.thread + {{{ C_STRUCTS.pthread.detached }}}) >> 2);
          if (detached) {
            PThread.returnWorkerToPool(worker);
          }
#if EXIT_RUNTIME // If building with -s EXIT_RUNTIME=0, no thread will post this message, so don't even compile it in.
        } else if (cmd === 'exitProcess') {
          // A pthread has requested to exit the whole application process (runtime).
          noExitRuntime = false;
          try {
            exit(d['returnCode']);
          } catch (e) {
            if (e instanceof ExitStatus) return;
            throw e;
          }
#endif
        } else if (cmd === 'cancelDone') {
          PThread.returnWorkerToPool(worker);
        } else if (cmd === 'objectTransfer') {
          PThread.receiveObjectTransfer(e.data);
        } else if (e.data.target === 'setimmediate') {
          worker.postMessage(e.data); // Worker wants to postMessage() to itself to implement setImmediate() emulation.
        } else {
          err("worker sent an unknown command " + cmd);
        }
        PThread.currentProxiedOperationCallerThread = undefined;
      };

      worker.onerror = function(e) {
        err('pthread sent an error! ' + e.filename + ':' + e.lineno + ': ' + e.message);
      };

#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) {
        worker.on('message', function(data) {
          worker.onmessage({ data: data });
        });
        worker.on('error', function(data) {
          worker.onerror(data);
        });
        worker.on('exit', function(data) {
          // TODO: update the worker queue?
          // See: https://github.com/emscripten-core/emscripten/issues/9763
        });
      }
#endif

#if ASSERTIONS
      assert(wasmMemory instanceof WebAssembly.Memory, 'WebAssembly memory should have been loaded by now!');
      assert(wasmModule instanceof WebAssembly.Module, 'WebAssembly Module should have been loaded by now!');
#endif

      // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
      worker.postMessage({
        'cmd': 'load',
        // If the application main .js file was loaded from a Blob, then it is not possible
        // to access the URL of the current script that could be passed to a Web Worker so that
        // it could load up the same file. In that case, developer must either deliver the Blob
        // object in Module['mainScriptUrlOrBlob'], or a URL to it, so that pthread Workers can
        // independently load up the same main application file.
        'urlOrBlob': Module['mainScriptUrlOrBlob'] || _scriptDir,
#if WASM2JS
        // the polyfill WebAssembly.Memory instance has function properties,
        // which will fail in postMessage, so just send a custom object with the
        // property we need, the buffer
        'wasmMemory': { 'buffer': wasmMemory.buffer },
#else // WASM2JS
        'wasmMemory': wasmMemory,
#endif // WASM2JS
        'wasmModule': wasmModule,
#if LOAD_SOURCE_MAP
        'wasmSourceMap': wasmSourceMap,
#endif
#if USE_OFFSET_CONVERTER
        'wasmOffsetConverter': wasmOffsetConverter,
#endif
      });
    },

    // Creates a new web Worker and places it in the unused worker pool to wait for its use.
    allocateUnusedWorker: function() {
#if MINIMAL_RUNTIME
      var pthreadMainJs = Module['worker'];
#else
      // Allow HTML module to configure the location where the 'worker.js' file will be loaded from,
      // via Module.locateFile() function. If not specified, then the default URL 'worker.js' relative
      // to the main html file is loaded.
      var pthreadMainJs = locateFile('{{{ PTHREAD_WORKER_FILE }}}');
#endif
#if PTHREADS_DEBUG
      out('Allocating a new web worker from ' + pthreadMainJs);
#endif
      PThread.unusedWorkers.push(new Worker(pthreadMainJs));
    },

    getNewWorker: function() {
      if (PThread.unusedWorkers.length == 0) {
        PThread.allocateUnusedWorker();
        PThread.loadWasmModuleToWorker(PThread.unusedWorkers[0]);
      }
      if (PThread.unusedWorkers.length > 0) return PThread.unusedWorkers.pop();
      else return null;
    },

    busySpinWait: function(msecs) {
      var t = performance.now() + msecs;
      while(performance.now() < t) {
        ;
      }
    }
  },

  $killThread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! killThread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in killThread!';
    {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
    var pthread = PThread.pthreads[pthread_ptr];
    pthread.worker.terminate();
    PThread.freeThreadData(pthread);
    // The worker was completely nuked (not just the pthread execution it was hosting), so remove it from running workers
    // but don't put it back to the pool.
    PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(pthread.worker), 1); // Not a running Worker anymore.
    pthread.worker.pthread = undefined;
  },

  $cleanupThread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! cleanupThread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in cleanupThread!';
    {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
    var pthread = PThread.pthreads[pthread_ptr];
    if (pthread) {
      var worker = pthread.worker;
      PThread.returnWorkerToPool(worker);
    }
  },

  $cancelThread: function(pthread_ptr) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! cancelThread() can only ever be called from main application thread!';
    if (!pthread_ptr) throw 'Internal Error! Null pthread_ptr in cancelThread!';
    var pthread = PThread.pthreads[pthread_ptr];
    pthread.worker.postMessage({ 'cmd': 'cancel' });
  },

  $spawnThread: function(threadParams) {
    if (ENVIRONMENT_IS_PTHREAD) throw 'Internal Error! spawnThread() can only ever be called from main application thread!';

    var worker = PThread.getNewWorker();

    if (worker.pthread !== undefined) throw 'Internal error!';
    if (!threadParams.pthread_ptr) throw 'Internal error, no pthread ptr!';
    PThread.runningWorkers.push(worker);

    // Allocate memory for thread-local storage and initialize it to zero.
    var tlsMemory = _malloc({{{ cDefine('PTHREAD_KEYS_MAX') }}} * 4);
    for (var i = 0; i < {{{ cDefine('PTHREAD_KEYS_MAX') }}}; ++i) {
      {{{ makeSetValue('tlsMemory', 'i*4', 0, 'i32') }}};
    }

    var stackHigh = threadParams.stackBase + threadParams.stackSize;

    var pthread = PThread.pthreads[threadParams.pthread_ptr] = { // Create a pthread info object to represent this thread.
      worker: worker,
      stackBase: threadParams.stackBase,
      stackSize: threadParams.stackSize,
      allocatedOwnStack: threadParams.allocatedOwnStack,
      thread: threadParams.pthread_ptr,
      threadInfoStruct: threadParams.pthread_ptr // Info area for this thread in Emscripten HEAP (shared)
    };
    var tis = pthread.threadInfoStruct >> 2;
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.threadStatus }}} >> 2), 0); // threadStatus <- 0, meaning not yet exited.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.threadExitCode }}} >> 2), 0); // threadExitCode <- 0.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.profilerBlock }}} >> 2), 0); // profilerBlock <- 0.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.detached }}} >> 2), threadParams.detached);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.tsd }}} >> 2), tlsMemory); // Init thread-local-storage memory array.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.tsd_used }}} >> 2), 0); // Mark initial status to unused.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.tid }}} >> 2), pthread.threadInfoStruct); // Main thread ID.
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.pid }}} >> 2), {{{ PROCINFO.pid }}}); // Process ID.

    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.attr }}} >> 2), threadParams.stackSize);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.stack_size }}} >> 2), threadParams.stackSize);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.stack }}} >> 2), stackHigh);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.attr }}} + 8 >> 2), stackHigh);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.attr }}} + 12 >> 2), threadParams.detached);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.attr }}} + 20 >> 2), threadParams.schedPolicy);
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.attr }}} + 24 >> 2), threadParams.schedPrio);

    var global_libc = _emscripten_get_global_libc();
    var global_locale = global_libc + {{{ C_STRUCTS.libc.global_locale }}};
    Atomics.store(HEAPU32, tis + ({{{ C_STRUCTS.pthread.locale }}} >> 2), global_locale);

#if PTHREADS_PROFILING
    PThread.createProfilerBlock(pthread.threadInfoStruct);
#endif

    worker.pthread = pthread;
    var msg = {
        'cmd': 'run',
        'start_routine': threadParams.startRoutine,
        'arg': threadParams.arg,
        'threadInfoStruct': threadParams.pthread_ptr,
        'selfThreadId': threadParams.pthread_ptr, // TODO: Remove this since thread ID is now the same as the thread address.
        'parentThreadId': threadParams.parent_pthread_ptr,
        'stackBase': threadParams.stackBase,
        'stackSize': threadParams.stackSize
    };
#if OFFSCREENCANVAS_SUPPORT
    // Note that we do not need to quote these names because they are only used in this file, and not from the external worker.js.
    msg.moduleCanvasId = threadParams.moduleCanvasId;
    msg.offscreenCanvases = threadParams.offscreenCanvases;
#endif
    worker.runPthread = function() {
      // Ask the worker to start executing its pthread entry point function.
      msg.time = performance.now();
      worker.postMessage(msg, threadParams.transferList);
    };
    if (worker.loaded) {
      worker.runPthread();
      delete worker.runPthread;
    }
  },

  emscripten_has_threading_support: function() {
    return typeof SharedArrayBuffer !== 'undefined';
  },

  emscripten_num_logical_cores: function() {
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) return require('os').cpus().length;
#endif
    return navigator['hardwareConcurrency'];
  },

  {{{ USE_LSAN || USE_ASAN ? 'emscripten_builtin_' : '' }}}pthread_create__deps: ['$spawnThread', 'pthread_getschedparam', 'pthread_self', 'memalign', '$resetPrototype'],
  {{{ USE_LSAN || USE_ASAN ? 'emscripten_builtin_' : '' }}}pthread_create: function(pthread_ptr, attr, start_routine, arg) {
    if (typeof SharedArrayBuffer === 'undefined') {
      err('Current environment does not support SharedArrayBuffer, pthreads are not available!');
      return {{{ cDefine('EAGAIN') }}};
    }
    if (!pthread_ptr) {
      err('pthread_create called with a null thread pointer!');
      return {{{ cDefine('EINVAL') }}};
    }

    var transferList = []; // List of JS objects that will transfer ownership to the Worker hosting the thread
    var error = 0;

#if OFFSCREENCANVAS_SUPPORT
    // Deduce which WebGL canvases (HTMLCanvasElements or OffscreenCanvases) should be passed over to the
    // Worker that hosts the spawned pthread.
    // Comma-delimited list of CSS selectors that must identify canvases by IDs: "#canvas1, #canvas2, ..."
    var transferredCanvasNames = attr ? {{{ makeGetValue('attr', 36, 'i32') }}} : 0;
#if OFFSCREENCANVASES_TO_PTHREAD
    // Proxied canvases string pointer -1 is used as a special token to fetch whatever canvases were passed to build
    // in -s OFFSCREENCANVASES_TO_PTHREAD= command line.
    if (transferredCanvasNames == -1) transferredCanvasNames = '{{{ OFFSCREENCANVASES_TO_PTHREAD }}}';
    else
#endif
    if (transferredCanvasNames) transferredCanvasNames = UTF8ToString(transferredCanvasNames).trim();
    if (transferredCanvasNames) transferredCanvasNames = transferredCanvasNames.split(',');
#if GL_DEBUG
    console.log('pthread_create: transferredCanvasNames="' + transferredCanvasNames + '"');
#endif

    var offscreenCanvases = {}; // Dictionary of OffscreenCanvas objects we'll transfer to the created thread to own
    var moduleCanvasId = Module['canvas'] ? Module['canvas'].id : '';
    for (var i in transferredCanvasNames) {
      var name = transferredCanvasNames[i].trim();
      var offscreenCanvasInfo;
      try {
        if (name == '#canvas') {
          if (!Module['canvas']) {
            err('pthread_create: could not find canvas with ID "' + name + '" to transfer to thread!');
            error = {{{ cDefine('EINVAL') }}};
            break;
          }
          name = Module['canvas'].id;
        }
#if ASSERTIONS
        assert(typeof GL === 'object', 'OFFSCREENCANVAS_SUPPORT assumes GL is in use (you can force-include it with -s \'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=["$GL"]\')');
#endif
        if (GL.offscreenCanvases[name]) {
          offscreenCanvasInfo = GL.offscreenCanvases[name];
          GL.offscreenCanvases[name] = null; // This thread no longer owns this canvas.
          if (Module['canvas'] instanceof OffscreenCanvas && name === Module['canvas'].id) Module['canvas'] = null;
        } else if (!ENVIRONMENT_IS_PTHREAD) {
          var canvas = (Module['canvas'] && Module['canvas'].id === name) ? Module['canvas'] : document.querySelector(name);
          if (!canvas) {
            err('pthread_create: could not find canvas with ID "' + name + '" to transfer to thread!');
            error = {{{ cDefine('EINVAL') }}};
            break;
          }
          if (canvas.controlTransferredOffscreen) {
            err('pthread_create: cannot transfer canvas with ID "' + name + '" to thread, since the current thread does not have control over it!');
            error = {{{ cDefine('EPERM') }}}; // Operation not permitted, some other thread is accessing the canvas.
            break;
          }
          if (canvas.transferControlToOffscreen) {
#if GL_DEBUG
            Module['printErr']('pthread_create: canvas.transferControlToOffscreen(), transferring canvas by name "' + name + '" (DOM id="' + canvas.id + '") from main thread to pthread');
#endif
            // Create a shared information block in heap so that we can control the canvas size from any thread.
            if (!canvas.canvasSharedPtr) {
              canvas.canvasSharedPtr = _malloc(12);
              {{{ makeSetValue('canvas.canvasSharedPtr', 0, 'canvas.width', 'i32') }}};
              {{{ makeSetValue('canvas.canvasSharedPtr', 4, 'canvas.height', 'i32') }}};
              {{{ makeSetValue('canvas.canvasSharedPtr', 8, 0, 'i32') }}}; // pthread ptr to the thread that owns this canvas, filled in below.
            }
            offscreenCanvasInfo = {
              offscreenCanvas: canvas.transferControlToOffscreen(),
              canvasSharedPtr: canvas.canvasSharedPtr,
              id: canvas.id
            }
            // After calling canvas.transferControlToOffscreen(), it is no longer possible to access certain operations on the canvas, such as resizing it or obtaining GL contexts via it.
            // Use this field to remember that we have permanently converted this Canvas to be controlled via an OffscreenCanvas (there is no way to undo this in the spec)
            canvas.controlTransferredOffscreen = true;
          } else {
            err('pthread_create: cannot transfer control of canvas "' + name + '" to pthread, because current browser does not support OffscreenCanvas!');
            // If building with OFFSCREEN_FRAMEBUFFER=1 mode, we don't need to be able to transfer control to offscreen, but WebGL can be proxied from worker to main thread.
#if !OFFSCREEN_FRAMEBUFFER
            Module['printErr']('pthread_create: Build with -s OFFSCREEN_FRAMEBUFFER=1 to enable fallback proxying of GL commands from pthread to main thread.');
            return {{{ cDefine('ENOSYS') }}}; // Function not implemented, browser doesn't have support for this.
#endif
          }
        }
        if (offscreenCanvasInfo) {
          transferList.push(offscreenCanvasInfo.offscreenCanvas);
          offscreenCanvases[offscreenCanvasInfo.id] = offscreenCanvasInfo;
        }
      } catch(e) {
        err('pthread_create: failed to transfer control of canvas "' + name + '" to OffscreenCanvas! Error: ' + e);
        return {{{ cDefine('EINVAL') }}}; // Hitting this might indicate an implementation bug or some other internal error
      }
    }
#endif

    // Synchronously proxy the thread creation to main thread if possible. If we need to transfer ownership of objects, then
    // proxy asynchronously via postMessage.
    if (ENVIRONMENT_IS_PTHREAD && (transferList.length === 0 || error)) {
      return _emscripten_sync_run_in_main_thread_4({{{ cDefine('EM_PROXIED_PTHREAD_CREATE') }}}, pthread_ptr, attr, start_routine, arg);
    }

    // If on the main thread, and accessing Canvas/OffscreenCanvas failed, abort with the detected error.
    if (error) return error;

    var stackSize = 0;
    var stackBase = 0;
    var detached = 0; // Default thread attr is PTHREAD_CREATE_JOINABLE, i.e. start as not detached.
    var schedPolicy = 0; /*SCHED_OTHER*/
    var schedPrio = 0;
    if (attr) {
      stackSize = {{{ makeGetValue('attr', 0, 'i32') }}};
      // Musl has a convention that the stack size that is stored to the pthread attribute structure is always musl's #define DEFAULT_STACK_SIZE
      // smaller than the actual created stack size. That is, stored stack size of 0 would mean a stack of DEFAULT_STACK_SIZE in size. All musl
      // functions hide this impl detail, and offset the size transparently, so pthread_*() API user does not see this offset when operating with
      // the pthread API. When reading the structure directly on JS side however, we need to offset the size manually here.
      stackSize += 81920 /*DEFAULT_STACK_SIZE*/;
      stackBase = {{{ makeGetValue('attr', 8, 'i32') }}};
      detached = {{{ makeGetValue('attr', 12/*_a_detach*/, 'i32') }}} !== 0/*PTHREAD_CREATE_JOINABLE*/;
      var inheritSched = {{{ makeGetValue('attr', 16/*_a_sched*/, 'i32') }}} === 0/*PTHREAD_INHERIT_SCHED*/;
      if (inheritSched) {
        var prevSchedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        var prevSchedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
        // If we are inheriting the scheduling properties from the parent thread, we need to identify the parent thread properly - this function call may
        // be getting proxied, in which case _pthread_self() will point to the thread performing the proxying, not the thread that initiated the call.
        var parentThreadPtr = PThread.currentProxiedOperationCallerThread ? PThread.currentProxiedOperationCallerThread : _pthread_self();
        _pthread_getschedparam(parentThreadPtr, attr + 20, attr + 24);
        schedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        schedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
        {{{ makeSetValue('attr', 20/*_a_policy*/, 'prevSchedPolicy', 'i32') }}};
        {{{ makeSetValue('attr', 24/*_a_prio*/, 'prevSchedPrio', 'i32') }}};
      } else {
        schedPolicy = {{{ makeGetValue('attr', 20/*_a_policy*/, 'i32') }}};
        schedPrio = {{{ makeGetValue('attr', 24/*_a_prio*/, 'i32') }}};
      }
    } else {
      // According to http://man7.org/linux/man-pages/man3/pthread_create.3.html, default stack size if not specified is 2 MB, so follow that convention.
      stackSize = {{{ DEFAULT_PTHREAD_STACK_SIZE }}};
    }
    var allocatedOwnStack = stackBase == 0; // If allocatedOwnStack == true, then the pthread impl maintains the stack allocation.
    if (allocatedOwnStack) {
      stackBase = _memalign({{{ STACK_ALIGN }}}, stackSize); // Allocate a stack if the user doesn't want to place the stack in a custom memory area.
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

#if OFFSCREENCANVAS_SUPPORT
    // Register for each of the transferred canvases that the new thread now owns the OffscreenCanvas.
    for (var i in offscreenCanvases) {
      {{{ makeSetValue('offscreenCanvases[i].canvasSharedPtr', 8, 'threadInfoStruct', 'i32') }}}; // pthread ptr to the thread that owns this canvas.
    }
#endif

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
      spawnThread(threadParams);
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

#if MINIMAL_RUNTIME
  emscripten_check_blocking_allowed__deps: ['$warnOnce'],
#endif
  emscripten_check_blocking_allowed: function() {
#if ASSERTIONS || IN_TEST_HARNESS || !MINIMAL_RUNTIME || !ALLOW_BLOCKING_ON_MAIN_THREAD
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) return;
#endif

    if (ENVIRONMENT_IS_WORKER) return; // Blocking in a worker/pthread is fine.

    warnOnce('Blocking on the main thread is very dangerous, see https://emscripten.org/docs/porting/pthreads.html#blocking-on-the-main-browser-thread');
#if !ALLOW_BLOCKING_ON_MAIN_THREAD
    abort('Blocking on the main thread is not allowed by default. See https://emscripten.org/docs/porting/pthreads.html#blocking-on-the-main-browser-thread');
#endif

#endif
  },

  _emscripten_do_pthread_join__deps: ['$cleanupThread', '_pthread_testcancel_js', 'emscripten_main_thread_process_queued_calls', 'emscripten_futex_wait',
#if ASSERTIONS || IN_TEST_HARNESS || !MINIMAL_RUNTIME || !ALLOW_BLOCKING_ON_MAIN_THREAD
  'emscripten_check_blocking_allowed'
#endif
  ],
  _emscripten_do_pthread_join: function(thread, status, block) {
    if (!thread) {
      err('pthread_join attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    if (ENVIRONMENT_IS_PTHREAD && selfThreadId == thread) {
      err('PThread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK;
    }
    else if (!ENVIRONMENT_IS_PTHREAD && PThread.mainThreadBlock == thread) {
      err('Main thread ' + thread + ' is attempting to join to itself!');
      return ERRNO_CODES.EDEADLK;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_join attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }

    var detached = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2);
    if (detached) {
      err('Attempted to join thread ' + thread + ', which was already detached!');
      return ERRNO_CODES.EINVAL; // The thread is already detached, can no longer join it!
    }

#if ASSERTIONS || IN_TEST_HARNESS || !MINIMAL_RUNTIME || !ALLOW_BLOCKING_ON_MAIN_THREAD
    if (block) {
      _emscripten_check_blocking_allowed();
    }
#endif

    for (;;) {
      var threadStatus = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
      if (threadStatus == 1) { // Exited?
        var threadExitCode = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadExitCode }}} ) >> 2);
        if (status) {{{ makeSetValue('status', 0, 'threadExitCode', 'i32') }}};
        Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 1); // Mark the thread as detached.

        if (!ENVIRONMENT_IS_PTHREAD) cleanupThread(thread);
        else postMessage({ 'cmd': 'cleanupThread', 'thread': thread });
        return 0;
      }
      if (!block) {
        return ERRNO_CODES.EBUSY;
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

  {{{ USE_LSAN ? 'emscripten_builtin_' : '' }}}pthread_join__deps: ['_emscripten_do_pthread_join'],
  {{{ USE_LSAN ? 'emscripten_builtin_' : '' }}}pthread_join: function(thread, status) {
    return __emscripten_do_pthread_join(thread, status, true);
  },

  pthread_tryjoin_np__deps: ['_emscripten_do_pthread_join'],
  pthread_tryjoin_np: function(thread, status) {
    return __emscripten_do_pthread_join(thread, status, false);
  },

  pthread_kill__deps: ['$killThread'],
  pthread_kill: function(thread, signal) {
    if (signal < 0 || signal >= 65/*_NSIG*/) return ERRNO_CODES.EINVAL;
    if (thread === PThread.MAIN_THREAD_ID) {
      if (signal == 0) return 0; // signal == 0 is a no-op.
      err('Main thread (id=' + thread + ') cannot be killed with pthread_kill!');
      return ERRNO_CODES.ESRCH;
    }
    if (!thread) {
      err('pthread_kill attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_kill attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    if (signal != 0) {
      if (!ENVIRONMENT_IS_PTHREAD) killThread(thread);
      else postMessage({ 'cmd': 'killThread', 'thread': thread});
    }
    return 0;
  },

  pthread_cancel__deps: ['$cancelThread'],
  pthread_cancel: function(thread) {
    if (thread === PThread.MAIN_THREAD_ID) {
      err('Main thread (id=' + thread + ') cannot be canceled!');
      return ERRNO_CODES.ESRCH;
    }
    if (!thread) {
      err('pthread_cancel attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_cancel attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    Atomics.compareExchange(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2, 0, 2); // Signal the thread that it needs to cancel itself.
    if (!ENVIRONMENT_IS_PTHREAD) cancelThread(thread);
    else postMessage({ 'cmd': 'cancelThread', 'thread': thread});
    return 0;
  },

  pthread_detach: function(thread) {
    if (!thread) {
      err('pthread_detach attempted on a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_detach attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return ERRNO_CODES.ESRCH;
    }
    var threadStatus = Atomics.load(HEAPU32, (thread + {{{ C_STRUCTS.pthread.threadStatus }}} ) >> 2);
    // Follow musl convention: detached:0 means not detached, 1 means the thread was created as detached, and 2 means that the thread was detached via pthread_detach.
    var wasDetached = Atomics.compareExchange(HEAPU32, (thread + {{{ C_STRUCTS.pthread.detached }}} ) >> 2, 0, 2);

    return wasDetached ? ERRNO_CODES.EINVAL : 0;
  },

  pthread_exit__deps: ['exit'],
  pthread_exit: function(status) {
    if (!ENVIRONMENT_IS_PTHREAD) _exit(status);
    else PThread.threadExit(status);
    // pthread_exit is marked noReturn, so we must not return from it.
    if (ENVIRONMENT_IS_NODE) {
      // exit the pthread properly on node, as a normal JS exception will halt
      // the entire application.
      process.exit(status);
    }
    throw 'unwind';
  },

  _pthread_ptr: 0,
  _pthread_is_main_runtime_thread: 0,
  _pthread_is_main_browser_thread: 0,

  $registerPthreadPtr__deps: ['_pthread_ptr', '_pthread_is_main_runtime_thread', '_pthread_is_main_browser_thread'],
  $registerPthreadPtr__asm: true,
  $registerPthreadPtr__sig: 'viii',
  $registerPthreadPtr: function(pthreadPtr, isMainBrowserThread, isMainRuntimeThread) {
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
      err('pthread_getschedparam called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_getschedparam attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
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
      err('pthread_setschedparam called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_setschedparam attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
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
      err('pthread_setschedprio called with a null thread pointer!');
      return ERRNO_CODES.ESRCH;
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_setschedprio attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
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

  pthread_cleanup_push__sig: 'vii',
  pthread_cleanup_push: function(routine, arg) {
    PThread.threadExitHandlers.push(function() { {{{ makeDynCall('vi', 'routine') }}}(arg) });
  },

  pthread_cleanup_pop: function(execute) {
    var routine = PThread.threadExitHandlers.pop();
    if (execute) routine();
  },

  // pthread_sigmask - examine and change mask of blocked signals
  pthread_sigmask: function(how, set, oldset) {
    err('pthread_sigmask() is not supported: this is a no-op.');
    return 0;
  },

  pthread_atfork: function(prepare, parent, child) {
    err('fork() is not supported: pthread_atfork is a no-op.');
    return 0;
  },

  // Stores the memory address that the main thread is waiting on, if any.
  _main_thread_futex_wait_address: '0',

  // Returns 0 on success, or one of the values -ETIMEDOUT, -EWOULDBLOCK or -EINVAL on error.
  emscripten_futex_wait__deps: ['_main_thread_futex_wait_address', 'emscripten_main_thread_process_queued_calls'],
  emscripten_futex_wait: function(addr, val, timeout) {
    if (addr <= 0 || addr > HEAP8.length || addr&3 != 0) return -{{{ cDefine('EINVAL') }}};
    if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_WORKER) {
#if PTHREADS_PROFILING
      PThread.setThreadStatusConditional(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}}, {{{ cDefine('EM_THREAD_STATUS_WAITFUTEX') }}});
#endif
      var ret = Atomics.wait(HEAP32, addr >> 2, val, timeout);
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
    // Waking (at least) INT_MAX waiters is defined to mean wake all callers.
    // For Atomics.notify() API Infinity is to be passed in that case.
    if (count >= {{{ cDefine('INT_MAX') }}}) count = Infinity;

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
    var ret = Atomics.notify(HEAP32, addr >> 2, count);
    if (ret >= 0) return ret + mainThreadWoken;
    throw 'Atomics.notify returned an unexpected value ' + ret;
  },

  __atomic_is_lock_free: function(size, ptr) {
    return size <= 4 && (size & (size-1)) == 0 && (ptr&(size-1)) == 0;
  },

  __call_main: function(argc, argv) {
    var returnCode = _main(argc, argv);
#if EXIT_RUNTIME
    if (!noExitRuntime) {
      // exitRuntime enabled, proxied main() finished in a pthread, shut down the process.
#if ASSERTIONS
      out('Proxied main thread 0x' + _pthread_self().toString(16) + ' finished with return code ' + returnCode + '. EXIT_RUNTIME=1 set, quitting process.');
#endif
      postMessage({ 'cmd': 'exitProcess', 'returnCode': returnCode });
      return returnCode;
    }
#else
    // EXIT_RUNTIME==0 set on command line, keeping main thread alive.
#if ASSERTIONS
    out('Proxied main thread 0x' + _pthread_self().toString(16) + ' finished with return code ' + returnCode + '. EXIT_RUNTIME=0 set, so keeping main thread alive for asynchronous event operations.');
#endif
#endif
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
  },

  emscripten_proxy_to_main_thread_js__docs: '/** @type{function(number, (number|boolean), ...(number|boolean))} */',
  emscripten_proxy_to_main_thread_js: function(index, sync) {
    // Additional arguments are passed after those two, which are the actual
    // function arguments.
    // The serialization buffer contains the number of call params, and then
    // all the args here.
    // We also pass 'sync' to C separately, since C needs to look at it.
    var numCallArgs = arguments.length - 2;
#if ASSERTIONS
    if (numCallArgs > {{{ cDefine('EM_QUEUED_JS_CALL_MAX_ARGS') }}}-1) throw 'emscripten_proxy_to_main_thread_js: Too many arguments ' + numCallArgs + ' to proxied function idx=' + index + ', maximum supported is ' + ({{{ cDefine('EM_QUEUED_JS_CALL_MAX_ARGS') }}}-1) + '!';
#endif
    // Allocate a buffer, which will be copied by the C code.
    var stack = stackSave();
    // First passed parameter specifies the number of arguments to the function.
    var args = stackAlloc(numCallArgs * 8);
    var b = args >> 3;
    for (var i = 0; i < numCallArgs; i++) {
      HEAPF64[b + i] = arguments[2 + i];
    }
    var ret = _emscripten_run_in_main_runtime_thread_js(index, numCallArgs, args, sync);
    stackRestore(stack);
    return ret;
  },

  emscripten_receive_on_main_thread_js_callArgs: '=[]',

  emscripten_receive_on_main_thread_js__deps: [
    'emscripten_proxy_to_main_thread_js',
    'emscripten_receive_on_main_thread_js_callArgs',
    '$readAsmConstArgs'],
  emscripten_receive_on_main_thread_js: function(index, numCallArgs, args) {
    _emscripten_receive_on_main_thread_js_callArgs.length = numCallArgs;
    var b = args >> 3;
    for (var i = 0; i < numCallArgs; i++) {
      _emscripten_receive_on_main_thread_js_callArgs[i] = HEAPF64[b + i];
    }
    // Proxied JS library funcs are encoded as positive values, and
    // EM_ASMs as negative values (see include_asm_consts)
    var isEmAsmConst = index < 0;
    var func = !isEmAsmConst ? proxiedFunctionTable[index] : ASM_CONSTS[-index - 1];
#if ASSERTIONS
    assert(func.length == numCallArgs, 'Call args mismatch in emscripten_receive_on_main_thread_js');
#endif
    return func.apply(null, _emscripten_receive_on_main_thread_js_callArgs);
  },

  $establishStackSpace: function(stackTop, stackMax) {
    STACK_BASE = STACKTOP = stackTop;
    STACK_MAX = stackMax;

#if STACK_OVERFLOW_CHECK >= 2
    ___set_stack_limits(STACK_BASE, STACK_MAX);
#endif

    // Call inside wasm module to set up the stack frame for this pthread in asm.js/wasm module scope
    stackRestore(stackTop);

#if STACK_OVERFLOW_CHECK
    // Write the stack cookie last, after we have set up the proper bounds and
    // current position of the stack.
    writeStackCookie();
#endif
  },

  // allow pthreads to check if noExitRuntime from worker.js
  $getNoExitRuntime: function() {
    return noExitRuntime;
  },

  // When using postMessage to send an object, it is processed by the structured clone algorithm.
  // The prototype, and hence methods, on that object is then lost. This function adds back the lost prototype.
  // This does not work with nested objects that has prototypes, but it suffices for WasmSourceMap and WasmOffsetConverter.
  $resetPrototype: function(constructor, attrs) {
    var object = Object.create(constructor.prototype);
    for (var key in attrs) {
      if (attrs.hasOwnProperty(key)) {
        object[key] = attrs[key];
      }
    }
    return object;
  },

  // This function is called internally to notify target thread ID that it has messages it needs to
  // process in its message queue inside the Wasm heap. As a helper, the caller must also pass the
  // ID of the main browser thread to this function, to avoid needlessly ping-ponging between JS and
  // Wasm boundaries.
  _emscripten_notify_thread_queue: function(targetThreadId, mainThreadId) {
    if (targetThreadId == mainThreadId) {
      postMessage({'cmd' : 'processQueuedMainThreadWork'});
    } else if (ENVIRONMENT_IS_PTHREAD) {
      postMessage({'targetThread': targetThreadId, 'cmd': 'processThreadQueue'});
    } else {
      var pthread = PThread.pthreads[targetThreadId];
      var worker = pthread && pthread.worker;
      if (!worker) {
#if ASSERTIONS
        err('Cannot send message to thread with ID ' + targetThreadId + ', unknown thread ID!');
#endif
        return /*0*/;
      }
      worker.postMessage({'cmd' : 'processThreadQueue'});
    }
    return 1;
  }
};

autoAddDeps(LibraryPThread, '$PThread');
mergeInto(LibraryManager.library, LibraryPThread);
