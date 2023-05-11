{{{
  global.captureModuleArg = () => MODULARIZE ? '' : 'self.Module=d;';
  global.instantiateModule = () => MODULARIZE ? `${EXPORT_NAME}(d);` : '';
  global.instantiateWasm = () => MINIMAL_RUNTIME ? '' : 'd[`instantiateWasm`]=(i,r)=>{var n=new WebAssembly.Instance(d[`wasm`],i);r(n,d[`wasm`]);return n.exports};';
  null;
}}}

#if WASM_WORKERS

#if !SHARED_MEMORY
#error "Internal error! SHARED_MEMORY should be enabled when building with WASM_WORKERS"
#endif
#if SINGLE_FILE
#error "-sSINGLE_FILE is not supported with -sWASM_WORKERS!"
#endif
#if LINKABLE
#error "-sLINKABLE is not supported with -sWASM_WORKERS!"
#endif
#if SIDE_MODULE
#error "-sSIDE_MODULE is not supported with -sWASM_WORKERS!"
#endif
#if MAIN_MODULE
#error "-sMAIN_MODULE is not supported with -sWASM_WORKERS!"
#endif
#if PROXY_TO_WORKER
#error "-sPROXY_TO_WORKER is not supported with -sWASM_WORKERS!"
#endif

#endif // ~WASM_WORKERS


mergeInto(LibraryManager.library, {
  $_wasmWorkers: {},
  $_wasmWorkersID: 1,

  // Starting up a Wasm Worker is an asynchronous operation, hence if the parent thread performs any
  // postMessage()-based wasm function calls s to the Worker, they must be delayed until the async
  // startup has finished, after which these postponed function calls can be dispatched.
  $_wasmWorkerDelayedMessageQueue: [],

  $_wasmWorkerAppendToQueue: function(e) {
    _wasmWorkerDelayedMessageQueue.push(e);
  },

  // Executes a wasm function call received via a postMessage.
  $_wasmWorkerRunPostMessage: function(e) {
    let data = e.data, wasmCall = data['_wsc']; // '_wsc' is short for 'wasm call', trying to use an identifier name that will never conflict with user code
    wasmCall && getWasmTableEntry(wasmCall)(...data['x']);
  },

  // src/postamble_minimal.js brings this symbol in to the build, and calls this function synchronously
  // from main JS file at the startup of each Worker.
  $_wasmWorkerInitializeRuntime__deps: ['$_wasmWorkerDelayedMessageQueue', '$_wasmWorkerRunPostMessage', '$_wasmWorkerAppendToQueue', 'emscripten_wasm_worker_initialize'],
  $_wasmWorkerInitializeRuntime: function() {
    let m = Module;
#if ASSERTIONS
    assert(m['sb'] % 16 == 0);
    assert(m['sz'] % 16 == 0);
#endif

#if STACK_OVERFLOW_CHECK >= 2
    // _emscripten_wasm_worker_initialize() initializes the stack for this Worker,
    // but it cannot call to extern __set_stack_limits() function, or Binaryen breaks
    // with "Fatal: Module::addFunction: __set_stack_limits already exists".
    // So for now, invoke this function from JS side. TODO: remove this in the future.
    // Note that this call is not exactly correct, since this limit will include
    // the TLS slot, that will be part of the region between m['sb'] and m['sz'],
    // so we need to fix up the call below.
    ___set_stack_limits(m['sb'] + m['sz'], m['sb']);
#endif
    // Run the C side Worker initialization for stack and TLS.
    _emscripten_wasm_worker_initialize(m['sb'], m['sz']);
#if PTHREADS
    // Record that this Wasm Worker supports synchronous blocking in emscripten_futex_wake().
    ___set_thread_state(/*thread_ptr=*/0, /*is_main_thread=*/0, /*is_runtime_thread=*/0, /*supports_wait=*/0);
#endif
#if STACK_OVERFLOW_CHECK >= 2
    // Fix up stack base. (TLS frame is created at the bottom address end of the stack)
    // See https://github.com/emscripten-core/emscripten/issues/16496
    ___set_stack_limits(_emscripten_stack_get_base(), _emscripten_stack_get_end());
#endif

#if AUDIO_WORKLET
    // Audio Worklets do not have postMessage()ing capabilities.
    if (typeof AudioWorkletGlobalScope === 'undefined') {
#endif
      // The Wasm Worker runtime is now up, so we can start processing
      // any postMessage function calls that have been received. Drop the temp
      // message handler that queued any pending incoming postMessage function calls ...
      removeEventListener('message', _wasmWorkerAppendToQueue);
      // ... then flush whatever messages we may have already gotten in the queue,
      //     and clear _wasmWorkerDelayedMessageQueue to undefined ...
      _wasmWorkerDelayedMessageQueue = _wasmWorkerDelayedMessageQueue.forEach(_wasmWorkerRunPostMessage);
      // ... and finally register the proper postMessage handler that immediately
      // dispatches incoming function calls without queueing them.
      addEventListener('message', _wasmWorkerRunPostMessage);
#if AUDIO_WORKLET
    }
#endif
  },

#if WASM_WORKERS == 2
  // In WASM_WORKERS == 2 build mode, we create the Wasm Worker global scope script from a string bundled in the main application JS file. This simplifies the number of deployed JS files with the app,
  // but has a downside that the generated build output will no longer be csp-eval compliant. https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Security-Policy/script-src#unsafe_eval_expressions
  $_wasmWorkerBlobUrl: "URL.createObjectURL(new Blob(['onmessage=function(d){onmessage=null;d=d.data;{{{ captureModuleArg() }}}{{{ instantiateWasm() }}}importScripts(d.js);{{{ instantiateModule() }}}d.wasm=d.mem=d.js=0;}'],{type:'application/javascript'}))",
#endif

  _emscripten_create_wasm_worker__deps: ['$_wasmWorkers', '$_wasmWorkersID', '$_wasmWorkerAppendToQueue', '$_wasmWorkerRunPostMessage'
#if WASM_WORKERS == 2
    , '$_wasmWorkerBlobUrl'
#endif
  ],
  _emscripten_create_wasm_worker__postset: 'if (ENVIRONMENT_IS_WASM_WORKER) {\n'
    + '_wasmWorkers[0] = this;\n'
    + 'addEventListener("message", _wasmWorkerAppendToQueue);\n'
    + '}\n',
  _emscripten_create_wasm_worker: function(stackLowestAddress, stackSize) {
    let worker = _wasmWorkers[_wasmWorkersID] = new Worker(
#if WASM_WORKERS == 2 // WASM_WORKERS=2 mode embeds .ww.js file contents into the main .js file as a Blob URL. (convenient, but not CSP security safe, since this is eval-like)
      _wasmWorkerBlobUrl
#elif MINIMAL_RUNTIME // MINIMAL_RUNTIME has a structure where the .ww.js file is loaded from the main HTML file in parallel to all other files for best performance
      Module['$wb'] // $wb="Wasm worker Blob", abbreviated since not DCEable
#else // default runtime loads the .ww.js file on demand.
      locateFile('{{{ WASM_WORKER_FILE }}}')
#endif
    );
    // Craft the Module object for the Wasm Worker scope:
    worker.postMessage({
      '$ww': _wasmWorkersID, // Signal with a non-zero value that this Worker will be a Wasm Worker, and not the main browser thread.
#if MINIMAL_RUNTIME
      'wasm': Module['wasm'],
      'js': Module['js'],
      'mem': wasmMemory,
#else
      'wasm': wasmModule,
      'js': Module['mainScriptUrlOrBlob'] || _scriptDir,
      'wasmMemory': wasmMemory,
#endif
      'sb': stackLowestAddress, // sb = stack bottom (lowest stack address, SP points at this when stack is full)
      'sz': stackSize,          // sz = stack size
    });
    worker.onmessage = _wasmWorkerRunPostMessage;
    return _wasmWorkersID++;
  },

  emscripten_terminate_wasm_worker: function(id) {
#if ASSERTIONS
    assert(id != 0, 'emscripten_terminate_wasm_worker() cannot be called with id=0!');
#endif
    if (_wasmWorkers[id]) {
      _wasmWorkers[id].terminate();
      delete _wasmWorkers[id];
    }
  },

  emscripten_terminate_all_wasm_workers: function() {
#if ASSERTIONS
    assert(!ENVIRONMENT_IS_WASM_WORKER, 'emscripten_terminate_all_wasm_workers() cannot be called from a Wasm Worker: only the main browser thread has visibility to terminate all Workers!');
#endif
    Object.values(_wasmWorkers).forEach((worker) => {
      worker.terminate();
    });
    _wasmWorkers = {};
  },

  emscripten_current_thread_is_wasm_worker: function() {
#if WASM_WORKERS
    return ENVIRONMENT_IS_WASM_WORKER;
#else
    // implicit return 0;
#endif
  },

  emscripten_wasm_worker_self_id: function() {
    return Module['$ww'];
  },

  emscripten_wasm_worker_post_function_v: function(id, funcPtr) {
    _wasmWorkers[id].postMessage({'_wsc': funcPtr, 'x': [] }); // "WaSm Call"
  },

  $_wasmWorkerPostFunction1__sig: 'vipd',
  $_wasmWorkerPostFunction1: function(id, funcPtr, arg0) {
    _wasmWorkers[id].postMessage({'_wsc': funcPtr, 'x': [arg0] }); // "WaSm Call"
  },

  emscripten_wasm_worker_post_function_vi: '$_wasmWorkerPostFunction1',
  emscripten_wasm_worker_post_function_vd: '$_wasmWorkerPostFunction1',

  $_wasmWorkerPostFunction2__sig: 'vipdd',
  $_wasmWorkerPostFunction2: function(id, funcPtr, arg0, arg1) {
    _wasmWorkers[id].postMessage({'_wsc': funcPtr, 'x': [arg0, arg1] }); // "WaSm Call"
  },
  emscripten_wasm_worker_post_function_vii: '$_wasmWorkerPostFunction2',
  emscripten_wasm_worker_post_function_vdd: '$_wasmWorkerPostFunction2',

  $_wasmWorkerPostFunction3__sig: 'vipddd',
  $_wasmWorkerPostFunction3: function(id, funcPtr, arg0, arg1, arg2) {
    _wasmWorkers[id].postMessage({'_wsc': funcPtr, 'x': [arg0, arg1, arg2] }); // "WaSm Call"
  },
  emscripten_wasm_worker_post_function_viii: '$_wasmWorkerPostFunction3',
  emscripten_wasm_worker_post_function_vddd: '$_wasmWorkerPostFunction3',

  emscripten_wasm_worker_post_function_sig__deps: ['$readEmAsmArgs'],
  emscripten_wasm_worker_post_function_sig: function(id, funcPtr, sigPtr, varargs) {
#if ASSERTIONS
    assert(id >= 0);
    assert(funcPtr);
    assert(sigPtr);
    assert(UTF8ToString(sigPtr)[0] != 'v', 'Do NOT specify the return argument in the signature string for a call to emscripten_wasm_worker_post_function_sig(), just pass the function arguments.');
    assert(varargs);
#endif
    _wasmWorkers[id].postMessage({'_wsc': funcPtr, 'x': readEmAsmArgs(sigPtr, varargs) });
  },

  $atomicWaitStates: "['ok', 'not-equal', 'timed-out']",

// Chrome 87 (and hence Edge 87) shipped Atomics.waitAsync (https://www.chromestatus.com/feature/6243382101803008)
// However its implementation is faulty: https://bugs.chromium.org/p/chromium/issues/detail?id=1167541
// Firefox Nightly 86.0a1 (2021-01-15) does not yet have it, https://bugzilla.mozilla.org/show_bug.cgi?id=1467846
// And at the time of writing, no other browser has it either.
#if MIN_EDGE_VERSION < 91 || MIN_CHROME_VERSION < 91 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED || MIN_FIREFOX_VERSION != TARGET_NOT_SUPPORTED || ENVIRONMENT_MAY_BE_NODE
  // Partially polyfill Atomics.waitAsync() if not available in the browser.
  // Also polyfill for old Chrome-based browsers, where Atomics.waitAsync is broken until Chrome 91,
  // see https://bugs.chromium.org/p/chromium/issues/detail?id=1167541
  // https://github.com/tc39/proposal-atomics-wait-async/blob/master/PROPOSAL.md
  // This polyfill performs polling with setTimeout() to observe a change in the target memory location.
  emscripten_atomic_wait_async__postset: `if (!Atomics['waitAsync'] || jstoi_q((navigator.userAgent.match(/Chrom(e|ium)\\/([0-9]+)\\./)||[])[2]) < 91) {
let __Atomics_waitAsyncAddresses = [/*[i32a, index, value, maxWaitMilliseconds, promiseResolve]*/];
function __Atomics_pollWaitAsyncAddresses() {
  let now = performance.now();
  let l = __Atomics_waitAsyncAddresses.length;
  for(let i = 0; i < l; ++i) {
    let a = __Atomics_waitAsyncAddresses[i];
    let expired = (now > a[3]);
    let awoken = (Atomics.load(a[0], a[1]) != a[2]);
    if (expired || awoken) {
      __Atomics_waitAsyncAddresses[i--] = __Atomics_waitAsyncAddresses[--l];
      __Atomics_waitAsyncAddresses.length = l;
      a[4](awoken ? 'ok': 'timed-out');
    }
  }
  if (l) {
    // If we still have addresses to wait, loop the timeout handler to continue polling.
    setTimeout(__Atomics_pollWaitAsyncAddresses, 10);
  }
}
#if ASSERTIONS
  if (!ENVIRONMENT_IS_WASM_WORKER) err('Current environment does not support Atomics.waitAsync(): polyfilling it, but this is going to be suboptimal.');
#endif
Atomics['waitAsync'] = function(i32a, index, value, maxWaitMilliseconds) {
  let val = Atomics.load(i32a, index);
  if (val != value) return { async: false, value: 'not-equal' };
  if (maxWaitMilliseconds <= 0) return { async: false, value: 'timed-out' };
  maxWaitMilliseconds = performance.now() + (maxWaitMilliseconds || Infinity);
  let promiseResolve;
  let promise = new Promise((resolve) => { promiseResolve = resolve; });
  if (!__Atomics_waitAsyncAddresses[0]) setTimeout(__Atomics_pollWaitAsyncAddresses, 10);
  __Atomics_waitAsyncAddresses.push([i32a, index, value, maxWaitMilliseconds, promiseResolve]);
  return { async: true, value: promise };
};
}`,

  // These dependencies are artificial, issued so that we still get the waitAsync polyfill emitted
  // if code only calls emscripten_lock/semaphore_async_acquire()
  // but not emscripten_atomic_wait_async() directly.
  emscripten_lock_async_acquire__deps: ['emscripten_atomic_wait_async'],
  emscripten_semaphore_async_acquire__deps: ['emscripten_atomic_wait_async'],

#endif

  $atomicLiveWaitAsyncs: '{}',
  $atomicLiveWaitAsyncsCounter: '0',

  emscripten_atomic_wait_async__deps: ['$atomicWaitStates', '$atomicLiveWaitAsyncs', '$atomicLiveWaitAsyncsCounter', '$jstoi_q'],
  emscripten_atomic_wait_async: function(addr, val, asyncWaitFinished, userData, maxWaitMilliseconds) {
    let wait = Atomics['waitAsync'](HEAP32, addr >> 2, val, maxWaitMilliseconds);
    if (!wait.async) return atomicWaitStates.indexOf(wait.value);
    // Increment waitAsync generation counter, account for wraparound in case application does huge amounts of waitAsyncs per second (not sure if possible?)
    // Valid counterrange: 0...2^31-1
    let counter = atomicLiveWaitAsyncsCounter;
    atomicLiveWaitAsyncsCounter = Math.max(0, (atomicLiveWaitAsyncsCounter+1)|0);
    atomicLiveWaitAsyncs[counter] = addr >> 2;
    wait.value.then((value) => {
      if (atomicLiveWaitAsyncs[counter]) {
        delete atomicLiveWaitAsyncs[counter];
        {{{ makeDynCall('viiii', 'asyncWaitFinished') }}}(addr, val, atomicWaitStates.indexOf(value), userData);
      }
    });
    return -counter;
  },

  emscripten_atomic_cancel_wait_async__deps: ['$atomicLiveWaitAsyncs'],
  emscripten_atomic_cancel_wait_async: function(waitToken) {
#if ASSERTIONS
    if (waitToken == 1 /* ATOMICS_WAIT_NOT_EQUAL */) warnOnce('Attempted to call emscripten_atomic_cancel_wait_async() with a value ATOMICS_WAIT_NOT_EQUAL (1) that is not a valid wait token! Check success in return value from call to emscripten_atomic_wait_async()');
    else if (waitToken == 2 /* ATOMICS_WAIT_TIMED_OUT */) warnOnce('Attempted to call emscripten_atomic_cancel_wait_async() with a value ATOMICS_WAIT_TIMED_OUT (2) that is not a valid wait token! Check success in return value from call to emscripten_atomic_wait_async()');
    else if (waitToken > 0) warnOnce('Attempted to call emscripten_atomic_cancel_wait_async() with an invalid wait token value ' + waitToken);
#endif
    if (atomicLiveWaitAsyncs[waitToken]) {
      // Notify the waitAsync waiters on the memory location, so that JavaScript garbage collection can occur
      // See https://github.com/WebAssembly/threads/issues/176
      // This has the unfortunate effect of causing spurious wakeup of all other waiters at the address (which
      // causes a small performance loss)
      Atomics.notify(HEAP32, atomicLiveWaitAsyncs[waitToken]);
      delete atomicLiveWaitAsyncs[waitToken];
      return 0 /* EMSCRIPTEN_RESULT_SUCCESS */;
    }
    // This waitToken does not exist.
    return -5 /* EMSCRIPTEN_RESULT_INVALID_PARAM */;
  },

  emscripten_atomic_cancel_all_wait_asyncs__deps: ['$atomicLiveWaitAsyncs'],
  emscripten_atomic_cancel_all_wait_asyncs: function() {
    let waitAsyncs = Object.values(atomicLiveWaitAsyncs);
    waitAsyncs.forEach((address) => {
      Atomics.notify(HEAP32, address);
    });
    atomicLiveWaitAsyncs = {};
    return waitAsyncs.length;
  },

  emscripten_atomic_cancel_all_wait_asyncs_at_address__deps: ['$atomicLiveWaitAsyncs'],
  emscripten_atomic_cancel_all_wait_asyncs_at_address: function(address) {
    address >>= 2;
    let numCancelled = 0;
    Object.keys(atomicLiveWaitAsyncs).forEach((waitToken) => {
      if (atomicLiveWaitAsyncs[waitToken] == address) {
        Atomics.notify(HEAP32, address);
        delete atomicLiveWaitAsyncs[waitToken];
        ++numCancelled;
      }
    });
    return numCancelled;
  },

  emscripten_navigator_hardware_concurrency: function() {
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) return require('os').cpus().length;
#endif
    return navigator['hardwareConcurrency'];
  },

  emscripten_atomics_is_lock_free: function(width) {
    return Atomics.isLockFree(width);
  },

  emscripten_lock_async_acquire: function(lock, asyncWaitFinished, userData, maxWaitMilliseconds) {
    let dispatch = (val, ret) => {
      setTimeout(() => {
        {{{ makeDynCall('viiii', 'asyncWaitFinished') }}}(lock, val, /*waitResult=*/ret, userData);
      }, 0);
    };
    let tryAcquireLock = () => {
      do {
        var val = Atomics.compareExchange(HEAP32, lock >> 2, 0/*zero represents lock being free*/, 1/*one represents lock being acquired*/);
        if (!val) return dispatch(0, 0/*'ok'*/);
        var wait = Atomics['waitAsync'](HEAP32, lock >> 2, val, maxWaitMilliseconds);
      } while(wait.value === 'not-equal');
#if ASSERTIONS
      assert(wait.async || wait.value === 'timed-out');
#endif
      if (wait.async) wait.value.then(tryAcquireLock);
      else dispatch(val, 2/*'timed-out'*/);
    };
    tryAcquireLock();
  },

  emscripten_semaphore_async_acquire: function(sem, num, asyncWaitFinished, userData, maxWaitMilliseconds) {
    let dispatch = (idx, ret) => {
      setTimeout(() => {
        {{{ makeDynCall('viiii', 'asyncWaitFinished') }}}(sem, /*val=*/idx, /*waitResult=*/ret, userData);
      }, 0);
    };
    let tryAcquireSemaphore = () => {
      let val = num;
      do {
        let ret = Atomics.compareExchange(HEAP32, sem >> 2,
                                          val, /* We expect this many semaphore resoures to be available*/
                                          val - num /* Acquire 'num' of them */);
        if (ret == val) return dispatch(ret/*index of resource acquired*/, 0/*'ok'*/);
        val = ret;
        let wait = Atomics['waitAsync'](HEAP32, sem >> 2, ret, maxWaitMilliseconds);
      } while(wait.value === 'not-equal');
#if ASSERTIONS
      assert(wait.async || wait.value === 'timed-out');
#endif
      if (wait.async) wait.value.then(tryAcquireSemaphore);
      else dispatch(-1/*idx*/, 2/*'timed-out'*/);
    };
    tryAcquireSemaphore();
  }
});
