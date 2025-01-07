/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

assert(SHARED_MEMORY);

addToLibrary({
// Chrome 87 shipped Atomics.waitAsync:
//   https://www.chromestatus.com/feature/6243382101803008
// However its implementation is faulty:
//   https://bugs.chromium.org/p/chromium/issues/detail?id=1167541
// Firefox Nightly 86.0a1 (2021-01-15) does not yet have it:
//   https://bugzilla.mozilla.org/show_bug.cgi?id=1467846
// And at the time of writing, no other browser has it either.
#if MIN_CHROME_VERSION < 91 || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED || MIN_FIREFOX_VERSION != TARGET_NOT_SUPPORTED || ENVIRONMENT_MAY_BE_NODE
  // Partially polyfill Atomics.waitAsync() if not available in the browser.
  // Also polyfill for old Chrome-based browsers, where Atomics.waitAsync is
  // broken until Chrome 91, see:
  //   https://bugs.chromium.org/p/chromium/issues/detail?id=1167541
  //   https://github.com/tc39/proposal-atomics-wait-async/blob/master/PROPOSAL.md
  // This polyfill performs polling with setTimeout() to observe a change in the
  // target memory location.
  $polyfillWaitAsync__postset: `if (!Atomics.waitAsync || (typeof navigator != 'undefined' && navigator.userAgent && jstoi_q((navigator.userAgent.match(/Chrom(e|ium)\\/([0-9]+)\\./)||[])[2]) < 91)) {
  let __Atomics_waitAsyncAddresses = [/*[i32a, index, value, maxWaitMilliseconds, promiseResolve]*/];
  function __Atomics_pollWaitAsyncAddresses() {
    let now = performance.now();
    let l = __Atomics_waitAsyncAddresses.length;
    for (let i = 0; i < l; ++i) {
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
  #if ASSERTIONS && WASM_WORKERS
    if (!ENVIRONMENT_IS_WASM_WORKER) err('Current environment does not support Atomics.waitAsync(): polyfilling it, but this is going to be suboptimal.');
  #endif
  /**
   * @param {number=} maxWaitMilliseconds
   */
  Atomics.waitAsync = (i32a, index, value, maxWaitMilliseconds) => {
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
  $polyfillWaitAsync__deps: ['$jstoi_q'],
#endif

  $polyfillWaitAsync__internal: true,
  $polyfillWaitAsync: () => {
    // nop, used for its postset to ensure `Atomics.waitAsync()` polyfill is
    // included exactly once and only included when needed.
    // Any function using Atomics.waitAsync should depend on this.
  },

  $atomicWaitStates__internal: true,
  $atomicWaitStates: ['ok', 'not-equal', 'timed-out'],
  $liveAtomicWaitAsyncs: {},
  $liveAtomicWaitAsyncs__internal: true,
  $liveAtomicWaitAsyncCounter: 0,
  $liveAtomicWaitAsyncCounter__internal: true,

  emscripten_atomic_wait_async__deps: ['$atomicWaitStates', '$liveAtomicWaitAsyncs', '$liveAtomicWaitAsyncCounter', '$polyfillWaitAsync', '$callUserCallback'],
  emscripten_atomic_wait_async: (addr, val, asyncWaitFinished, userData, maxWaitMilliseconds) => {
    let wait = Atomics.waitAsync(HEAP32, {{{ getHeapOffset('addr', 'i32') }}}, val, maxWaitMilliseconds);
    if (!wait.async) return atomicWaitStates.indexOf(wait.value);
    // Increment waitAsync generation counter, account for wraparound in case
    // application does huge amounts of waitAsyncs per second (not sure if
    // possible?)
    // Valid counterrange: 0...2^31-1
    let counter = liveAtomicWaitAsyncCounter;
    liveAtomicWaitAsyncCounter = Math.max(0, (liveAtomicWaitAsyncCounter+1)|0);
    liveAtomicWaitAsyncs[counter] = addr;
    {{{ runtimeKeepalivePush() }}}
    wait.value.then((value) => {
      if (liveAtomicWaitAsyncs[counter]) {
        {{{ runtimeKeepalivePop() }}}
        delete liveAtomicWaitAsyncs[counter];
        callUserCallback(() => {{{ makeDynCall('vpiip', 'asyncWaitFinished') }}}(addr, val, atomicWaitStates.indexOf(value), userData));
      }
    });
    return -counter;
  },

  emscripten_atomic_cancel_wait_async__deps: ['$liveAtomicWaitAsyncs'],
  emscripten_atomic_cancel_wait_async: (waitToken) => {
#if ASSERTIONS
    if (waitToken == {{{ cDefs.ATOMICS_WAIT_NOT_EQUAL }}}) {
      warnOnce('Attempted to call emscripten_atomic_cancel_wait_async() with a value ATOMICS_WAIT_NOT_EQUAL (1) that is not a valid wait token! Check success in return value from call to emscripten_atomic_wait_async()');
    } else if (waitToken == {{{ cDefs.ATOMICS_WAIT_TIMED_OUT }}}) {
      warnOnce('Attempted to call emscripten_atomic_cancel_wait_async() with a value ATOMICS_WAIT_TIMED_OUT (2) that is not a valid wait token! Check success in return value from call to emscripten_atomic_wait_async()');
    } else if (waitToken > 0) {
      warnOnce(`Attempted to call emscripten_atomic_cancel_wait_async() with an invalid wait token value ${waitToken}`);
    }
#endif
    var address = liveAtomicWaitAsyncs[waitToken];
    if (address) {
      // Notify the waitAsync waiters on the memory location, so that JavaScript
      // garbage collection can occur.
      // See https://github.com/WebAssembly/threads/issues/176
      // This has the unfortunate effect of causing spurious wakeup of all other
      // waiters at the address (which causes a small performance loss).
      Atomics.notify(HEAP32, {{{ getHeapOffset('address', 'i32') }}});
      delete liveAtomicWaitAsyncs[waitToken];
      {{{ runtimeKeepalivePop() }}}
      return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
    }
    // This waitToken does not exist.
    return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
  },

  emscripten_atomic_cancel_all_wait_asyncs__deps: ['$liveAtomicWaitAsyncs'],
  emscripten_atomic_cancel_all_wait_asyncs: () => {
    let waitAsyncs = Object.values(liveAtomicWaitAsyncs);
    waitAsyncs.forEach((address) => {
      Atomics.notify(HEAP32, {{{ getHeapOffset('address', 'i32') }}});
    });
    liveAtomicWaitAsyncs = {};
    return waitAsyncs.length;
  },

  emscripten_atomic_cancel_all_wait_asyncs_at_address__deps: ['$liveAtomicWaitAsyncs'],
  emscripten_atomic_cancel_all_wait_asyncs_at_address: (address) => {
    let numCancelled = 0;
    Object.keys(liveAtomicWaitAsyncs).forEach((waitToken) => {
      if (liveAtomicWaitAsyncs[waitToken] == address) {
        Atomics.notify(HEAP32, {{{ getHeapOffset('address', 'i32') }}});
        delete liveAtomicWaitAsyncs[waitToken];
        numCancelled++;
      }
    });
    return numCancelled;
  },

  emscripten_has_threading_support: () => typeof SharedArrayBuffer != 'undefined',

  emscripten_num_logical_cores: () =>
#if ENVIRONMENT_MAY_BE_NODE
    ENVIRONMENT_IS_NODE ? require('os').cpus().length :
#endif
    navigator['hardwareConcurrency'],
});
