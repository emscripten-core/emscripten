/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryPThread = {
  $PThread__postset: 'if (!ENVIRONMENT_IS_PTHREAD) PThread.initMainThreadBlock();',
  $PThread__deps: ['_emscripten_thread_init',
                   'emscripten_futex_wake', '$killThread',
                   '$cancelThread', '$cleanupThread', '$zeroMemory',
                   '_emscripten_thread_free_data',
                   'exit',
#if !MINIMAL_RUNTIME
                   '$handleException',
#endif
                   ],
  $PThread: {
    // Contains all Workers that are idle/unused and not currently hosting an
    // executing pthread.  Unused Workers can either be pooled up before page
    // startup, but also when a pthread quits, its hosting Worker is not
    // terminated, but is returned to this pool as an optimization so that
    // starting the next thread is faster.
    unusedWorkers: [],
    // Contains all Workers that are currently hosting an active pthread.
    runningWorkers: [],
    tlsInitFunctions: [],
    initMainThreadBlock: function() {
#if ASSERTIONS
      assert(!ENVIRONMENT_IS_PTHREAD);
#endif

#if PTHREAD_POOL_SIZE
      var pthreadPoolSize = {{{ PTHREAD_POOL_SIZE }}};
      // Start loading up the Worker pool, if requested.
      for (var i = 0; i < pthreadPoolSize; ++i) {
        PThread.allocateUnusedWorker();
      }
#endif
    },

    initWorker: function() {
#if USE_CLOSURE_COMPILER
      // worker.js is not compiled together with us, and must access certain
      // things.
      PThread['receiveObjectTransfer'] = PThread.receiveObjectTransfer;
      PThread['threadInit'] = PThread.threadInit;
#if !MINIMAL_RUNTIME
      PThread['setExitStatus'] = PThread.setExitStatus;
#endif
#endif
    },
    // Maps pthread_t to pthread info objects
    pthreads: {},

#if PTHREADS_PROFILING
    createProfilerBlock: function(pthreadPtr) {
      var profilerBlock = _malloc({{{ C_STRUCTS.thread_profiler_block.__size__ }}});
      Atomics.store(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2, profilerBlock);

      // Zero fill contents at startup.
      zeroMemory(profilerBlock, {{{ C_STRUCTS.thread_profiler_block.__size__ }}});
      HEAPF64[(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.currentStatusStartTime }}} ) >> 3] = performance.now();
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
      stringToUTF8(name, profilerBlock + {{{ C_STRUCTS.thread_profiler_block.name }}}, {{{ cDefine('EM_THREAD_NAME_MAX') }}});
    },

    getThreadName: function(pthreadPtr) {
      var profilerBlock = Atomics.load(HEAPU32, (pthreadPtr + {{{ C_STRUCTS.pthread.profilerBlock }}} ) >> 2);
      if (!profilerBlock) return "";
      return UTF8ToString(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.name }}});
    },

    threadStatusToString: function(threadStatus) {
      switch (threadStatus) {
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
#endif

#if !MINIMAL_RUNTIME
    setExitStatus: function(status) {
      EXITSTATUS = status;
    },
#endif

    terminateAllThreads: function() {
#if ASSERTIONS
      assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! terminateAllThreads() can only ever be called from main application thread!');
#endif
#if PTHREADS_DEBUG
      err('terminateAllThreads');
#endif
      for (var t in PThread.pthreads) {
        var pthread = PThread.pthreads[t];
        if (pthread && pthread.worker) {
          PThread.returnWorkerToPool(pthread.worker);
        }
      }

#if ASSERTIONS
      // At this point there should be zero pthreads and zero runningWorkers.
      // All workers should be now be the unused queue.
      assert(Object.keys(PThread.pthreads).length === 0);
      assert(PThread.runningWorkers.length === 0);
#endif

      for (var i = 0; i < PThread.unusedWorkers.length; ++i) {
        var worker = PThread.unusedWorkers[i];
#if ASSERTIONS
        // This Worker should not be hosting a pthread at this time.
        assert(!worker.pthread);
#endif
        worker.terminate();
      }
      PThread.unusedWorkers = [];
    },
    returnWorkerToPool: function(worker) {
      // We don't want to run main thread queued calls here, since we are doing
      // some operations that leave the worker queue in an invalid state until
      // we are completely done (it would be bad if free() ends up calling a
      // queued pthread_create which looks at the global data structures we are
      // modifying).
      PThread.runWithoutMainThreadQueuedCalls(function() {
        delete PThread.pthreads[worker.pthread.threadInfoStruct];
        // Note: worker is intentionally not terminated so the pool can
        // dynamically grow.
        PThread.unusedWorkers.push(worker);
        PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker), 1);
        // Not a running Worker anymore
        __emscripten_thread_free_data(worker.pthread.threadInfoStruct);
        // Detach the worker from the pthread object, and return it to the
        // worker pool as an unused worker.
        worker.pthread = undefined;
      });
    },
    // Runs a function with processing of queued calls to the main thread
    // disabled. This is useful to avoid something like free() ending up waiting
    // for a lock, then running processing events, and those events can end up
    // doing things that interfere with what we were doing before (for example,
    // if we are tearing down a thread, calling free to erase its data could
    // end up calling a proxied pthread_create, which gets a free worker, and
    // can interfere).
    // This is only safe to call if we do not need queued calls to run. That is
    // the case when doing something like free(), which just needs the malloc
    // lock to be released.
    runWithoutMainThreadQueuedCalls: function(func) {
#if ASSERTIONS
      assert(PThread.mainRuntimeThread, 'runWithoutMainThreadQueuedCalls must be done on the main runtime thread');
      assert(__emscripten_allow_main_runtime_queued_calls);
#endif
      HEAP32[__emscripten_allow_main_runtime_queued_calls >> 2] = 0;
      try {
        func();
      } finally {
        HEAP32[__emscripten_allow_main_runtime_queued_calls >> 2] = 1;
      }
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
    // Called by worker.js each time a thread is started.
    threadInit: function() {
#if PTHREADS_DEBUG
      err('Pthread 0x' + _pthread_self().toString(16) + ' threadInit.');
#endif
#if PTHREADS_PROFILING
      PThread.setThreadStatus(_pthread_self(), {{{ cDefine('EM_THREAD_STATUS_RUNNING') }}});
#endif
      // Call thread init functions (these are the emscripten_tls_init for each
      // module loaded.
      for (var i in PThread.tlsInitFunctions) {
        PThread.tlsInitFunctions[i]();
      }
    },
    // Loads the WebAssembly module into the given list of Workers.
    // onFinishedLoading: A callback function that will be called once all of
    //                    the workers have been initialized and are
    //                    ready to host pthreads.
    loadWasmModuleToWorker: function(worker, onFinishedLoading) {
      worker.onmessage = function(e) {
        var d = e['data'];
        var cmd = d['cmd'];
        // Sometimes we need to backproxy events to the calling thread (e.g.
        // HTML5 DOM events handlers such as
        // emscripten_set_mousemove_callback()), so keep track in a globally
        // accessible variable about the thread that initiated the proxying.
        if (worker.pthread) PThread.currentProxiedOperationCallerThread = worker.pthread.threadInfoStruct;

        // If this message is intended to a recipient that is not the main thread, forward it to the target thread.
        if (d['targetThread'] && d['targetThread'] != _pthread_self()) {
          var thread = PThread.pthreads[d.targetThread];
          if (thread) {
            thread.worker.postMessage(d, d['transferList']);
          } else {
            err('Internal error! Worker sent a message "' + cmd + '" to target pthread ' + d['targetThread'] + ', but that thread no longer exists!');
          }
          PThread.currentProxiedOperationCallerThread = undefined;
          return;
        }

        if (cmd === 'processQueuedMainThreadWork') {
          // TODO: Must post message to main Emscripten thread in PROXY_TO_WORKER mode.
          _emscripten_main_thread_process_queued_calls();
        } else if (cmd === 'spawnThread') {
          spawnThread(d);
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
        } else if (cmd === 'detachedExit') {
#if ASSERTIONS
          assert(worker.pthread);
#endif
          PThread.returnWorkerToPool(worker);
        } else if (d.target === 'setimmediate') {
          // Worker wants to postMessage() to itself to implement setImmediate()
          // emulation.
          worker.postMessage(d);
        } else if (cmd === 'onAbort') {
          if (Module['onAbort']) {
            Module['onAbort'](d['arg']);
          }
        } else {
          err("worker sent an unknown command " + cmd);
        }
        PThread.currentProxiedOperationCallerThread = undefined;
      };

      worker.onerror = function(e) {
        err('pthread sent an error! ' + e.filename + ':' + e.lineno + ': ' + e.message);
        throw e;
      };

#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) {
        worker.on('message', function(data) {
          worker.onmessage({ data: data });
        });
        worker.on('error', function(e) {
          worker.onerror(e);
        });
        worker.on('detachedExit', function() {
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
        'urlOrBlob': Module['mainScriptUrlOrBlob']
#if !EXPORT_ES6
        || _scriptDir
#endif
        ,
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
#if MAIN_MODULE
        'dynamicLibraries': Module['dynamicLibraries'],
#endif
      });
    },

    // Creates a new web Worker and places it in the unused worker pool to wait for its use.
    allocateUnusedWorker: function() {
#if MINIMAL_RUNTIME
      var pthreadMainJs = Module['worker'];
#else
#if EXPORT_ES6 && USE_ES6_IMPORT_META
      // If we're using module output and there's no explicit override, use bundler-friendly pattern.
      if (!Module['locateFile']) {
#if PTHREADS_DEBUG
        err('Allocating a new web worker from ' + new URL('{{{ PTHREAD_WORKER_FILE }}}', import.meta.url));
#endif
#if TRUSTED_TYPES
        // Use Trusted Types compatible wrappers.
        if (typeof trustedTypes !== 'undefined' && trustedTypes.createPolicy) {
          var p = trustedTypes.createPolicy(
            'emscripten#workerPolicy1',
            {
              createScriptURL: function(ignored) {
                return new URL('{{{ PTHREAD_WORKER_FILE }}}', import.meta.url);
              }
            }
          );
          PThread.unusedWorkers.push(new Worker(p.createScriptURL('ignored')));
        } else
 #endif
        PThread.unusedWorkers.push(new Worker(new URL('{{{ PTHREAD_WORKER_FILE }}}', import.meta.url)));
        return;
      }
#endif
      // Allow HTML module to configure the location where the 'worker.js' file will be loaded from,
      // via Module.locateFile() function. If not specified, then the default URL 'worker.js' relative
      // to the main html file is loaded.
      var pthreadMainJs = locateFile('{{{ PTHREAD_WORKER_FILE }}}');
#endif
#if PTHREADS_DEBUG
      err('Allocating a new web worker from ' + pthreadMainJs);
#endif
#if TRUSTED_TYPES
      // Use Trusted Types compatible wrappers.
      if (typeof trustedTypes !== 'undefined' && trustedTypes.createPolicy) {
        var p = trustedTypes.createPolicy('emscripten#workerPolicy2', { createScriptURL: function(ignored) { return pthreadMainJs } });
        PThread.unusedWorkers.push(new Worker(p.createScriptURL('ignored')));
      } else
#endif
      PThread.unusedWorkers.push(new Worker(pthreadMainJs));
    },

    getNewWorker: function() {
      if (PThread.unusedWorkers.length == 0) {
#if !PROXY_TO_PTHREAD && PTHREAD_POOL_SIZE_STRICT
#if ASSERTIONS
        err('Tried to spawn a new thread, but the thread pool is exhausted.\n' +
        'This might result in a deadlock unless some threads eventually exit or the code explicitly breaks out to the event loop.\n' +
        'If you want to increase the pool size, use setting `-s PTHREAD_POOL_SIZE=...`.'
#if PTHREAD_POOL_SIZE_STRICT == 1
        + '\nIf you want to throw an explicit error instead of the risk of deadlocking in those cases, use setting `-s PTHREAD_POOL_SIZE_STRICT=2`.'
#endif
        );
#endif // ASSERTIONS

#if PTHREAD_POOL_SIZE_STRICT == 2
        // Don't return a Worker, which will translate into an EAGAIN error.
        return;
#endif
#endif
        PThread.allocateUnusedWorker();
        PThread.loadWasmModuleToWorker(PThread.unusedWorkers[0]);
      }
      return PThread.unusedWorkers.pop();
    }
  },

  $killThread__deps: ['_emscripten_thread_free_data'],
  $killThread: function(pthread_ptr) {
#if PTHREADS_DEBUG
    err('killThread 0x' + pthread_ptr.toString(16));
#endif
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! killThread() can only ever be called from main application thread!');
    assert(pthread_ptr, 'Internal Error! Null pthread_ptr in killThread!');
#endif
    {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
    var pthread = PThread.pthreads[pthread_ptr];
    delete PThread.pthreads[pthread_ptr];
    pthread.worker.terminate();
    __emscripten_thread_free_data(pthread_ptr);
    // The worker was completely nuked (not just the pthread execution it was hosting), so remove it from running workers
    // but don't put it back to the pool.
    PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(pthread.worker), 1); // Not a running Worker anymore.
    pthread.worker.pthread = undefined;
  },

  __emscripten_thread_cleanup: function(thread) {
    if (!ENVIRONMENT_IS_PTHREAD) cleanupThread(thread);
    else postMessage({ 'cmd': 'cleanupThread', 'thread': thread });
  },

  $cleanupThread: function(pthread_ptr) {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! cleanupThread() can only ever be called from main application thread!');
    assert(pthread_ptr, 'Internal Error! Null pthread_ptr in cleanupThread!');
#endif
    var pthread = PThread.pthreads[pthread_ptr];
    // If pthread has been removed from this map this also means that pthread_ptr points
    // to already freed data. Such situation may occur in following circumstance:
    // Joining thread from non-main browser thread (this also includes thread running main()
    // when compiled with `PROXY_TO_PTHREAD`) - in such situation it may happen that following
    // code flow occur (MB - Main Browser Thread, S1, S2 - Worker Threads):
    // S2: thread ends, 'exit' message is sent to MB
    // S1: calls pthread_join(S2), this causes:
    //     a. S2 is marked as detached,
    //     b. 'cleanupThread' message is sent to MB.
    // MB: handles 'exit' message, as thread is detached, so returnWorkerToPool()
    //     is called and all thread related structs are freed/released.
    // MB: handles 'cleanupThread' message which calls this function.
    if (pthread) {
      {{{ makeSetValue('pthread_ptr', C_STRUCTS.pthread.self, 0, 'i32') }}};
      var worker = pthread.worker;
      PThread.returnWorkerToPool(worker);
    }
  },

  $registerTlsInit: function(tlsInitFunc, moduleExports, metadata) {
#if DYLINK_DEBUG
    err("registerTlsInit: " + tlsInitFunc);
#endif
#if RELOCATABLE
    // In relocatable builds, we use the result of calling tlsInitFunc
    // (`emscripten_tls_init`) to relocate the TLS exports of the module
    // according to this new __tls_base.
    function tlsInitWrapper() {
      var __tls_base = tlsInitFunc();
#if DYLINK_DEBUG
      err('tlsInit -> ' + __tls_base);
#endif
      if (!__tls_base) {
#if ASSERTIONS
        // __tls_base should never be zero if there are tls exports
        assert(__tls_base || Object.keys(metadata.tlsExports).length == 0);
#endif
        return;
      }
      for (var sym in metadata.tlsExports) {
        metadata.tlsExports[sym] = moduleExports[sym];
      }
      relocateExports(metadata.tlsExports, __tls_base, /*replace=*/true);
    }

    // Register this function so that its gets called for each thread on
    // startup.
    PThread.tlsInitFunctions.push(tlsInitWrapper);

    // If the main thread is already running we also need to call this function
    // now.  If the main thread is not yet running this will happen when it
    // is initialized and processes `PThread.tlsInitFunctions`.
    if (runtimeInitialized) {
      tlsInitWrapper();
    }
#else
    PThread.tlsInitFunctions.push(tlsInitFunc);
#endif
  },

  $cancelThread: function(pthread_ptr) {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! cancelThread() can only ever be called from main application thread!');
    assert(pthread_ptr, 'Internal Error! Null pthread_ptr in cancelThread!');
#endif
    var pthread = PThread.pthreads[pthread_ptr];
    pthread.worker.postMessage({ 'cmd': 'cancel' });
  },

  $spawnThread: function(threadParams) {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! spawnThread() can only ever be called from main application thread!');
    assert(threadParams.pthread_ptr, 'Internal error, no pthread ptr!');
#endif

    var worker = PThread.getNewWorker();
    if (!worker) {
      // No available workers in the PThread pool.
      return {{{ cDefine('EAGAIN') }}};
    }
#if ASSERTIONS
    assert(!worker.pthread, 'Internal error!');
#endif

    PThread.runningWorkers.push(worker);

    // Create a pthread info object to represent this thread.
    var pthread = PThread.pthreads[threadParams.pthread_ptr] = {
      worker: worker,
      // Info area for this thread in Emscripten HEAP (shared)
      threadInfoStruct: threadParams.pthread_ptr
    };

#if PTHREADS_PROFILING
    PThread.createProfilerBlock(pthread.threadInfoStruct);
#endif

    worker.pthread = pthread;
    var msg = {
        'cmd': 'run',
        'start_routine': threadParams.startRoutine,
        'arg': threadParams.arg,
        'threadInfoStruct': threadParams.pthread_ptr,
    };
#if OFFSCREENCANVAS_SUPPORT
    // Note that we do not need to quote these names because they are only used
    // in this file, and not from the external worker.js.
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
    return 0;
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

  __emscripten_init_main_thread_js: function(tb) {
#if PTHREADS_PROFILING
    PThread.createProfilerBlock(tb);
    PThread.setThreadName(tb, "Browser main thread");
#endif

    // Pass the thread address to the native code where they stored in wasm
    // globals which act as a form of TLS. Global constructors trying
    // to access this value will read the wrong value, but that is UB anyway.
    __emscripten_thread_init(tb, /*isMainBrowserThread=*/!ENVIRONMENT_IS_WORKER, /*isMainRuntimeThread=*/1, /*canBlock=*/!ENVIRONMENT_IS_WEB);
#if ASSERTIONS
    PThread.mainRuntimeThread = true;
    // Verify that this native symbol used by futex_wait/wake is exported correctly.
    assert(__emscripten_main_thread_futex > 0);
#endif
    PThread.threadInit();
  },

  // ASan wraps the emscripten_builtin_pthread_create call in
  // __lsan::ScopedInterceptorDisabler.  Unfortunately, that only disables it on
  // the thread that made the call.  __pthread_create_js gets proxied to the
  // main thread, where LSan is not disabled. This makes it necessary for us to
  // disable LSan here (using __noleakcheck), so that it does not detect
  // pthread's internal allocations as leaks.  If/when we remove all the
  // allocations from __pthread_create_js we could also remove this.
  __pthread_create_js__noleakcheck: true,
  __pthread_create_js__sig: 'iiiii',
  __pthread_create_js__deps: ['$spawnThread', 'pthread_self', 'memalign', 'emscripten_sync_run_in_main_thread_4'],
  __pthread_create_js: function(pthread_ptr, attr, start_routine, arg) {
    if (typeof SharedArrayBuffer === 'undefined') {
      err('Current environment does not support SharedArrayBuffer, pthreads are not available!');
      return {{{ cDefine('EAGAIN') }}};
    }

    // List of JS objects that will transfer ownership to the Worker hosting the thread
    var transferList = [];
    var error = 0;

#if OFFSCREENCANVAS_SUPPORT
    // Deduce which WebGL canvases (HTMLCanvasElements or OffscreenCanvases) should be passed over to the
    // Worker that hosts the spawned pthread.
    // Comma-delimited list of CSS selectors that must identify canvases by IDs: "#canvas1, #canvas2, ..."
    var transferredCanvasNames = attr ? {{{ makeGetValue('attr', C_STRUCTS.pthread_attr_t._a_transferredcanvases, POINTER_TYPE) }}} : 0;
#if OFFSCREENCANVASES_TO_PTHREAD
    // Proxied canvases string pointer -1 is used as a special token to fetch
    // whatever canvases were passed to build in -s
    // OFFSCREENCANVASES_TO_PTHREAD= command line.
    if (transferredCanvasNames == -1) transferredCanvasNames = '{{{ OFFSCREENCANVASES_TO_PTHREAD }}}';
    else
#endif
    if (transferredCanvasNames) transferredCanvasNames = UTF8ToString(transferredCanvasNames).trim();
    if (transferredCanvasNames) transferredCanvasNames = transferredCanvasNames.split(',');
#if GL_DEBUG
    err('pthread_create: transferredCanvasNames="' + transferredCanvasNames + '"');
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
            err('pthread_create: canvas.transferControlToOffscreen(), transferring canvas by name "' + name + '" (DOM id="' + canvas.id + '") from main thread to pthread');
#endif
            // Create a shared information block in heap so that we can control
            // the canvas size from any thread.
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
            // After calling canvas.transferControlToOffscreen(), it is no
            // longer possible to access certain operations on the canvas, such
            // as resizing it or obtaining GL contexts via it.
            // Use this field to remember that we have permanently converted
            // this Canvas to be controlled via an OffscreenCanvas (there is no
            // way to undo this in the spec)
            canvas.controlTransferredOffscreen = true;
          } else {
            err('pthread_create: cannot transfer control of canvas "' + name + '" to pthread, because current browser does not support OffscreenCanvas!');
            // If building with OFFSCREEN_FRAMEBUFFER=1 mode, we don't need to
            // be able to transfer control to offscreen, but WebGL can be
            // proxied from worker to main thread.
#if !OFFSCREEN_FRAMEBUFFER
            err('pthread_create: Build with -s OFFSCREEN_FRAMEBUFFER=1 to enable fallback proxying of GL commands from pthread to main thread.');
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

    // Synchronously proxy the thread creation to main thread if possible. If we
    // need to transfer ownership of objects, then proxy asynchronously via
    // postMessage.
    if (ENVIRONMENT_IS_PTHREAD && (transferList.length === 0 || error)) {
      return _emscripten_sync_run_in_main_thread_4({{{ cDefine('EM_PROXIED_PTHREAD_CREATE') }}}, pthread_ptr, attr, start_routine, arg);
    }

    // If on the main thread, and accessing Canvas/OffscreenCanvas failed, abort
    // with the detected error.
    if (error) return error;

#if OFFSCREENCANVAS_SUPPORT
    // Register for each of the transferred canvases that the new thread now
    // owns the OffscreenCanvas.
    for (var i in offscreenCanvases) {
      // pthread ptr to the thread that owns this canvas.
      {{{ makeSetValue('offscreenCanvases[i].canvasSharedPtr', 8, 'pthread_ptr', 'i32') }}};
    }
#endif

    var threadParams = {
      startRoutine: start_routine,
      pthread_ptr: pthread_ptr,
      arg: arg,
#if OFFSCREENCANVAS_SUPPORT
      moduleCanvasId: moduleCanvasId,
      offscreenCanvases: offscreenCanvases,
#endif
      transferList: transferList
    };

    if (ENVIRONMENT_IS_PTHREAD) {
      // The prepopulated pool of web workers that can host pthreads is stored
      // in the main JS thread. Therefore if a pthread is attempting to spawn a
      // new thread, the thread creation must be deferred to the main JS thread.
      threadParams.cmd = 'spawnThread';
      postMessage(threadParams, transferList);
      // When we defer thread creation this way, we have no way to detect thread
      // creation synchronously today, so we have to assume success and return 0.
      return 0;
    }

    // We are the main thread, so we have the pthread warmup pool in this
    // thread and can fire off JS thread creation directly ourselves.
    return spawnThread(threadParams);
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

  pthread_kill__deps: ['$killThread', 'emscripten_main_browser_thread_id'],
  pthread_kill: function(thread, signal) {
    if (signal < 0 || signal >= 65/*_NSIG*/) return {{{ cDefine('EINVAL') }}};
    if (thread === _emscripten_main_browser_thread_id()) {
      if (signal == 0) return 0; // signal == 0 is a no-op.
      err('Main thread (id=' + thread + ') cannot be killed with pthread_kill!');
      return {{{ cDefine('ESRCH') }}};
    }
    if (!thread) {
      err('pthread_kill attempted on a null thread pointer!');
      return {{{ cDefine('ESRCH') }}};
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_kill attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return {{{ cDefine('ESRCH') }}};
    }
    if (signal != 0) {
      if (!ENVIRONMENT_IS_PTHREAD) killThread(thread);
      else postMessage({ 'cmd': 'killThread', 'thread': thread});
    }
    return 0;
  },

  pthread_cancel__deps: ['$cancelThread', 'emscripten_main_browser_thread_id'],
  pthread_cancel: function(thread) {
    if (thread === _emscripten_main_browser_thread_id()) {
      err('Main thread (id=' + thread + ') cannot be canceled!');
      return {{{ cDefine('ESRCH') }}};
    }
    if (!thread) {
      err('pthread_cancel attempted on a null thread pointer!');
      return {{{ cDefine('ESRCH') }}};
    }
    var self = {{{ makeGetValue('thread', C_STRUCTS.pthread.self, 'i32') }}};
    if (self !== thread) {
      err('pthread_cancel attempted on thread ' + thread + ', which does not point to a valid thread, or does not exist anymore!');
      return {{{ cDefine('ESRCH') }}};
    }
    // Signal the thread that it needs to cancel itself.
    Atomics.store(HEAPU32, (thread + {{{ C_STRUCTS.pthread.cancel }}}) >> 2, 1);
    if (!ENVIRONMENT_IS_PTHREAD) cancelThread(thread);
    else postMessage({ 'cmd': 'cancelThread', 'thread': thread});
    return 0;
  },

  __pthread_detached_exit: function() {
    // Called at the end of pthread_exit (which occurs also when leaving the
    // thread main function) if an only if the thread is in a detached state.
    postMessage({ 'cmd': 'detachedExit' });
  },

  // Returns 0 on success, or one of the values -ETIMEDOUT, -EWOULDBLOCK or -EINVAL on error.
  _emscripten_futex_wait_non_blocking__deps: ['emscripten_main_thread_process_queued_calls'],
  _emscripten_futex_wait_non_blocking: function(addr, val, timeout) {
#if ASSERTIONS
    // Should only be called from the main web thread where atomics.wait is not allowed.
    assert(ENVIRONMENT_IS_WEB);
#endif

    // Atomics.wait is not available in the main browser thread, so simulate it via busy spinning.
    var tNow = performance.now();
    var tEnd = tNow + timeout;

    // Register globally which address the main thread is simulating to be
    // waiting on. When zero, the main thread is not waiting on anything, and on
    // nonzero, the contents of the address pointed by __emscripten_main_thread_futex
    // tell which address the main thread is simulating its wait on.
    // We need to be careful of recursion here: If we wait on a futex, and
    // then call _emscripten_main_thread_process_queued_calls() below, that
    // will call code that takes the proxying mutex - which can once more
    // reach this code in a nested call. To avoid interference between the
    // two (there is just a single __emscripten_main_thread_futex at a time), unmark
    // ourselves before calling the potentially-recursive call. See below for
    // how we handle the case of our futex being notified during the time in
    // between when we are not set as the value of __emscripten_main_thread_futex.
#if ASSERTIONS
    assert(__emscripten_main_thread_futex > 0);
#endif
    var lastAddr = Atomics.exchange(HEAP32, __emscripten_main_thread_futex >> 2, addr);
#if ASSERTIONS
    // We must not have already been waiting.
    assert(lastAddr == 0);
#endif

    while (1) {
      // Check for a timeout.
      tNow = performance.now();
      if (tNow > tEnd) {
        // We timed out, so stop marking ourselves as waiting.
        lastAddr = Atomics.exchange(HEAP32, __emscripten_main_thread_futex >> 2, 0);
#if ASSERTIONS
        // The current value must have been our address which we set, or
        // in a race it was set to 0 which means another thread just allowed
        // us to run, but (tragically) that happened just a bit too late.
        assert(lastAddr == addr || lastAddr == 0);
#endif
        return -{{{ cDefine('ETIMEDOUT') }}};
      }
      // We are performing a blocking loop here, so we must handle proxied
      // events from pthreads, to avoid deadlocks.
      // Note that we have to do so carefully, as we may take a lock while
      // doing so, which can recurse into this function; stop marking
      // ourselves as waiting while we do so.
      lastAddr = Atomics.exchange(HEAP32, __emscripten_main_thread_futex >> 2, 0);
#if ASSERTIONS
      assert(lastAddr == addr || lastAddr == 0);
#endif
      if (lastAddr == 0) {
        // We were told to stop waiting, so stop.
        break;
      }
      _emscripten_main_thread_process_queued_calls();

      // Check the value, as if we were starting the futex all over again.
      // This handles the following case:
      //
      //  * wait on futex A
      //  * recurse into emscripten_main_thread_process_queued_calls(),
      //    which waits on futex B. that sets the __emscripten_main_thread_futex address to
      //    futex B, and there is no longer any mention of futex A.
      //  * a worker is done with futex A. it checks __emscripten_main_thread_futex but does
      //    not see A, so it does nothing special for the main thread.
      //  * a worker is done with futex B. it flips mainThreadMutex from B
      //    to 0, ending the wait on futex B.
      //  * we return to the wait on futex A. __emscripten_main_thread_futex is 0, but that
      //    is because of futex B being done - we can't tell from
      //    __emscripten_main_thread_futex whether A is done or not. therefore, check the
      //    memory value of the futex.
      //
      // That case motivates the design here. Given that, checking the memory
      // address is also necessary for other reasons: we unset and re-set our
      // address in __emscripten_main_thread_futex around calls to
      // emscripten_main_thread_process_queued_calls(), and a worker could
      // attempt to wake us up right before/after such times.
      //
      // Note that checking the memory value of the futex is valid to do: we
      // could easily have been delayed (relative to the worker holding on
      // to futex A), which means we could be starting all of our work at the
      // later time when there is no need to block. The only "odd" thing is
      // that we may have caused side effects in that "delay" time. But the
      // only side effects we can have are to call
      // emscripten_main_thread_process_queued_calls(). That is always ok to
      // do on the main thread (it's why it is ok for us to call it in the
      // middle of this function, and elsewhere). So if we check the value
      // here and return, it's the same is if what happened on the main thread
      // was the same as calling emscripten_main_thread_process_queued_calls()
      // a few times times before calling emscripten_futex_wait().
      if (Atomics.load(HEAP32, addr >> 2) != val) {
        return -{{{ cDefine('EWOULDBLOCK') }}};
      }

      // Mark us as waiting once more, and continue the loop.
      lastAddr = Atomics.exchange(HEAP32, __emscripten_main_thread_futex >> 2, addr);
#if ASSERTIONS
      assert(lastAddr == 0);
#endif
    }
    return 0;
  },

  // Returns the number of threads (>= 0) woken up, or the value -EINVAL on error.
  // Pass count == INT_MAX to wake up all threads.
  emscripten_futex_wake: function(addr, count) {
    if (addr <= 0 || addr > HEAP8.length || addr&3 != 0 || count < 0) return -{{{ cDefine('EINVAL') }}};
    if (count == 0) return 0;
    // Waking (at least) INT_MAX waiters is defined to mean wake all callers.
    // For Atomics.notify() API Infinity is to be passed in that case.
    if (count >= {{{ cDefine('INT_MAX') }}}) count = Infinity;

    // See if main thread is waiting on this address? If so, wake it up by resetting its wake location to zero.
    // Note that this is not a fair procedure, since we always wake main thread first before any workers, so
    // this scheme does not adhere to real queue-based waiting.
    var mainThreadWaitAddress = Atomics.load(HEAP32, __emscripten_main_thread_futex >> 2);
    var mainThreadWoken = 0;
    if (mainThreadWaitAddress == addr) {
#if ASSERTIONS
      // We only use __emscripten_main_thread_futex on the main browser thread, where we
      // cannot block while we wait. Therefore we should only see it set from
      // other threads, and not on the main thread itself. In other words, the
      // main thread must never try to wake itself up!
      assert(!ENVIRONMENT_IS_WEB);
#endif
      var loadedAddr = Atomics.compareExchange(HEAP32, __emscripten_main_thread_futex >> 2, mainThreadWaitAddress, 0);
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

  __call_main__deps: ['exit', '$exitOnMainThread'],
  __call_main: function(argc, argv) {
    var returnCode = {{{ exportedAsmFunc('_main') }}}(argc, argv);
#if EXIT_RUNTIME
    if (!keepRuntimeAlive()) {
      // exitRuntime enabled, proxied main() finished in a pthread, shut down the process.
#if PTHREADS_DEBUG
      err('Proxied main thread 0x' + _pthread_self().toString(16) + ' finished with return code ' + returnCode + '. EXIT_RUNTIME=1 set, quitting process.');
#endif
      exitOnMainThread(returnCode);
    }
#else
    // EXIT_RUNTIME==0 set on command line, keeping main thread alive.
#if PTHREADS_DEBUG
    err('Proxied main thread 0x' + _pthread_self().toString(16) + ' finished with return code ' + returnCode + '. EXIT_RUNTIME=0 set, so keeping main thread alive for asynchronous event operations.');
#endif
#endif
  },

#if ASSERTIONS
  emscripten_conditional_set_current_thread_status__sig: 'vii',
  emscripten_conditional_set_current_thread_status: function(expectedStatus, newStatus) {
#if PTHREADS_PROFILING
    PThread.setThreadStatusConditional(_pthread_self(), expectedStatus, newStatus);
#endif
  },

  emscripten_set_current_thread_status__sig: 'vi',
  emscripten_set_current_thread_status: function(newStatus) {
#if PTHREADS_PROFILING
    PThread.setThreadStatus(_pthread_self(), newStatus);
#endif
  },

  emscripten_set_thread_name__sig: 'vii',
  emscripten_set_thread_name: function(threadId, name) {
#if PTHREADS_PROFILING
    PThread.setThreadName(threadId, UTF8ToString(name));
#endif
  },
#endif

  // This function is call by a pthread to signal that exit() was called and
  // that the entire process should exit.
  // This function is always called from a pthread, but is executed on the
  // main thread due the __proxy attribute.
  $exitOnMainThread__deps: ['exit',
#if !MINIMAL_RUNTIME
    '$handleException',
#endif
  ],
  $exitOnMainThread__proxy: 'async',
  $exitOnMainThread: function(returnCode) {
#if PTHREADS_DEBUG
    err('exitOnMainThread');
#endif
#if MINIMAL_RUNTIME
    _exit(returnCode);
#else
    try {
      _exit(returnCode);
    } catch (e) {
      handleException(e);
    }
#endif
  },

  emscripten_proxy_to_main_thread_js__deps: ['emscripten_run_in_main_runtime_thread_js'],
  emscripten_proxy_to_main_thread_js__docs: '/** @type{function(number, (number|boolean), ...(number|boolean))} */',
  emscripten_proxy_to_main_thread_js: function(index, sync) {
    // Additional arguments are passed after those two, which are the actual
    // function arguments.
    // The serialization buffer contains the number of call params, and then
    // all the args here.
    // We also pass 'sync' to C separately, since C needs to look at it.
    var numCallArgs = arguments.length - 2;
    var outerArgs = arguments;
#if ASSERTIONS
    if (numCallArgs > {{{ cDefine('EM_QUEUED_JS_CALL_MAX_ARGS') }}}-1) throw 'emscripten_proxy_to_main_thread_js: Too many arguments ' + numCallArgs + ' to proxied function idx=' + index + ', maximum supported is ' + ({{{ cDefine('EM_QUEUED_JS_CALL_MAX_ARGS') }}}-1) + '!';
#endif
    // Allocate a buffer, which will be copied by the C code.
    return withStackSave(function() {
      // First passed parameter specifies the number of arguments to the function.
      // When BigInt support is enabled, we must handle types in a more complex
      // way, detecting at runtime if a value is a BigInt or not (as we have no
      // type info here). To do that, add a "prefix" before each value that
      // indicates if it is a BigInt, which effectively doubles the number of
      // values we serialize for proxying. TODO: pack this?
      var serializedNumCallArgs = numCallArgs {{{ WASM_BIGINT ? "* 2" : "" }}};
      var args = stackAlloc(serializedNumCallArgs * 8);
      var b = args >> 3;
      for (var i = 0; i < numCallArgs; i++) {
        var arg = outerArgs[2 + i];
  #if WASM_BIGINT
        if (typeof arg === 'bigint') {
          // The prefix is non-zero to indicate a bigint.
          HEAP64[b + 2*i] = BigInt(1);
          HEAP64[b + 2*i + 1] = arg;
        } else {
          // The prefix is zero to indicate a JS Number.
          HEAP64[b + 2*i] = BigInt(0);
          HEAPF64[b + 2*i + 1] = arg;
        }
  #else
        HEAPF64[b + i] = arg;
  #endif
      }
      return _emscripten_run_in_main_runtime_thread_js(index, serializedNumCallArgs, args, sync);
    });
  },

  emscripten_receive_on_main_thread_js_callArgs: '=[]',

  emscripten_receive_on_main_thread_js__deps: [
    'emscripten_proxy_to_main_thread_js',
    'emscripten_receive_on_main_thread_js_callArgs'],
  emscripten_receive_on_main_thread_js: function(index, numCallArgs, args) {
#if WASM_BIGINT
    numCallArgs /= 2;
#endif
    _emscripten_receive_on_main_thread_js_callArgs.length = numCallArgs;
    var b = args >> 3;
    for (var i = 0; i < numCallArgs; i++) {
#if WASM_BIGINT
      if (HEAP64[b + 2*i]) {
        // It's a BigInt.
        _emscripten_receive_on_main_thread_js_callArgs[i] = HEAP64[b + 2*i + 1];
      } else {
        // It's a Number.
        _emscripten_receive_on_main_thread_js_callArgs[i] = HEAPF64[b + 2*i + 1];
      }
#else
      _emscripten_receive_on_main_thread_js_callArgs[i] = HEAPF64[b + i];
#endif
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

  // TODO(sbc): Do we really need this to be dynamically settable from JS like this?
  // See https://github.com/emscripten-core/emscripten/issues/15101.
  _emscripten_default_pthread_stack_size: function() {
    return {{{ DEFAULT_PTHREAD_STACK_SIZE }}};
  },

  $establishStackSpace__internal: true,
  $establishStackSpace: function() {
    var pthread_ptr = _pthread_self();
    var stackTop = {{{ makeGetValue('pthread_ptr', C_STRUCTS.pthread.stack, 'i32') }}};
    var stackSize = {{{ makeGetValue('pthread_ptr', C_STRUCTS.pthread.stack_size, 'i32') }}};
    var stackMax = stackTop - stackSize;
#if ASSERTIONS
    assert(stackTop != 0);
    assert(stackMax != 0);
    assert(stackTop > stackMax);
#endif
    // Set stack limits used by `emscripten/stack.h` function.  These limits are
    // cached in wasm-side globals to make checks as fast as possible.
    _emscripten_stack_set_limits(stackTop, stackMax);
#if STACK_OVERFLOW_CHECK >= 2
    // Set stack limits used by binaryen's `StackCheck` pass.
    // TODO(sbc): Can this be combined with the above.
    ___set_stack_limits(stackTop, stackMax);
#endif

    // Call inside wasm module to set up the stack frame for this pthread in wasm module scope
    stackRestore(stackTop);

#if STACK_OVERFLOW_CHECK
    // Write the stack cookie last, after we have set up the proper bounds and
    // current position of the stack.
    writeStackCookie();
#endif
  },

  $invokeEntryPoint: function(ptr, arg) {
    return {{{ makeDynCall('ii', 'ptr') }}}(arg);
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
