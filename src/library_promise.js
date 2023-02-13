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
    dbg('makePromise: ' + promiseInfo.id);
#endif
    return promiseInfo;
  },

  emscripten_promise_create__deps: ['$makePromise'],
  emscripten_promise_create__sig: 'p',
  emscripten_promise_create: function() {
    return makePromise().id;
  },

  emscripten_promise_destroy__deps: ['$promiseMap'],
  emscripten_promise_destroy__sig: 'vp',
  emscripten_promise_destroy: function(id) {
#if RUNTIME_DEBUG
    dbg('emscripten_promise_destroy: ' + id);
#endif
    promiseMap.free(id);
  },

  emscripten_promise_resolve__deps: ['$promiseMap',
                                     '$getPromise',
                                     'emscripten_promise_destroy'],
  emscripten_promise_resolve__sig: 'vpip',
  emscripten_promise_resolve: function(id, result, value) {
#if RUNTIME_DEBUG
    dbg('emscripten_promise_resolve: ' + id);
#endif
    var info = promiseMap.get(id);
    switch (result) {
      case {{{ cDefine('EM_PROMISE_FULFILL') }}}:
        info.resolve(value);
        return;
      case {{{ cDefine('EM_PROMISE_MATCH') }}}:
        info.resolve(getPromise(value));
        return;
      case {{{ cDefine('EM_PROMISE_MATCH_RELEASE') }}}:
        info.resolve(getPromise(value));
        _emscripten_promise_destroy(value);
        return;
      case {{{ cDefine('EM_PROMISE_REJECT') }}}:
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
      dbg('emscripten promise callback: ' + value);
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
        case {{{ cDefine('EM_PROMISE_FULFILL') }}}:
          return resultVal;
        case {{{ cDefine('EM_PROMISE_MATCH') }}}:
          return getPromise(resultVal);
        case {{{ cDefine('EM_PROMISE_MATCH_RELEASE') }}}:
          var ret = getPromise(resultVal);
          _emscripten_promise_destroy(resultVal);
          return ret;
        case {{{ cDefine('EM_PROMISE_REJECT') }}}:
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
  emscripten_promise_then__sig: 'ppppp',
  emscripten_promise_then: function(id,
                                    onFulfilled,
                                    onRejected,
                                    userData) {
#if RUNTIME_DEBUG
    dbg('emscripten_promise_then: ' + id);
#endif
    {{{ runtimeKeepalivePush() }}};
    var promise = getPromise(id);
    var newId = promiseMap.allocate({
      promise: promise.then(makePromiseCallback(onFulfilled, userData),
                            makePromiseCallback(onRejected, userData))
    });
#if RUNTIME_DEBUG
    dbg('emscripten_promise_then: -> ' + newId);
#endif
    return newId;
  },

  emscripten_promise_all__deps: ['$promiseMap', '$getPromise'],
  emscripten_promise_all__sig: 'pppp',
  emscripten_promise_all: function(idBuf, resultBuf, size) {
    var promises = [];
    for (var i = 0; i < size; i++) {
      var id = {{{ makeGetValue('idBuf', `i*${Runtime.POINTER_SIZE}`, 'i32') }}};
      promises[i] = getPromise(id);
    }
#if RUNTIME_DEBUG
    dbg('emscripten_promise_all: ' + promises);
#endif
    var id = promiseMap.allocate({
      promise: Promise.all(promises).then((results) => {
        if (resultBuf) {
          for (var i = 0; i < size; i++) {
            var result = results[i];
            {{{ makeSetValue('resultBuf', `i*${Runtime.POINTER_SIZE}`, 'result', '*') }}};
          }
        }
        return resultBuf;
      })
    });
#if RUNTIME_DEBUG
    dbg('create: ' + id);
#endif
    return id;
  },
});
