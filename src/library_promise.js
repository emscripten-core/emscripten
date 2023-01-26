/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $promiseMap__deps: ['$handleAllocator'],
  $promiseMap: "new handleAllocator();",

  $getPromise__deps: ['$promiseMap'],
  $getPromise: function(id) {
    return promiseMap.get(id).promise;
  },
  emscripten_promise_create__deps: ['$promiseMap'],
  emscripten_promise_create__sig: 'p',
  emscripten_promise_create: function() {
    var promiseInfo = {};
    promiseInfo.promise = new Promise((resolve, reject) => {
      promiseInfo.reject = reject;
      promiseInfo.resolve = resolve;
    });
    var id = promiseMap.allocate(promiseInfo);
#if RUNTIME_DEBUG
    dbg('emscripten_promise_create: ' + id);
#endif
    return id;
  },

  emscripten_promise_destroy__deps: ['$promiseMap'],
  emscripten_promise_destroy__sig: 'vp',
  emscripten_promise_destroy: function(id) {
#if RUNTIME_DEBUG
    dbg('emscripten_promise_destroy: ' + id);
#endif
    promiseMap.free(id);
  },

  emscripten_promise_resolve__deps: ['$promiseMap', '$getPromise'],
  emscripten_promise_resolve__sig: 'vpip',
  emscripten_promise_resolve: function(id, result, value) {
#if RUNTIME_DEBUG
    err('emscripten_promise_resolve: ' + id);
#endif
    var info = promiseMap.get(id);
    if (result == {{{ cDefine('EM_PROMISE_FULFILL') }}}) {
      info.resolve(value);
    } else if (result == {{{ cDefine('EM_PROMISE_MATCH') }}}) {
      info.resolve(getPromise(value));
    } else if (result == {{{ cDefine('EM_PROMISE_MATCH_RELEASE') }}}) {
      info.resolve(getPromise(value));
      _emscripten_promise_destroy(value);
    } else if (result == {{{ cDefine('EM_PROMISE_REJECT') }}}) {
      info.reject(value);
    } else {
      abort('invalid promise result');
    }
  },

  $makePromiseCallback__deps: ['$callUserCallback',
                               '$withStackSave',
                               '$getPromise'],
  $makePromiseCallback: function(callback, userData) {
    return (value) => {
#if RUNTIME_DEBUG
      dbg('emscripten promise callback: ' + value);
#endif
      return withStackSave(() => {
        var resultPtr = stackAlloc(8);
        {{{ makeSetValue('resultPtr', 0, '0', '*') }}};
        var result;
        callUserCallback(() => {
          result = {{{makeDynCall('ippp', 'callback')}}}(
              resultPtr, userData, value);
        });
        var resultVal = {{{ makeGetValue('resultPtr', 0, '*') }}};
        if (result == {{{ cDefine('EM_PROMISE_FULFILL') }}}) {
          return resultVal;
        } else if (result == {{{ cDefine('EM_PROMISE_MATCH') }}}) {
          return getPromise(resultVal);
        } else if (result == {{{ cDefine('EM_PROMISE_MATCH_RELEASE') }}}) {
          var ret = getPromise(resultVal);
          _emscripten_promise_destroy(resultVal);
          return ret;
        } else if (result == {{{ cDefine('EM_PROMISE_REJECT') }}}) {
          throw resultVal;
        } else {
          // The callback must have thrown an exception intercepted by
          // `callUserCallback` or otherwise failed. Use a null ptr (0) as the
          // default rejection value.
          throw 0;
        }
      });
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
    var promise = getPromise(id);
    var newId = promiseMap.allocate({
      promise: promise.then(makePromiseCallback(onFulfilled, userData),
                            makePromiseCallback(onRejected, userData))
    });
#if RUNTIME_DEBUG
    dbg('create: ' + newId);
#endif
    return newId;
  },

  emscripten_promise_all__deps: ['$promiseMap', '$getPromise', '$POINTER_SIZE'],
  emscripten_promise_all__sig: 'pppp',
  emscripten_promise_all: function(idBuf, resultBuf, size) {
    var promises = [];
    for (var i = 0; i < size; i++) {
      var id = {{{ makeGetValue('idBuf', 'i*POINTER_SIZE', 'i32') }}};
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
            {{{ makeSetValue('resultBuf', 'i*POINTER_SIZE', 'result', '*') }}};
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
