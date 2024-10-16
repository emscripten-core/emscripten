/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Because only modern JS engines support SAB we can use modern JS language
 * features within this file (ES2020).
 */

#if !PTHREADS
#error "Internal error! PTHREADS should be enabled when including library_pthread.js."
#endif
#if !SHARED_MEMORY
#error "Internal error! SHARED_MEMORY should be enabled when including library_pthread.js."
#endif
#if PTHREADS == 2
#error "PTHREADS=2 is no longer supported"
#endif
#if BUILD_AS_WORKER
#error "pthreads + BUILD_AS_WORKER require separate modes that don't work together, see https://github.com/emscripten-core/emscripten/issues/8854"
#endif
#if EVAL_CTORS
#error "EVAL_CTORS is not compatible with pthreads yet (passive segments)"
#endif

{{{
#if MEMORY64
globalThis.MAX_PTR = Number((2n ** 64n) - 1n);
#else
globalThis.MAX_PTR = (2 ** 32) - 1
#endif
}}}

var LibraryPThread = {
  $PThread__postset: 'PThread.init();',
  $PThread__deps: ['_emscripten_thread_init',
                   '$terminateWorker',
                   '$cleanupThread',
#if MAIN_MODULE
                   '$markAsFinished',
#endif
                   '$spawnThread',
                   '_emscripten_thread_free_data',
                   'exit',
#if PTHREADS_DEBUG || ASSERTIONS
                   '$ptrToString',
#endif
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
    // Maps pthread_t pointers to the workers on which they are running.  For
    // the reverse mapping, each worker has a `pthread_ptr` when its running a
    // pthread.
    pthreads: {},
#if ASSERTIONS
    nextWorkerID: 1,
    debugInit() {
      function pthreadLogPrefix() {
        var t = 0;
        if (runtimeInitialized && typeof _pthread_self != 'undefined'
#if EXIT_RUNTIME
        && !runtimeExited
#endif
        ) {
          t = _pthread_self();
        }
        return `w:${workerID},t:${ptrToString(t)}: `;
      }

      // Prefix all err()/dbg() messages with the calling thread ID.
      var origDbg = dbg;
      dbg = (...args) => origDbg(pthreadLogPrefix() + args.join(' '));
#if PTHREADS_DEBUG
      // With PTHREADS_DEBUG also prefix all err() messages.
      var origErr = err;
      err = (...args) => origErr(pthreadLogPrefix() + args.join(' '));
#endif
    },
#endif
    init() {
#if ASSERTIONS
      PThread.debugInit();
#endif
      if ({{{ ENVIRONMENT_IS_MAIN_THREAD() }}}) {
        PThread.initMainThread();
      }
    },
    initMainThread() {
#if PTHREAD_POOL_SIZE
      var pthreadPoolSize = {{{ PTHREAD_POOL_SIZE }}};
      // Start loading up the Worker pool, if requested.
      while (pthreadPoolSize--) {
        PThread.allocateUnusedWorker();
      }
#endif
#if !MINIMAL_RUNTIME
      // MINIMAL_RUNTIME takes care of calling loadWasmModuleToAllWorkers
      // in postamble_minimal.js
      addOnPreRun(() => {
        addRunDependency('loading-workers')
        PThread.loadWasmModuleToAllWorkers(() => removeRunDependency('loading-workers'));
      });
#endif
#if MAIN_MODULE
      PThread.outstandingPromises = {};
      // Finished threads are threads that have finished running but we not yet
      // joined.
      PThread.finishedThreads = new Set();
#endif
    },

#if PTHREADS_PROFILING
    getThreadName(pthreadPtr) {
      var profilerBlock = {{{ makeGetValue('pthreadPtr', C_STRUCTS.pthread.profilerBlock, '*') }}};
      if (!profilerBlock) return "";
      return UTF8ToString(profilerBlock + {{{ C_STRUCTS.thread_profiler_block.name }}});
    },

    threadStatusToString(threadStatus) {
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

    threadStatusAsString(pthreadPtr) {
      var profilerBlock = {{{ makeGetValue('pthreadPtr', C_STRUCTS.pthread.profilerBlock, '*') }}};
      var status = (profilerBlock == 0) ? 0 : Atomics.load(HEAPU32, {{{ getHeapOffset('profilerBlock + ' + C_STRUCTS.thread_profiler_block.threadStatus, 'i32') }}});
      return PThread.threadStatusToString(status);
    },
#endif

    terminateAllThreads: () => {
#if ASSERTIONS
      assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! terminateAllThreads() can only ever be called from main application thread!');
#endif
#if PTHREADS_DEBUG
      dbg('terminateAllThreads');
#endif
      // Attempt to kill all workers.  Sadly (at least on the web) there is no
      // way to terminate a worker synchronously, or to be notified when a
      // worker in actually terminated.  This means there is some risk that
      // pthreads will continue to be executing after `worker.terminate` has
      // returned.  For this reason, we don't call `returnWorkerToPool` here or
      // free the underlying pthread data structures.
      for (var worker of PThread.runningWorkers) {
        terminateWorker(worker);
      }
      for (var worker of PThread.unusedWorkers) {
        terminateWorker(worker);
      }
      PThread.unusedWorkers = [];
      PThread.runningWorkers = [];
      PThread.pthreads = [];
    },
    returnWorkerToPool: (worker) => {
      // We don't want to run main thread queued calls here, since we are doing
      // some operations that leave the worker queue in an invalid state until
      // we are completely done (it would be bad if free() ends up calling a
      // queued pthread_create which looks at the global data structures we are
      // modifying). To achieve that, defer the free() til the very end, when
      // we are all done.
      var pthread_ptr = worker.pthread_ptr;
      delete PThread.pthreads[pthread_ptr];
      // Note: worker is intentionally not terminated so the pool can
      // dynamically grow.
      PThread.unusedWorkers.push(worker);
      PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker), 1);
      // Not a running Worker anymore
      // Detach the worker from the pthread object, and return it to the
      // worker pool as an unused worker.
      worker.pthread_ptr = 0;

#if ENVIRONMENT_MAY_BE_NODE && PROXY_TO_PTHREAD
      if (ENVIRONMENT_IS_NODE) {
        // Once the proxied main thread has finished, mark it as weakly
        // referenced so that its existence does not prevent Node.js from
        // exiting.  This has no effect if the worker is already weakly
        // referenced.
        worker.unref();
      }
#endif

      // Finally, free the underlying (and now-unused) pthread structure in
      // linear memory.
      __emscripten_thread_free_data(pthread_ptr);
    },
    receiveObjectTransfer(data) {
#if OFFSCREENCANVAS_SUPPORT
      if (typeof GL != 'undefined') {
        Object.assign(GL.offscreenCanvases, data.offscreenCanvases);
        if (!Module['canvas'] && data.moduleCanvasId && GL.offscreenCanvases[data.moduleCanvasId]) {
          Module['canvas'] = GL.offscreenCanvases[data.moduleCanvasId].offscreenCanvas;
          Module['canvas'].id = data.moduleCanvasId;
        }
      }
#endif
    },
    // Called by worker.js each time a thread is started.
    threadInitTLS() {
#if PTHREADS_DEBUG
      dbg('threadInitTLS');
#endif
      // Call thread init functions (these are the _emscripten_tls_init for each
      // module loaded.
      PThread.tlsInitFunctions.forEach((f) => f());
    },
    // Loads the WebAssembly module into the given Worker.
    // onFinishedLoading: A callback function that will be called once all of
    //                    the workers have been initialized and are
    //                    ready to host pthreads.
    loadWasmModuleToWorker: (worker) => new Promise((onFinishedLoading) => {
      worker.onmessage = (e) => {
        var d = e['data'];
        var cmd = d.cmd;

        // If this message is intended to a recipient that is not the main
        // thread, forward it to the target thread.
        if (d.targetThread && d.targetThread != _pthread_self()) {
          var targetWorker = PThread.pthreads[d.targetThread];
          if (targetWorker) {
            targetWorker.postMessage(d, d.transferList);
          } else {
            err(`Internal error! Worker sent a message "${cmd}" to target pthread ${d.targetThread}, but that thread no longer exists!`);
          }
          return;
        }

        if (cmd === 'checkMailbox') {
          checkMailbox();
        } else if (cmd === 'spawnThread') {
          spawnThread(d);
        } else if (cmd === 'cleanupThread') {
          cleanupThread(d.thread);
#if MAIN_MODULE
        } else if (cmd === 'markAsFinished') {
          markAsFinished(d.thread);
#endif
        } else if (cmd === 'loaded') {
          worker.loaded = true;
#if ENVIRONMENT_MAY_BE_NODE && PTHREAD_POOL_SIZE
          // Check that this worker doesn't have an associated pthread.
          if (ENVIRONMENT_IS_NODE && !worker.pthread_ptr) {
            // Once worker is loaded & idle, mark it as weakly referenced,
            // so that mere existence of a Worker in the pool does not prevent
            // Node.js from exiting the app.
            worker.unref();
          }
#endif
          onFinishedLoading(worker);
        } else if (cmd === 'alert') {
          alert(`Thread ${d.threadId}: ${d.text}`);
        } else if (d.target === 'setimmediate') {
          // Worker wants to postMessage() to itself to implement setImmediate()
          // emulation.
          worker.postMessage(d);
        } else if (cmd === 'callHandler') {
          Module[d.handler](...d.args);
        } else if (cmd) {
          // The received message looks like something that should be handled by this message
          // handler, (since there is a e.data.cmd field present), but is not one of the
          // recognized commands:
          err(`worker sent an unknown command ${cmd}`);
        }
      };

      worker.onerror = (e) => {
        var message = 'worker sent an error!';
#if ASSERTIONS
        if (worker.pthread_ptr) {
          message = `Pthread ${ptrToString(worker.pthread_ptr)} sent an error!`;
        }
#endif
        err(`${message} ${e.filename}:${e.lineno}: ${e.message}`);
        throw e;
      };

#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) {
        worker.on('message', (data) => worker.onmessage({ data: data }));
        worker.on('error', (e) => worker.onerror(e));
      }
#endif

#if ASSERTIONS
      assert(wasmMemory instanceof WebAssembly.Memory, 'WebAssembly memory should have been loaded by now!');
      assert(wasmModule instanceof WebAssembly.Module, 'WebAssembly Module should have been loaded by now!');
#endif

      // When running on a pthread, none of the incoming parameters on the module
      // object are present. Proxy known handlers back to the main thread if specified.
      var handlers = [];
      var knownHandlers = [
#if expectToReceiveOnModule('onExit')
        'onExit',
#endif
#if expectToReceiveOnModule('onAbort')
        'onAbort',
#endif
#if expectToReceiveOnModule('print')
        'print',
#endif
#if expectToReceiveOnModule('printErr')
        'printErr',
#endif
      ];
      for (var handler of knownHandlers) {
        if (Module.propertyIsEnumerable(handler)) {
          handlers.push(handler);
        }
      }

#if ASSERTIONS
      worker.workerID = PThread.nextWorkerID++;
#endif

      // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
      worker.postMessage({
        cmd: 'load',
        handlers: handlers,
#if WASM2JS
        // the polyfill WebAssembly.Memory instance has function properties,
        // which will fail in postMessage, so just send a custom object with the
        // property we need, the buffer
        wasmMemory: { 'buffer': wasmMemory.buffer },
#else // WASM2JS
        wasmMemory,
#endif // WASM2JS
        wasmModule,
#if LOAD_SOURCE_MAP
        wasmSourceMap,
#endif
#if USE_OFFSET_CONVERTER
        wasmOffsetConverter,
#endif
#if MAIN_MODULE
        dynamicLibraries,
        // Share all modules that have been loaded so far.  New workers
        // won't start running threads until these are all loaded.
        sharedModules,
#endif
#if ASSERTIONS
        'workerID': worker.workerID,
#endif
      });
    }),

    loadWasmModuleToAllWorkers(onMaybeReady) {
#if !PTHREAD_POOL_SIZE
      onMaybeReady();
#else
      // Instantiation is synchronous in pthreads.
      if (
        ENVIRONMENT_IS_PTHREAD
#if WASM_WORKERS
        || ENVIRONMENT_IS_WASM_WORKER
#endif
      ) {
        return onMaybeReady();
      }

      let pthreadPoolReady = Promise.all(PThread.unusedWorkers.map(PThread.loadWasmModuleToWorker));
#if PTHREAD_POOL_DELAY_LOAD
      // PTHREAD_POOL_DELAY_LOAD means we want to proceed synchronously without
      // waiting for the pthread pool during the startup phase.
      // If the user wants to wait on it elsewhere, they can do so via the
      // Module['pthreadPoolReady'] promise.
      Module['pthreadPoolReady'] = pthreadPoolReady;
      onMaybeReady();
#else
      pthreadPoolReady.then(onMaybeReady);
#endif // PTHREAD_POOL_DELAY_LOAD
#endif // PTHREAD_POOL_SIZE
    },

    // Creates a new web Worker and places it in the unused worker pool to wait for its use.
    allocateUnusedWorker() {
      var worker;
      var workerOptions = {
#if EXPORT_ES6
        'type': 'module',
#endif
#if ENVIRONMENT_MAY_BE_NODE
        // This is the way that we signal to the node worker that it is hosting
        // a pthread.
        'workerData': 'em-pthread',
#endif
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
        // This is the way that we signal to the Web Worker that it is hosting
        // a pthread.
#if ASSERTIONS
        'name': 'em-pthread-' + PThread.nextWorkerID,
#else
        'name': 'em-pthread',
#endif
#endif
      };
#if EXPORT_ES6 && USE_ES6_IMPORT_META
      // If we're using module output, use bundler-friendly pattern.
#if PTHREADS_DEBUG
      dbg(`Allocating a new web worker from ${import.meta.url}`);
#endif
#if TRUSTED_TYPES
      // Use Trusted Types compatible wrappers.
      if (typeof trustedTypes != 'undefined' && trustedTypes.createPolicy) {
        var p = trustedTypes.createPolicy(
          'emscripten#workerPolicy1',
          {
            createScriptURL: (ignored) => new URL("{{{ TARGET_JS_NAME }}}", import.meta.url)
          }
        );
        worker = new Worker(p.createScriptURL('ignored'), workerOptions);
      } else
#endif
      // We need to generate the URL with import.meta.url as the base URL of the JS file
      // instead of just using new URL(import.meta.url) because bundler's only recognize
      // the first case in their bundling step. The latter ends up producing an invalid
      // URL to import from the server (e.g., for webpack the file:// path).
      worker = new Worker(new URL('{{{ TARGET_JS_NAME }}}', import.meta.url), workerOptions);
#else
      var pthreadMainJs = _scriptName;
#if expectToReceiveOnModule('mainScriptUrlOrBlob')
      // We can't use makeModuleReceiveWithVar here since we want to also
      // call URL.createObjectURL on the mainScriptUrlOrBlob.
      if (Module['mainScriptUrlOrBlob']) {
        pthreadMainJs = Module['mainScriptUrlOrBlob'];
        if (typeof pthreadMainJs != 'string') {
          pthreadMainJs = URL.createObjectURL(pthreadMainJs);
        }
      }
#endif
#if PTHREADS_DEBUG
      dbg(`Allocating a new web worker from ${pthreadMainJs}`);
#endif
#if TRUSTED_TYPES
      // Use Trusted Types compatible wrappers.
      if (typeof trustedTypes != 'undefined' && trustedTypes.createPolicy) {
        var p = trustedTypes.createPolicy('emscripten#workerPolicy2', { createScriptURL: (ignored) => pthreadMainJs });
        worker = new Worker(p.createScriptURL('ignored'), workerOptions);
      } else
#endif
      worker = new Worker(pthreadMainJs, workerOptions);
#endif // EXPORT_ES6 && USE_ES6_IMPORT_META
      PThread.unusedWorkers.push(worker);
    },

    getNewWorker() {
      if (PThread.unusedWorkers.length == 0) {
// PTHREAD_POOL_SIZE_STRICT should show a warning and, if set to level `2`, return from the function.
#if (PTHREAD_POOL_SIZE_STRICT && ASSERTIONS) || PTHREAD_POOL_SIZE_STRICT == 2
// However, if we're in Node.js, then we can create new workers on the fly and PTHREAD_POOL_SIZE_STRICT
// should be ignored altogether.
#if ENVIRONMENT_MAY_BE_NODE
        if (!ENVIRONMENT_IS_NODE) {
#endif
#if ASSERTIONS
            err('Tried to spawn a new thread, but the thread pool is exhausted.\n' +
            'This might result in a deadlock unless some threads eventually exit or the code explicitly breaks out to the event loop.\n' +
            'If you want to increase the pool size, use setting `-sPTHREAD_POOL_SIZE=...`.'
#if PTHREAD_POOL_SIZE_STRICT == 1
              + '\nIf you want to throw an explicit error instead of the risk of deadlocking in those cases, use setting `-sPTHREAD_POOL_SIZE_STRICT=2`.'
#endif
            );
#endif // ASSERTIONS
#if PTHREAD_POOL_SIZE_STRICT == 2
            return;
#endif
#if ENVIRONMENT_MAY_BE_NODE
        }
#endif
#endif // PTHREAD_POOL_SIZE_STRICT
#if PTHREAD_POOL_SIZE_STRICT < 2 || ENVIRONMENT_MAY_BE_NODE
        PThread.allocateUnusedWorker();
        PThread.loadWasmModuleToWorker(PThread.unusedWorkers[0]);
#endif
      }
      return PThread.unusedWorkers.pop();
    }
  },

  $terminateWorker: (worker) => {
#if PTHREADS_DEBUG
    dbg(`terminateWorker: ${worker.workerID}`);
#endif
    worker.terminate();
    // terminate() can be asynchronous, so in theory the worker can continue
    // to run for some amount of time after termination.  However from our POV
    // the worker now dead and we don't want to hear from it again, so we stub
    // out its message handler here.  This avoids having to check in each of
    // the onmessage handlers if the message was coming from valid worker.
    worker.onmessage = (e) => {
#if ASSERTIONS
      var cmd = e['data'].cmd;
      err(`received "${cmd}" command from terminated worker: ${worker.workerID}`);
#endif
    };
  },

  _emscripten_thread_cleanup: (thread) => {
    // Called when a thread needs to be cleaned up so it can be reused.
    // A thread is considered reusable when it either returns from its
    // entry point, calls pthread_exit, or acts upon a cancellation.
    // Detached threads are responsible for calling this themselves,
    // otherwise pthread_join is responsible for calling this.
#if PTHREADS_DEBUG
    dbg(`_emscripten_thread_cleanup: ${ptrToString(thread)}`)
#endif
    if (!ENVIRONMENT_IS_PTHREAD) cleanupThread(thread);
    else postMessage({ cmd: 'cleanupThread', thread });
  },

  _emscripten_thread_set_strongref: (thread) => {
    // Called when a thread needs to be strongly referenced.
    // Currently only used for:
    // - keeping the "main" thread alive in PROXY_TO_PTHREAD mode;
    // - crashed threads that needs to propagate the uncaught exception
    //   back to the main thread.
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      PThread.pthreads[thread].ref();
    }
#endif
  },

  $cleanupThread: (pthread_ptr) => {
#if PTHREADS_DEBUG
    dbg(`cleanupThread: ${ptrToString(pthread_ptr)}`)
#endif
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! cleanupThread() can only ever be called from main application thread!');
    assert(pthread_ptr, 'Internal Error! Null pthread_ptr in cleanupThread!');
#endif
    var worker = PThread.pthreads[pthread_ptr];
#if MAIN_MODULE
    PThread.finishedThreads.delete(pthread_ptr);
    if (pthread_ptr in PThread.outstandingPromises) {
      PThread.outstandingPromises[pthread_ptr].resolve();
    }
#endif
#if ASSERTIONS
    assert(worker);
#endif
    PThread.returnWorkerToPool(worker);
  },

#if MAIN_MODULE
  $registerTLSInit: (tlsInitFunc, moduleExports, metadata) => {
#if DYLINK_DEBUG
    dbg("registerTLSInit: " + tlsInitFunc);
#endif
    // In relocatable builds, we use the result of calling tlsInitFunc
    // (`_emscripten_tls_init`) to relocate the TLS exports of the module
    // according to this new __tls_base.
    function tlsInitWrapper() {
      var __tls_base = tlsInitFunc();
#if DYLINK_DEBUG
      dbg(`tlsInit -> ${__tls_base}`);
#endif
      if (!__tls_base) {
#if ASSERTIONS
        // __tls_base should never be zero if there are tls exports
        assert(__tls_base || metadata.tlsExports.size == 0);
#endif
        return;
      }
      var tlsExports = {};
      metadata.tlsExports.forEach((s) => tlsExports[s] = moduleExports[s]);
      relocateExports(tlsExports, __tls_base, /*replace=*/true);
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
  },
#else
  $registerTLSInit: (tlsInitFunc) => PThread.tlsInitFunctions.push(tlsInitFunc),
#endif

  $spawnThread: (threadParams) => {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! spawnThread() can only ever be called from main application thread!');
    assert(threadParams.pthread_ptr, 'Internal error, no pthread ptr!');
#endif

    var worker = PThread.getNewWorker();
    if (!worker) {
      // No available workers in the PThread pool.
      return {{{ cDefs.EAGAIN }}};
    }
#if ASSERTIONS
    assert(!worker.pthread_ptr, 'Internal error!');
#endif

    PThread.runningWorkers.push(worker);

    // Add to pthreads map
    PThread.pthreads[threadParams.pthread_ptr] = worker;

    worker.pthread_ptr = threadParams.pthread_ptr;
    var msg = {
        cmd: 'run',
        start_routine: threadParams.startRoutine,
        arg: threadParams.arg,
        pthread_ptr: threadParams.pthread_ptr,
    };
#if OFFSCREENCANVAS_SUPPORT
    // Note that we do not need to quote these names because they are only used
    // in this file, and not from the external worker.js.
    msg.moduleCanvasId = threadParams.moduleCanvasId;
    msg.offscreenCanvases = threadParams.offscreenCanvases;
#endif
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      // Mark worker as weakly referenced once we start executing a pthread,
      // so that its existence does not prevent Node.js from exiting.  This
      // has no effect if the worker is already weakly referenced (e.g. if
      // this worker was previously idle/unused).
      worker.unref();
    }
#endif
    // Ask the worker to start executing its pthread entry point function.
    worker.postMessage(msg, threadParams.transferList);
    return 0;
  },

  _emscripten_init_main_thread_js: (tb) => {
    // Pass the thread address to the native code where they stored in wasm
    // globals which act as a form of TLS. Global constructors trying
    // to access this value will read the wrong value, but that is UB anyway.
    __emscripten_thread_init(
      tb,
      /*is_main=*/!ENVIRONMENT_IS_WORKER,
      /*is_runtime=*/1,
      /*can_block=*/!ENVIRONMENT_IS_WEB,
      /*default_stacksize=*/{{{ DEFAULT_PTHREAD_STACK_SIZE }}},
#if PTHREADS_PROFILING
      /*start_profiling=*/true,
#else
      /*start_profiling=*/false,
#endif
    );
    PThread.threadInitTLS();
  },

  $pthreadCreateProxied__internal: true,
  $pthreadCreateProxied__proxy: 'sync',
  $pthreadCreateProxied__deps: ['__pthread_create_js'],
  $pthreadCreateProxied: (pthread_ptr, attr, startRoutine, arg) => ___pthread_create_js(pthread_ptr, attr, startRoutine, arg),

#if OFFSCREENCANVAS_SUPPORT
  // ASan wraps the emscripten_builtin_pthread_create call in
  // __lsan::ScopedInterceptorDisabler.  Unfortunately, that only disables it on
  // the thread that made the call.  __pthread_create_js gets proxied to the
  // main thread, where LSan is not disabled. This makes it necessary for us to
  // disable LSan here (using __noleakcheck), so that it does not detect
  // pthread's internal allocations as leaks.  If/when we remove all the
  // allocations from __pthread_create_js we could also remove this.
  __pthread_create_js__noleakcheck: true,
#endif
  __pthread_create_js__deps: ['$spawnThread', 'pthread_self', '$pthreadCreateProxied',
    'emscripten_has_threading_support',
#if OFFSCREENCANVAS_SUPPORT
    'malloc',
#endif
  ],
  __pthread_create_js: (pthread_ptr, attr, startRoutine, arg) => {
    if (!_emscripten_has_threading_support()) {
#if ASSERTIONS
      dbg('pthread_create: environment does not support SharedArrayBuffer, pthreads are not available');
#endif
      return {{{ cDefs.EAGAIN }}};
    }
#if PTHREADS_DEBUG
    dbg("createThread: " + ptrToString(pthread_ptr));
#endif

    // List of JS objects that will transfer ownership to the Worker hosting the thread
    var transferList = [];
    var error = 0;

#if OFFSCREENCANVAS_SUPPORT
    // Deduce which WebGL canvases (HTMLCanvasElements or OffscreenCanvases) should be passed over to the
    // Worker that hosts the spawned pthread.
    // Comma-delimited list of CSS selectors that must identify canvases by IDs: "#canvas1, #canvas2, ..."
    var transferredCanvasNames = attr ? {{{ makeGetValue('attr', C_STRUCTS.pthread_attr_t._a_transferredcanvases, '*') }}} : 0;
#if OFFSCREENCANVASES_TO_PTHREAD
    // Proxied canvases string pointer -1/MAX_PTR is used as a special token to
    // fetch whatever canvases were passed to build in
    // -sOFFSCREENCANVASES_TO_PTHREAD= command line.
    if (transferredCanvasNames == {{{ MAX_PTR }}}) {
      transferredCanvasNames = '{{{ OFFSCREENCANVASES_TO_PTHREAD }}}';
    } else
#endif
    {
      transferredCanvasNames = UTF8ToString(transferredCanvasNames).trim();
    }
    transferredCanvasNames = transferredCanvasNames ? transferredCanvasNames.split(',') : [];
#if GL_DEBUG
    dbg(`pthread_create: transferredCanvasNames="${transferredCanvasNames}"`);
#endif

    var offscreenCanvases = {}; // Dictionary of OffscreenCanvas objects we'll transfer to the created thread to own
    var moduleCanvasId = Module['canvas']?.id || '';
    // Note that transferredCanvasNames might be null (so we cannot do a for-of loop).
    for (var name of transferredCanvasNames) {
      name = name.trim();
      var offscreenCanvasInfo;
      try {
        if (name == '#canvas') {
          if (!Module['canvas']) {
            err(`pthread_create: could not find canvas with ID "${name}" to transfer to thread!`);
            error = {{{ cDefs.EINVAL }}};
            break;
          }
          name = Module['canvas'].id;
        }
#if ASSERTIONS
        assert(typeof GL == 'object', 'OFFSCREENCANVAS_SUPPORT assumes GL is in use (you can force-include it with \'-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$GL\')');
#endif
        if (GL.offscreenCanvases[name]) {
          offscreenCanvasInfo = GL.offscreenCanvases[name];
          GL.offscreenCanvases[name] = null; // This thread no longer owns this canvas.
          if (Module['canvas'] instanceof OffscreenCanvas && name === Module['canvas'].id) Module['canvas'] = null;
        } else if (!ENVIRONMENT_IS_PTHREAD) {
          var canvas = (Module['canvas'] && Module['canvas'].id === name) ? Module['canvas'] : document.querySelector(name);
          if (!canvas) {
            err(`pthread_create: could not find canvas with ID "${name}" to transfer to thread!`);
            error = {{{ cDefs.EINVAL }}};
            break;
          }
          if (canvas.controlTransferredOffscreen) {
            err(`pthread_create: cannot transfer canvas with ID "${name}" to thread, since the current thread does not have control over it!`);
            error = {{{ cDefs.EPERM }}}; // Operation not permitted, some other thread is accessing the canvas.
            break;
          }
          if (canvas.transferControlToOffscreen) {
#if GL_DEBUG
            dbg(`pthread_create: canvas.transferControlToOffscreen(), transferring canvas by name "${name}" (DOM id="${canvas.id}") from main thread to pthread`);
#endif
            // Create a shared information block in heap so that we can control
            // the canvas size from any thread.
            if (!canvas.canvasSharedPtr) {
              canvas.canvasSharedPtr = _malloc({{{ 8 + POINTER_SIZE }}});
              {{{ makeSetValue('canvas.canvasSharedPtr', 0, 'canvas.width', 'i32') }}};
              {{{ makeSetValue('canvas.canvasSharedPtr', 4, 'canvas.height', 'i32') }}};
              {{{ makeSetValue('canvas.canvasSharedPtr', 8, 0, '*') }}}; // pthread ptr to the thread that owns this canvas, filled in below.
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
            err(`pthread_create: cannot transfer control of canvas "${name}" to pthread, because current browser does not support OffscreenCanvas!`);
            // If building with OFFSCREEN_FRAMEBUFFER=1 mode, we don't need to
            // be able to transfer control to offscreen, but WebGL can be
            // proxied from worker to main thread.
#if !OFFSCREEN_FRAMEBUFFER
            err('pthread_create: Build with -sOFFSCREEN_FRAMEBUFFER to enable fallback proxying of GL commands from pthread to main thread.');
            return {{{ cDefs.ENOSYS }}}; // Function not implemented, browser doesn't have support for this.
#endif
          }
        }
        if (offscreenCanvasInfo) {
          transferList.push(offscreenCanvasInfo.offscreenCanvas);
          offscreenCanvases[offscreenCanvasInfo.id] = offscreenCanvasInfo;
        }
      } catch(e) {
        err(`pthread_create: failed to transfer control of canvas "${name}" to OffscreenCanvas! Error: ${e}`);
        return {{{ cDefs.EINVAL }}}; // Hitting this might indicate an implementation bug or some other internal error
      }
    }
#endif // OFFSCREENCANVAS_SUPPORT

    // Synchronously proxy the thread creation to main thread if possible. If we
    // need to transfer ownership of objects, then proxy asynchronously via
    // postMessage.
    if (ENVIRONMENT_IS_PTHREAD && (transferList.length === 0 || error)) {
      return pthreadCreateProxied(pthread_ptr, attr, startRoutine, arg);
    }

    // If on the main thread, and accessing Canvas/OffscreenCanvas failed, abort
    // with the detected error.
    if (error) return error;

#if OFFSCREENCANVAS_SUPPORT
    // Register for each of the transferred canvases that the new thread now
    // owns the OffscreenCanvas.
    for (var canvas of Object.values(offscreenCanvases)) {
      // pthread ptr to the thread that owns this canvas.
      {{{ makeSetValue('canvas.canvasSharedPtr', 8, 'pthread_ptr', '*') }}};
    }
#endif

    var threadParams = {
      startRoutine,
      pthread_ptr,
      arg,
#if OFFSCREENCANVAS_SUPPORT
      moduleCanvasId,
      offscreenCanvases,
#endif
      transferList,
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

  emscripten_check_blocking_allowed__deps: ['$warnOnce'],
  emscripten_check_blocking_allowed: () => {
#if (ASSERTIONS || !ALLOW_BLOCKING_ON_MAIN_THREAD) && !MINIMAL_RUNTIME
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
  $exitOnMainThread: (returnCode) => {
#if PTHREADS_DEBUG
    dbg('exitOnMainThread');
#endif
#if PROXY_TO_PTHREAD
    {{{ runtimeKeepalivePop() }}};
#endif
    _exit(returnCode);
  },

#if MEMORY64
  // Calls proxyToMainThread but returns a bigint rather than a number
  $proxyToMainThreadPtr__deps: ['$proxyToMainThread'],
  $proxyToMainThreadPtr: (...args) => BigInt(proxyToMainThread(...args)),
#endif

  $proxyToMainThread__deps: ['$stackSave', '$stackRestore', '$stackAlloc', '_emscripten_run_on_main_thread_js', ...i53ConversionDeps],
  $proxyToMainThread__docs: '/** @type{function(number, (number|boolean), ...number)} */',
  $proxyToMainThread: (funcIndex, emAsmAddr, sync, ...callArgs) => {
    // EM_ASM proxying is done by passing a pointer to the address of the EM_ASM
    // content as `emAsmAddr`.  JS library proxying is done by passing an index
    // into `proxiedJSCallArgs` as `funcIndex`. If `emAsmAddr` is non-zero then
    // `funcIndex` will be ignored.
    // Additional arguments are passed after the first three are the actual
    // function arguments.
    // The serialization buffer contains the number of call params, and then
    // all the args here.
    // We also pass 'sync' to C separately, since C needs to look at it.
    // Allocate a buffer, which will be copied by the C code.
    //
    // First passed parameter specifies the number of arguments to the function.
    // When BigInt support is enabled, we must handle types in a more complex
    // way, detecting at runtime if a value is a BigInt or not (as we have no
    // type info here). To do that, add a "prefix" before each value that
    // indicates if it is a BigInt, which effectively doubles the number of
    // values we serialize for proxying. TODO: pack this?
    var serializedNumCallArgs = callArgs.length {{{ WASM_BIGINT ? "* 2" : "" }}};
    var sp = stackSave();
    var args = stackAlloc(serializedNumCallArgs * 8);
    var b = {{{ getHeapOffset('args', 'i64') }}};
    for (var i = 0; i < callArgs.length; i++) {
      var arg = callArgs[i];
#if WASM_BIGINT
      if (typeof arg == 'bigint') {
        // The prefix is non-zero to indicate a bigint.
        HEAP64[b + 2*i] = 1n;
        HEAP64[b + 2*i + 1] = arg;
      } else {
        // The prefix is zero to indicate a JS Number.
        HEAP64[b + 2*i] = 0n;
        HEAPF64[b + 2*i + 1] = arg;
      }
#else
      HEAPF64[b + i] = arg;
#endif
    }
    var rtn = __emscripten_run_on_main_thread_js(funcIndex, emAsmAddr, serializedNumCallArgs, args, sync);
    stackRestore(sp);
    return rtn;
  },

  // Reuse global JS array to avoid creating JS garbage for each proxied call
  $proxiedJSCallArgs: '=[]',

  _emscripten_receive_on_main_thread_js__deps: [
    '$proxyToMainThread',
    '$proxiedJSCallArgs'],
  _emscripten_receive_on_main_thread_js: (funcIndex, emAsmAddr, callingThread, numCallArgs, args) => {
    // Sometimes we need to backproxy events to the calling thread (e.g.
    // HTML5 DOM events handlers such as
    // emscripten_set_mousemove_callback()), so keep track in a globally
    // accessible variable about the thread that initiated the proxying.
#if WASM_BIGINT
    numCallArgs /= 2;
#endif
    proxiedJSCallArgs.length = numCallArgs;
    var b = {{{ getHeapOffset('args', 'i64') }}};
    for (var i = 0; i < numCallArgs; i++) {
#if WASM_BIGINT
      if (HEAP64[b + 2*i]) {
        // It's a BigInt.
        proxiedJSCallArgs[i] = HEAP64[b + 2*i + 1];
      } else {
        // It's a Number.
        proxiedJSCallArgs[i] = HEAPF64[b + 2*i + 1];
      }
#else
      proxiedJSCallArgs[i] = HEAPF64[b + i];
#endif
    }
    // Proxied JS library funcs use funcIndex and EM_ASM functions use emAsmAddr
#if HAVE_EM_ASM
    var func = emAsmAddr ? ASM_CONSTS[emAsmAddr] : proxiedFunctionTable[funcIndex];
#else
#if ASSERTIONS
    assert(!emAsmAddr);
#endif
    var func = proxiedFunctionTable[funcIndex];
#endif
#if ASSERTIONS
    assert(!(funcIndex && emAsmAddr));
    assert(func.length == numCallArgs, 'Call args mismatch in _emscripten_receive_on_main_thread_js');
#endif
    PThread.currentProxiedOperationCallerThread = callingThread;
    var rtn = func(...proxiedJSCallArgs);
    PThread.currentProxiedOperationCallerThread = 0;
#if MEMORY64
    // In memory64 mode some proxied functions return bigint/pointer but
    // our return type is i53/double.
    if (typeof rtn == "bigint") {
      rtn = bigintToI53Checked(rtn);
    }
#endif
#if ASSERTIONS
    // Proxied functions can return any type except bigint.  All other types
    // cooerce to f64/double (the return type of this function in C) but not
    // bigint.
    assert(typeof rtn != "bigint");
#endif
    return rtn;
  },

  $establishStackSpace__internal: true,
  $establishStackSpace__deps: ['$stackRestore'],
  $establishStackSpace: (pthread_ptr) => {
#if ALLOW_MEMORY_GROWTH
    // If memory growth is enabled, the memory views may have gotten out of date,
    // so resync them before accessing the pthread ptr below.
    updateMemoryViews();
#endif
    var stackHigh = {{{ makeGetValue('pthread_ptr', C_STRUCTS.pthread.stack, '*') }}};
    var stackSize = {{{ makeGetValue('pthread_ptr', C_STRUCTS.pthread.stack_size, '*') }}};
    var stackLow = stackHigh - stackSize;
#if PTHREADS_DEBUG
    dbg(`establishStackSpace: ${ptrToString(stackHigh)} -> ${ptrToString(stackLow)}`);
#endif
#if ASSERTIONS
    assert(stackHigh != 0);
    assert(stackLow != 0);
    assert(stackHigh > stackLow, 'stackHigh must be higher then stackLow');
#endif
    // Set stack limits used by `emscripten/stack.h` function.  These limits are
    // cached in wasm-side globals to make checks as fast as possible.
    _emscripten_stack_set_limits(stackHigh, stackLow);

#if STACK_OVERFLOW_CHECK >= 2
    setStackLimits();
#endif STACK_OVERFLOW_CHECK

    // Call inside wasm module to set up the stack frame for this pthread in wasm module scope
    stackRestore(stackHigh);

#if STACK_OVERFLOW_CHECK
    // Write the stack cookie last, after we have set up the proper bounds and
    // current position of the stack.
    writeStackCookie();
#endif
  },

  $invokeEntryPoint__deps: [
    '_emscripten_thread_exit',
#if !MINIMAL_RUNTIME
    '$keepRuntimeAlive',
    '$runtimeKeepaliveCounter',
#endif
  ],
  $invokeEntryPoint: (ptr, arg) => {
#if PTHREADS_DEBUG
    dbg(`invokeEntryPoint: ${ptrToString(ptr)}`);
#endif
#if !MINIMAL_RUNTIME
    // An old thread on this worker may have been canceled without returning the
    // `runtimeKeepaliveCounter` to zero. Reset it now so the new thread won't
    // be affected.
    runtimeKeepaliveCounter = 0;

#if isSymbolNeeded('$noExitRuntime')
    // Same for noExitRuntime.  The default for pthreads should always be false
    // otherwise pthreads would never complete and attempts to pthread_join to
    // them would block forever.
    // pthreads can still choose to set `noExitRuntime` explicitly, or
    // call emscripten_unwind_to_js_event_loop to extend their lifetime beyond
    // their main function.  See comment in src/runtime_pthread.js for more.
    noExitRuntime = 0;
#endif
#endif

#if MAIN_MODULE
    // Before we call the thread entry point, make sure any shared libraries
    // have been loaded on this there.  Otherwise our table might be not be
    // in sync and might not contain the function pointer `ptr` at all.
    __emscripten_dlsync_self();
#endif
    // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
    // Native codebases sometimes spawn threads with other thread entry point
    // signatures, such as void ThreadMain(void *arg), void *ThreadMain(), or
    // void ThreadMain().  That is not acceptable per C/C++ specification, but
    // x86 compiler ABI extensions enable that to work. If you find the
    // following line to crash, either change the signature to "proper" void
    // *ThreadMain(void *arg) form, or try linking with the Emscripten linker
    // flag -sEMULATE_FUNCTION_POINTER_CASTS to add in emulation for this x86
    // ABI extension.
    var result = {{{ makeDynCall('pp', 'ptr') }}}(arg);
#if STACK_OVERFLOW_CHECK
    checkStackCookie();
#endif
    function finish(result) {
#if MINIMAL_RUNTIME
      // In MINIMAL_RUNTIME the noExitRuntime concept does not apply to
      // pthreads. To exit a pthread with live runtime, use the function
      // emscripten_unwind_to_js_event_loop() in the pthread body.
      __emscripten_thread_exit(result);
#else
      if (keepRuntimeAlive()) {
        EXITSTATUS = result;
      } else {
        __emscripten_thread_exit(result);
      }
#endif
    }
#if ASYNCIFY == 2
    Promise.resolve(result).then(finish);
#else
    finish(result);
#endif
  },

#if MAIN_MODULE
  _emscripten_thread_exit_joinable: (thread) => {
    // Called when a thread exits and is joinable.  We mark these threads
    // as finished, which means that are in state where are no longer actually
    // running, but remain around waiting to be joined.  In this state they
    // cannot run any more proxied work.
    if (!ENVIRONMENT_IS_PTHREAD) markAsFinished(thread);
    else postMessage({ cmd: 'markAsFinished', thread });
  },

  $markAsFinished: (pthread_ptr) => {
#if PTHREADS_DEBUG
    dbg(`markAsFinished: ${ptrToString(pthread_ptr)}`);
#endif
    PThread.finishedThreads.add(pthread_ptr);
    if (pthread_ptr in PThread.outstandingPromises) {
      PThread.outstandingPromises[pthread_ptr].resolve();
    }
  },

  // Asynchronous version dlsync_threads. Always run on the main thread.
  // This work happens asynchronously. The `callback` is called once this work
  // is completed, passing the ctx.
  // TODO(sbc): Should we make a new form of __proxy attribute for JS library
  // function that run asynchronously like but blocks the caller until they are
  // done.  Perhaps "sync_with_ctx"?
  _emscripten_dlsync_threads_async__deps: ['_emscripten_proxy_dlsync_async', 'emscripten_promise_create', '$getPromise'],
  _emscripten_dlsync_threads_async: (caller, callback, ctx) => {
#if PTHREADS_DEBUG
    dbg("_emscripten_dlsync_threads_async caller=" + ptrToString(caller));
#endif
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! _emscripten_dlsync_threads_async() can only ever be called from main thread');
#endif

    const promises = [];
    assert(Object.keys(PThread.outstandingPromises).length === 0);

    // This first promise resolves once the main thread has loaded all modules.
    var info = makePromise();
    promises.push(info.promise);
    __emscripten_dlsync_self_async(info.id);


    // We then create a sequence of promises, one per thread, that resolve once
    // each thread has performed its sync using _emscripten_proxy_dlsync.
    // Any new threads that are created after this call will automatically be
    // in sync because we call `__emscripten_dlsync_self` in
    // invokeEntryPoint before the threads entry point is called.
    for (const ptr of Object.keys(PThread.pthreads)) {
      const pthread_ptr = Number(ptr);
      if (pthread_ptr !== caller && !PThread.finishedThreads.has(pthread_ptr)) {
        info = makePromise();
        __emscripten_proxy_dlsync_async(pthread_ptr, info.id);
        PThread.outstandingPromises[pthread_ptr] = info;
        promises.push(info.promise);
      }
    }

#if PTHREADS_DEBUG
    dbg(`_emscripten_dlsync_threads_async: waiting on ${promises.length} promises`);
#endif
    // Once all promises are resolved then we know all threads are in sync and
    // we can call the callback.
    Promise.all(promises).then(() => {
      PThread.outstandingPromises = {};
#if PTHREADS_DEBUG
      dbg('_emscripten_dlsync_threads_async done: calling callback');
#endif
      {{{ makeDynCall('vp', 'callback') }}}(ctx);
    });
  },

  // Synchronous version dlsync_threads. This is only needed for the case then
  // the main thread call dlopen and in that case we have not choice but to
  // synchronously block the main thread until all other threads are in sync.
  // When `dlopen` is called from a worker, the worker itself is blocked but
  // the operation its waiting on (on the main thread) can be async.
  _emscripten_dlsync_threads__deps: ['_emscripten_proxy_dlsync'],
  _emscripten_dlsync_threads: () => {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! _emscripten_dlsync_threads() can only ever be called from main thread');
#endif
    for (const ptr of Object.keys(PThread.pthreads)) {
      const pthread_ptr = Number(ptr);
      if (!PThread.finishedThreads.has(pthread_ptr)) {
        __emscripten_proxy_dlsync(pthread_ptr);
      }
    }
  },
#elif RELOCATABLE
  // Provide a dummy version of _emscripten_thread_exit_joinable when
  // RELOCATABLE is used without MAIN_MODULE.  This is because the call
  // site in pthread_create.c is not able to distinguish between these
  // two cases.
  _emscripten_thread_exit_joinable: (thread) => {},
#endif // MAIN_MODULE

  $checkMailbox__deps: ['$callUserCallback',
                        '_emscripten_check_mailbox',
                        '_emscripten_thread_mailbox_await'],
  $checkMailbox: () => {
    // Only check the mailbox if we have a live pthread runtime. We implement
    // pthread_self to return 0 if there is no live runtime.
    var pthread_ptr = _pthread_self();
    if (pthread_ptr) {
      // If we are using Atomics.waitAsync as our notification mechanism, wait
      // for a notification before processing the mailbox to avoid missing any
      // work that could otherwise arrive after we've finished processing the
      // mailbox and before we're ready for the next notification.
      __emscripten_thread_mailbox_await(pthread_ptr);
      callUserCallback(__emscripten_check_mailbox);
    }
  },

  _emscripten_thread_mailbox_await__deps: ['$checkMailbox'],
  _emscripten_thread_mailbox_await: (pthread_ptr) => {
    if (typeof Atomics.waitAsync === 'function') {
      // Wait on the pthread's initial self-pointer field because it is easy and
      // safe to access from sending threads that need to notify the waiting
      // thread.
      // TODO: How to make this work with wasm64?
      var wait = Atomics.waitAsync(HEAP32, {{{ getHeapOffset('pthread_ptr', 'i32') }}}, pthread_ptr);
#if ASSERTIONS
      assert(wait.async);
#endif
      wait.value.then(checkMailbox);
      var waitingAsync = pthread_ptr + {{{ C_STRUCTS.pthread.waiting_async }}};
      Atomics.store(HEAP32, {{{ getHeapOffset('waitingAsync', 'i32') }}}, 1);
    }
    // If `Atomics.waitAsync` is not implemented, then we will always fall back
    // to postMessage and there is no need to do anything here.
  },

  // PostMessage is used to notify threads instead of Atomics.notify whenever
  // the environment does not implement Atomics.waitAsync or when messaging a
  // new thread that has not had a chance to initialize itself and execute
  // Atomics.waitAsync to prepare for the notification.
  _emscripten_notify_mailbox_postmessage__deps: ['$checkMailbox'],
  _emscripten_notify_mailbox_postmessage: (targetThread, currThreadId) => {
    if (targetThread == currThreadId) {
      setTimeout(checkMailbox);
    } else if (ENVIRONMENT_IS_PTHREAD) {
      postMessage({targetThread, cmd: 'checkMailbox'});
    } else {
      var worker = PThread.pthreads[targetThread];
      if (!worker) {
#if ASSERTIONS
        err(`Cannot send message to thread with ID ${targetThread}, unknown thread ID!`);
#endif
        return;
      }
      worker.postMessage({cmd: 'checkMailbox'});
    }
  }
};

autoAddDeps(LibraryPThread, '$PThread');
addToLibrary(LibraryPThread);
