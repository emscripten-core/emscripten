/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

assert(SHARED_MEMORY);

addToLibrary({
  $atomicWaitStates__internal: true,
  $atomicWaitStates: ['ok', 'not-equal', 'timed-out'],
  $liveAtomicWaitAsyncs: {},
  $liveAtomicWaitAsyncs__internal: true,
  $liveAtomicWaitAsyncCounter: 0,
  $liveAtomicWaitAsyncCounter__internal: true,

  emscripten_atomic_wait_async__deps: ['$atomicWaitStates', '$liveAtomicWaitAsyncs', '$liveAtomicWaitAsyncCounter', '$callUserCallback'],
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

  emscripten_has_threading_support: () => !!globalThis.SharedArrayBuffer,

  emscripten_num_logical_cores: () =>
#if ENVIRONMENT_MAY_BE_NODE
    ENVIRONMENT_IS_NODE ? require('os').cpus().length :
#endif
    navigator['hardwareConcurrency'],
});
