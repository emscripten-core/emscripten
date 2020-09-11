/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Safe timers
var LibrarySafeTimers = {
  $SafeTimers__postset: 'Module["requestAnimationFrame"] = function Module_requestAnimationFrame(func) { SafeTimers.requestAnimationFrame(func) };\n',
  $SafeTimers: {
    // abort and pause-aware versions TODO: build main loop on top of this?

    allowAsyncCallbacks: true,
    queuedAsyncCallbacks: [],

    pauseAsyncCallbacks: function() {
      SafeTimers.allowAsyncCallbacks = false;
    },

    resumeAsyncCallbacks: function() { // marks future callbacks as ok to execute, and synchronously runs any remaining ones right now
      SafeTimers.allowAsyncCallbacks = true;
      if (SafeTimers.queuedAsyncCallbacks.length > 0) {
        var callbacks = SafeTimers.queuedAsyncCallbacks;
        SafeTimers.queuedAsyncCallbacks = [];
        callbacks.forEach(function(func) {
          func();
        });
      }
    },

    safeRequestAnimationFrame: function(func) {
      return SafeTimers.requestAnimationFrame(function() {
        if (ABORT) return;
        if (SafeTimers.allowAsyncCallbacks) {
          func();
        } else {
          SafeTimers.queuedAsyncCallbacks.push(func);
        }
      });
    },

    safeSetTimeout: function(func, timeout) {
      noExitRuntime = true;
      return setTimeout(function() {
        if (ABORT) return;
        if (SafeTimers.allowAsyncCallbacks) {
          func();
        } else {
          SafeTimers.queuedAsyncCallbacks.push(func);
        }
      }, timeout);
    },

    safeSetInterval: function(func, timeout) {
      noExitRuntime = true;
      return setInterval(function() {
        if (ABORT) return;
        if (SafeTimers.allowAsyncCallbacks) {
          func();
        } // drop it on the floor otherwise, next interval will kick in
      }, timeout);
    },

    nextRAF: 0,

    fakeRequestAnimationFrame: function(func) {
      // try to keep 60fps between calls to here
      var now = Date.now();
      if (SafeTimers.nextRAF === 0) {
        SafeTimers.nextRAF = now + 1000/60;
      } else {
        while (now + 2 >= SafeTimers.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
          SafeTimers.nextRAF += 1000/60;
        }
      }
      var delay = Math.max(SafeTimers.nextRAF - now, 0);
      setTimeout(func, delay);
    },

    requestAnimationFrame: function(func) {
      if (typeof requestAnimationFrame === 'function') {
        requestAnimationFrame(func);
        return;
      }
      var RAF = SafeTimers.fakeRequestAnimationFrame;
#if LEGACY_VM_SUPPORT
      if (typeof window !== 'undefined') {
        RAF = window['requestAnimationFrame'] ||
              window['mozRequestAnimationFrame'] ||
              window['webkitRequestAnimationFrame'] ||
              window['msRequestAnimationFrame'] ||
              window['oRequestAnimationFrame'] ||
              RAF;
      }
#endif
      RAF(func);
    },
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_async_call: function(func, arg, millis) {
    noExitRuntime = true;

    function wrapper() {
      {{{ makeDynCall('vi', 'func') }}}(arg);
    }

    if (millis >= 0) {
      SafeTimers.safeSetTimeout(wrapper, millis);
    } else {
      SafeTimers.safeRequestAnimationFrame(wrapper);
    }
  },
};

autoAddDeps(LibrarySafeTimers, '$SafeTimers');

mergeInto(LibraryManager.library, LibrarySafeTimers);