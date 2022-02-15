/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Implementation of functions from emscripten/eventloop.h.

LibraryJSEventLoop = {
  emscripten_unwind_to_js_event_loop: function() {
    throw 'unwind';
  },

  // Just like setImmediate but returns an i32 that can be passed back
  // to wasm rather than a JS object.
  $setImmediateWrapped: function(func) {
    if (!setImmediateWrapped.mapping) setImmediateWrapped.mapping = [];
    var id = setImmediateWrapped.mapping.length;
    setImmediateWrapped.mapping[id] = setImmediate(function() {
      setImmediateWrapped.mapping[id] = undefined;
      func();
    });
    return id;
  },

  // Just like clearImmediate but takes an i32 rather than an object.
  $clearImmediateWrapped: function(id) {
#if ASSERTIONS
    assert(id);
    assert(setImmediateWrapped.mapping[id]);
#endif
    clearImmediate(setImmediateWrapped.mapping[id]);
    setImmediateWrapped.mapping[id] = undefined;
  },

  $polyfillSetImmediate__deps: ['$setImmediateWrapped', '$clearImmediateWrapped'],
  $polyfillSetImmediate__postset:
    'var emSetImmediate;\n' +
    'var emClearImmediate;\n' +
    'if (typeof setImmediate != "undefined") {\n' +
      'emSetImmediate = setImmediateWrapped;\n' +
      'emClearImmediate = clearImmediateWrapped;\n' +
    '} else if (typeof addEventListener == "function") {\n' +
      'var __setImmediate_id_counter = 0;\n' +
      'var __setImmediate_queue = [];\n' +
      'var __setImmediate_message_id = "_si";\n' +
      'function __setImmediate_cb(/** @type {Event} */e) {\n' +
        'if (e.data === __setImmediate_message_id) {\n' +
          'e.stopPropagation();\n' +
          '__setImmediate_queue.shift()();\n' +
          '++__setImmediate_id_counter;\n' +
        '}\n' +
      '}\n' +
      'addEventListener("message", __setImmediate_cb, true);\n' +
      'emSetImmediate = function(func) {\n' +
        'postMessage(__setImmediate_message_id, "*");\n' +
        'return __setImmediate_id_counter + __setImmediate_queue.push(func) - 1;\n' +
      '}\n' +
      'emClearImmediate = /**@type{function(number=)}*/(function(id) {\n' +
        'var index = id - __setImmediate_id_counter;\n' +
        'if (index >= 0 && index < __setImmediate_queue.length) __setImmediate_queue[index] = function() {};\n' + // must preserve the order and count of elements in the queue, so replace the pending callback with an empty function
      '})\n' +
    '}',

  $polyfillSetImmediate: function() {
    // nop, used for its postset to ensure setImmediate() polyfill is
    // not duplicated between emscripten_set_immediate() and
    // emscripten_set_immediate_loop() if application links to both of them.
  },

  emscripten_set_immediate__deps: ['$polyfillSetImmediate', '$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_set_immediate: function(cb, userData) {
    polyfillSetImmediate();
    {{{ runtimeKeepalivePush(); }}}
    return emSetImmediate(function() {
      {{{ runtimeKeepalivePop(); }}}
      callUserCallback(function() {
        {{{ makeDynCall('vi', 'cb') }}}(userData);
      });
    });
  },

  emscripten_clear_immediate__deps: ['$polyfillSetImmediate',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePop',
#endif
  ],
  emscripten_clear_immediate: function(id) {
    {{{ runtimeKeepalivePop(); }}}
    emClearImmediate(id);
  },

  emscripten_set_immediate_loop__deps: ['$polyfillSetImmediate', '$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_set_immediate_loop: function(cb, userData) {
    polyfillSetImmediate();
    function tick() {
      {{{ runtimeKeepalivePop(); }}}
      callUserCallback(function() {
        if ({{{ makeDynCall('ii', 'cb') }}}(userData)) {
          {{{ runtimeKeepalivePush(); }}}
          emSetImmediate(tick);
        }
      });
    }
    {{{ runtimeKeepalivePush(); }}}
    return emSetImmediate(tick);
  },

  emscripten_set_timeout__deps: ['$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_set_timeout: function(cb, msecs, userData) {
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        {{{ makeDynCall('vi', 'cb') }}}(userData);
      });
    }, msecs);
  },

  emscripten_clear_timeout: function(id) {
    clearTimeout(id);
  },

  emscripten_set_timeout_loop__deps: ['$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_set_timeout_loop: function(cb, msecs, userData) {
    function tick() {
      var t = performance.now();
      var n = t + msecs;
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        if ({{{ makeDynCall('idi', 'cb') }}}(t, userData)) {
          // Save a little bit of code space: modern browsers should treat
          // negative setTimeout as timeout of 0
          // (https://stackoverflow.com/questions/8430966/is-calling-settimeout-with-a-negative-delay-ok)
          {{{ runtimeKeepalivePush() }}}
          setTimeout(tick, n - performance.now());
        }
      });
    }
    {{{ runtimeKeepalivePush() }}}
    return setTimeout(tick, 0);
  },

  emscripten_set_interval__deps: ['$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_set_interval: function(cb, msecs, userData) {
    {{{ runtimeKeepalivePush() }}}
    return setInterval(function() {
      callUserCallback(function() {
        {{{ makeDynCall('vi', 'cb') }}}(userData)
      });
    }, msecs);
  },

#if !MINIMAL_RUNTIME
  emscripten_clear_interval__deps: ['$runtimeKeepalivePop'],
#endif
  emscripten_clear_interval: function(id) {
    {{{ runtimeKeepalivePop() }}}
    clearInterval(id);
  },
};

mergeInto(LibraryManager.library, LibraryJSEventLoop);
