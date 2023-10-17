/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Implementation of functions from emscripten/eventloop.h.

LibraryJSEventLoop = {
  emscripten_unwind_to_js_event_loop: () => {
    throw 'unwind';
  },

  $safeSetTimeout__deps: ['$callUserCallback'],
  $safeSetTimeout__docs: '/** @param {number=} timeout */',
  $safeSetTimeout: (func, timeout) => {
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(() => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(func);
    }, timeout);
  },

  // Just like setImmediate but returns an i32 that can be passed back
  // to wasm rather than a JS object.
  $setImmediateWrapped: (func) => {
    if (!setImmediateWrapped.mapping) setImmediateWrapped.mapping = [];
    var id = setImmediateWrapped.mapping.length;
    setImmediateWrapped.mapping[id] = setImmediate(() => {
      setImmediateWrapped.mapping[id] = undefined;
      func();
    });
    return id;
  },

  // Just like clearImmediate but takes an i32 rather than an object.
  $clearImmediateWrapped: (id) => {
#if ASSERTIONS
    assert(id);
    assert(setImmediateWrapped.mapping[id]);
#endif
    clearImmediate(setImmediateWrapped.mapping[id]);
    setImmediateWrapped.mapping[id] = undefined;
  },

  $polyfillSetImmediate__deps: ['$setImmediateWrapped', '$clearImmediateWrapped'],
  $polyfillSetImmediate__postset: `
    var emSetImmediate;
    var emClearImmediate;
    if (typeof setImmediate != "undefined") {
      emSetImmediate = setImmediateWrapped;
      emClearImmediate = clearImmediateWrapped;
    } else if (typeof addEventListener == "function") {
      var __setImmediate_id_counter = 0;
      var __setImmediate_queue = [];
      var __setImmediate_message_id = "_si";
      /** @param {Event} e */
      var __setImmediate_cb = (e) => {
        if (e.data === __setImmediate_message_id) {
          e.stopPropagation();
          __setImmediate_queue.shift()();
          ++__setImmediate_id_counter;
        }
      }
      addEventListener("message", __setImmediate_cb, true);
      emSetImmediate = (func) => {
        postMessage(__setImmediate_message_id, "*");
        return __setImmediate_id_counter + __setImmediate_queue.push(func) - 1;
      }
      emClearImmediate = /**@type{function(number=)}*/((id) => {
        var index = id - __setImmediate_id_counter;
        // must preserve the order and count of elements in the queue, so replace the pending callback with an empty function
        if (index >= 0 && index < __setImmediate_queue.length) __setImmediate_queue[index] = () => {};
      })
    }`,

  $polyfillSetImmediate: () => {
    // nop, used for its postset to ensure setImmediate() polyfill is
    // not duplicated between emscripten_set_immediate() and
    // emscripten_set_immediate_loop() if application links to both of them.
  },

  emscripten_set_immediate__deps: ['$polyfillSetImmediate', '$callUserCallback'],
  emscripten_set_immediate: (cb, userData) => {
    {{{ runtimeKeepalivePush(); }}}
    return emSetImmediate(() => {
      {{{ runtimeKeepalivePop(); }}}
      callUserCallback(() => {{{ makeDynCall('vp', 'cb') }}}(userData));
    });
  },

  emscripten_clear_immediate__deps: ['$polyfillSetImmediate'],
  emscripten_clear_immediate: (id) => {
    {{{ runtimeKeepalivePop(); }}}
    emClearImmediate(id);
  },

  emscripten_set_immediate_loop__deps: ['$polyfillSetImmediate', '$callUserCallback'],
  emscripten_set_immediate_loop: (cb, userData) => {
    function tick() {
      callUserCallback(() => {
        if ({{{ makeDynCall('ip', 'cb') }}}(userData)) {
          emSetImmediate(tick);
        } else {
          {{{ runtimeKeepalivePop(); }}}
        }
      });
    }
    {{{ runtimeKeepalivePush(); }}}
    emSetImmediate(tick);
  },

  emscripten_set_timeout__deps: ['$safeSetTimeout'],
  emscripten_set_timeout: (cb, msecs, userData) =>
    safeSetTimeout(() => {{{ makeDynCall('vp', 'cb') }}}(userData), msecs),

  emscripten_clear_timeout: (id) => clearTimeout(id),

  emscripten_set_timeout_loop__deps: ['$callUserCallback', 'emscripten_get_now'],
  emscripten_set_timeout_loop: (cb, msecs, userData) => {
    function tick() {
      var t = _emscripten_get_now();
      var n = t + msecs;
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {
        if ({{{ makeDynCall('idp', 'cb') }}}(t, userData)) {
          // Save a little bit of code space: modern browsers should treat
          // negative setTimeout as timeout of 0
          // (https://stackoverflow.com/questions/8430966/is-calling-settimeout-with-a-negative-delay-ok)
          {{{ runtimeKeepalivePush() }}}
          setTimeout(tick, n - _emscripten_get_now());
        }
      });
    }
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(tick, 0);
  },

  emscripten_set_interval__deps: ['$callUserCallback'],
  emscripten_set_interval: (cb, msecs, userData) => {
    {{{ runtimeKeepalivePush() }}}
    return setInterval(() => {
      callUserCallback(() => {{{ makeDynCall('vp', 'cb') }}}(userData));
    }, msecs);
  },

  emscripten_clear_interval: (id) => {
    {{{ runtimeKeepalivePop() }}}
    clearInterval(id);
  },
};

addToLibrary(LibraryJSEventLoop);
