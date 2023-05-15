/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $promiseMap__deps: ['$HandleAllocator'],
  $promiseMap: "new HandleAllocator();",

  $getPromise__deps: ['$promiseMap'],
  $getPromise: function(id) {
    return promiseMap.get(id).promise;
  },

  $makePromise__deps: ['$promiseMap'],
  $makePromise: function() {
    var promiseInfo = {};
    promiseInfo.promise = new Promise((resolve, reject) => {
      promiseInfo.reject = reject;
      promiseInfo.resolve = resolve;
    });
    promiseInfo.id = promiseMap.allocate(promiseInfo);
#if RUNTIME_DEBUG
    dbg(`makePromise: ${promiseInfo.id}`);
#endif
    return promiseInfo;
  },

  $idsToPromises__deps: ['$promiseMap', '$getPromise'],
  $idsToPromises: function(idBuf, size) {
    var promises = [];
    for (var i = 0; i < size; i++) {
      var id = {{{ makeGetValue('idBuf', `i*${POINTER_SIZE}`, 'i32') }}};
      promises[i] = getPromise(id);
    }
    return promises;
  },

  emscripten_promise_create__deps: ['$makePromise'],
  emscripten_promise_create: function() {
    return makePromise().id;
  },

  emscripten_promise_destroy__deps: ['$promiseMap'],
  emscripten_promise_destroy: function(id) {
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_destroy: ${id}`);
#endif
    promiseMap.free(id);
  },

  emscripten_promise_resolve__deps: ['$promiseMap',
                                     '$getPromise',
                                     'emscripten_promise_destroy'],
  emscripten_promise_resolve: function(id, result, value) {
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_resolve: ${id}`);
#endif
    var info = promiseMap.get(id);
    switch (result) {
      case {{{ cDefs.EM_PROMISE_FULFILL }}}:
        info.resolve(value);
        return;
      case {{{ cDefs.EM_PROMISE_MATCH }}}:
        info.resolve(getPromise(value));
        return;
      case {{{ cDefs.EM_PROMISE_MATCH_RELEASE }}}:
        info.resolve(getPromise(value));
        _emscripten_promise_destroy(value);
        return;
      case {{{ cDefs.EM_PROMISE_REJECT }}}:
        info.reject(value);
        return;
    }
#if ASSERTIONS
    abort("unexpected promise callback result " + result);
#endif
  },

  $makePromiseCallback__deps: ['$getPromise',
                               '$POINTER_SIZE',
                               'emscripten_promise_destroy'],
  $makePromiseCallback: function(callback, userData) {
    return (value) => {
#if RUNTIME_DEBUG
      dbg(`emscripten promise callback: ${value}`);
#endif
      {{{ runtimeKeepalivePop() }}};
      var stack = stackSave();
      // Allocate space for the result value and initialize it to NULL.
      var resultPtr = stackAlloc(POINTER_SIZE);
      {{{ makeSetValue('resultPtr', 0, '0', '*') }}};
      try {
        var result =
            {{{ makeDynCall('ippp', 'callback') }}}(resultPtr, userData, value);
        var resultVal = {{{ makeGetValue('resultPtr', 0, '*') }}};
      } catch (e) {
        // If the thrown value is potentially a valid pointer, use it as the
        // rejection reason. Otherwise use a null pointer as the reason. If we
        // allow arbitrary objects to be thrown here, we will get a TypeError in
        // MEMORY64 mode when they are later converted to void* rejection
        // values.
#if MEMORY64
        if (typeof e !== 'bigint') {
          throw 0n;
        }
#else
        if (typeof e !== 'number') {
          throw 0;
        }
#endif
        throw e;
      } finally {
        // Thrown errors will reject the promise, but at least we will restore
        // the stack first.
        stackRestore(stack);
      }
      switch (result) {
        case {{{ cDefs.EM_PROMISE_FULFILL }}}:
          return resultVal;
        case {{{ cDefs.EM_PROMISE_MATCH }}}:
          return getPromise(resultVal);
        case {{{ cDefs.EM_PROMISE_MATCH_RELEASE }}}:
          var ret = getPromise(resultVal);
          _emscripten_promise_destroy(resultVal);
          return ret;
        case {{{ cDefs.EM_PROMISE_REJECT }}}:
          throw resultVal;
      }
#if ASSERTIONS
      abort("unexpected promise callback result " + result);
#endif
    };
  },

  emscripten_promise_then__deps: ['$promiseMap',
                                  '$getPromise',
                                  '$makePromiseCallback'],
  emscripten_promise_then: function(id,
                                    onFulfilled,
                                    onRejected,
                                    userData) {
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_then: ${id}`);
#endif
    {{{ runtimeKeepalivePush() }}};
    var promise = getPromise(id);
    var newId = promiseMap.allocate({
      promise: promise.then(makePromiseCallback(onFulfilled, userData),
                            makePromiseCallback(onRejected, userData))
    });
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_then: -> ${newId}`);
#endif
    return newId;
  },

  emscripten_promise_all__deps: ['$promiseMap', '$idsToPromises'],
  emscripten_promise_all: function(idBuf, resultBuf, size) {
    var promises = idsToPromises(idBuf, size);
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_all: ${promises}`);
#endif
    var id = promiseMap.allocate({
      promise: Promise.all(promises).then((results) => {
        if (resultBuf) {
          for (var i = 0; i < size; i++) {
            var result = results[i];
            {{{ makeSetValue('resultBuf', `i*${POINTER_SIZE}`, 'result', '*') }}};
          }
        }
        return resultBuf;
      })
    });
#if RUNTIME_DEBUG
    dbg(`create: ${id}`);
#endif
    return id;
  },

  emscripten_promise_all_settled__deps: ['$promiseMap', '$idsToPromises'],
  emscripten_promise_all_settled: function(idBuf, resultBuf, size) {
    var promises = idsToPromises(idBuf, size);
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_all_settled: ${promises}`);
#endif
    var id = promiseMap.allocate({
      promise: Promise.allSettled(promises).then((results) => {
        if (resultBuf) {
          for (var i = 0; i < size; i++) {
            var baseOffset = i * {{{ C_STRUCTS.em_settled_result_t.__size__ }}};
            var resultOffset =
                baseOffset + {{{ C_STRUCTS.em_settled_result_t.result }}};
            var valueOffset =
                baseOffset + {{{ C_STRUCTS.em_settled_result_t.value }}};
            if (results[i].status === 'fulfilled') {
              var fulfill = {{{ cDefs.EM_PROMISE_FULFILL }}};
              {{{ makeSetValue('resultBuf', 'resultOffset', 'fulfill', 'i32') }}};
              {{{ makeSetValue('resultBuf', 'valueOffset', 'results[i].value', '*') }}};
            } else {
              var reject = {{{ cDefs.EM_PROMISE_REJECT }}};
              {{{ makeSetValue('resultBuf', 'resultOffset', 'reject', 'i32') }}};
              // Closure can't type `reason` in some contexts.
              var reason = /** @type {number} */ (results[i].reason);
              {{{ makeSetValue('resultBuf', 'valueOffset', 'reason', '*') }}};
            }
          }
        }
        return resultBuf;
      })
    });
#if RUNTIME_DEBUG
    dbg(`create: ${id}`);
#endif
    return id;
  },


  emscripten_promise_any__deps: [
    '$promiseMap', '$idsToPromises',
#if !SUPPORTS_PROMISE_ANY && !INCLUDE_FULL_LIBRARY
    () => error("emscripten_promise_any used, but Promise.any is not supported by the current runtime configuration (run with EMCC_DEBUG=1 in the env for more details)"),
#endif
  ],
  emscripten_promise_any: function(idBuf, errorBuf, size) {
    var promises = idsToPromises(idBuf, size);
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_any: ${promises}`);
#endif
#if ASSERTIONS
    assert(typeof Promise.any !== 'undefined', "Promise.any does not exist");
#endif
    var id = promiseMap.allocate({
      promise: Promise.any(promises).catch((err) => {
        if (errorBuf) {
          for (var i = 0; i < size; i++) {
            {{{ makeSetValue('errorBuf', `i*${POINTER_SIZE}`, 'err.errors[i]', '*') }}};
          }
        }
        throw errorBuf;
      })
    });
#if RUNTIME_DEBUG
    dbg(`create: ${id}`);
#endif
    return id;
  },

  emscripten_promise_race__deps: ['$promiseMap', '$idsToPromises'],
  emscripten_promise_race: function(idBuf, size) {
    var promises = idsToPromises(idBuf, size);
#if RUNTIME_DEBUG
    dbg(`emscripten_promise_race: ${promises}`);
#endif
    var id = promiseMap.allocate({
      promise: Promise.race(promises)
    });
#if RUNTIME_DEBUG
    dbg(`create: ${id}`);
#endif
    return id;
  }
});
