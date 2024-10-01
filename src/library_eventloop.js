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
    setImmediateWrapped.mapping ||= [];
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

  emscripten_clear_timeout: 'clearTimeout',

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

  $registerPostMainLoop: (f) => {
    // Does nothing unless $MainLoop is included/used.
    typeof MainLoop != 'undefined' && MainLoop.postMainLoop.push(f);
  },

  $registerPreMainLoop: (f) => {
    // Does nothing unless $MainLoop is included/used.
    typeof MainLoop != 'undefined' && MainLoop.preMainLoop.push(f);
  },

  $MainLoop__internal: true,
  $MainLoop__deps: ['$setMainLoop', '$callUserCallback', 'emscripten_set_main_loop_timing'],
  $MainLoop__postset: `
    Module["requestAnimationFrame"] = MainLoop.requestAnimationFrame;
    Module["pauseMainLoop"] = MainLoop.pause;
    Module["resumeMainLoop"] = MainLoop.resume;
    MainLoop.init();`,
  $MainLoop: {
    running: false,
    scheduler: null,
    method: '',
    // Each main loop is numbered with a ID in sequence order. Only one main
    // loop can run at a time. This variable stores the ordinal number of the
    // main loop that is currently allowed to run. All previous main loops
    // will quit themselves. This is incremented whenever a new main loop is
    // created.
    currentlyRunningMainloop: 0,
    // The main loop tick function that will be called at each iteration.
    func: null,
    // The argument that will be passed to the main loop. (of type void*)
    arg: 0,
    timingMode: 0,
    timingValue: 0,
    currentFrameNumber: 0,
    queue: [],
    preMainLoop: [],
    postMainLoop: [],

    pause() {
      MainLoop.scheduler = null;
      // Incrementing this signals the previous main loop that it's now become old, and it must return.
      MainLoop.currentlyRunningMainloop++;
    },

    resume() {
      MainLoop.currentlyRunningMainloop++;
      var timingMode = MainLoop.timingMode;
      var timingValue = MainLoop.timingValue;
      var func = MainLoop.func;
      MainLoop.func = null;
      // do not set timing and call scheduler, we will do it on the next lines
      setMainLoop(func, 0, false, MainLoop.arg, true);
      _emscripten_set_main_loop_timing(timingMode, timingValue);
      MainLoop.scheduler();
    },

    updateStatus() {
#if expectToReceiveOnModule('setStatus')
      if (Module['setStatus']) {
        var message = Module['statusMessage'] || 'Please wait...';
        var remaining = MainLoop.remainingBlockers ?? 0;
        var expected = MainLoop.expectedBlockers ?? 0;
        if (remaining) {
          if (remaining < expected) {
            Module['setStatus'](`{message} ({expected - remaining}/{expected})`);
          } else {
            Module['setStatus'](message);
          }
        } else {
          Module['setStatus']('');
        }
      }
#endif
    },

    init() {
#if expectToReceiveOnModule('preMainLoop')
      Module['preMainLoop'] && MainLoop.preMainLoop.push(Module['preMainLoop']);
#endif
#if expectToReceiveOnModule('postMainLoop')
      Module['postMainLoop'] && MainLoop.postMainLoop.push(Module['postMainLoop']);
#endif
    },

    runIter(func) {
      if (ABORT) return;
      for (var pre of MainLoop.preMainLoop) {
        if (pre() === false) {
          return; // |return false| skips a frame
        }
      }
      callUserCallback(func);
      for (var post of MainLoop.postMainLoop) {
        post();
      }
#if STACK_OVERFLOW_CHECK
      checkStackCookie();
#endif
    },

    nextRAF: 0,

    fakeRequestAnimationFrame(func) {
      // try to keep 60fps between calls to here
      var now = Date.now();
      if (MainLoop.nextRAF === 0) {
        MainLoop.nextRAF = now + 1000/60;
      } else {
        while (now + 2 >= MainLoop.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
          MainLoop.nextRAF += 1000/60;
        }
      }
      var delay = Math.max(MainLoop.nextRAF - now, 0);
      setTimeout(func, delay);
    },

    requestAnimationFrame(func) {
      if (typeof requestAnimationFrame == 'function') {
        requestAnimationFrame(func);
        return;
      }
      var RAF = MainLoop.fakeRequestAnimationFrame;
#if LEGACY_VM_SUPPORT
      if (typeof window != 'undefined') {
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

  emscripten_get_main_loop_timing__deps: ['$MainLoop'],
  emscripten_get_main_loop_timing: (mode, value) => {
    if (mode) {{{ makeSetValue('mode', 0, 'MainLoop.timingMode', 'i32') }}};
    if (value) {{{ makeSetValue('value', 0, 'MainLoop.timingValue', 'i32') }}};
  },

  emscripten_set_main_loop_timing__deps: ['$MainLoop'],
  emscripten_set_main_loop_timing: (mode, value) => {
    MainLoop.timingMode = mode;
    MainLoop.timingValue = value;

    if (!MainLoop.func) {
#if ASSERTIONS
      err('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
#endif
      return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
    }

    if (!MainLoop.running) {
      {{{ runtimeKeepalivePush() }}}
      MainLoop.running = true;
    }
    if (mode == {{{ cDefs.EM_TIMING_SETTIMEOUT }}}) {
      MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
        var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now())|0;
        setTimeout(MainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
      };
      MainLoop.method = 'timeout';
    } else if (mode == {{{ cDefs.EM_TIMING_RAF }}}) {
      MainLoop.scheduler = function MainLoop_scheduler_rAF() {
        MainLoop.requestAnimationFrame(MainLoop.runner);
      };
      MainLoop.method = 'rAF';
    } else if (mode == {{{ cDefs.EM_TIMING_SETIMMEDIATE}}}) {
      if (typeof MainLoop.setImmediate == 'undefined') {
        if (typeof setImmediate == 'undefined') {
          // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
          var setImmediates = [];
          var emscriptenMainLoopMessageId = 'setimmediate';
          /** @param {Event} event */
          var MainLoop_setImmediate_messageHandler = (event) => {
            // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
            // so check for both cases.
            if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
              event.stopPropagation();
              setImmediates.shift()();
            }
          };
          addEventListener("message", MainLoop_setImmediate_messageHandler, true);
          MainLoop.setImmediate = /** @type{function(function(): ?, ...?): number} */((func) => {
            setImmediates.push(func);
            if (ENVIRONMENT_IS_WORKER) {
              Module['setImmediates'] ??= [];
              Module['setImmediates'].push(func);
              postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
            } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
          });
        } else {
          MainLoop.setImmediate = setImmediate;
        }
      }
      MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
        MainLoop.setImmediate(MainLoop.runner);
      };
      MainLoop.method = 'immediate';
    }
    return 0;
  },

  emscripten_set_main_loop__deps: ['$setMainLoop'],
  emscripten_set_main_loop: (func, fps, simulateInfiniteLoop) => {
    var iterFunc = {{{ makeDynCall('v', 'func') }}};
    setMainLoop(iterFunc, fps, simulateInfiniteLoop);
  },

  $setMainLoop__internal: true,
  $setMainLoop__deps: [
    '$MainLoop',
    'emscripten_set_main_loop_timing', 'emscripten_get_now',
#if OFFSCREEN_FRAMEBUFFER
    'emscripten_webgl_commit_frame',
#endif
#if !MINIMAL_RUNTIME
    '$maybeExit',
#endif
  ],
  $setMainLoop__docs: `
  /**
   * @param {number=} arg
   * @param {boolean=} noSetTiming
   */`,
  $setMainLoop: (iterFunc, fps, simulateInfiniteLoop, arg, noSetTiming) => {
#if ASSERTIONS
    assert(!MainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');
#endif
    MainLoop.func = iterFunc;
    MainLoop.arg = arg;

    var thisMainLoopId = MainLoop.currentlyRunningMainloop;
    function checkIsRunning() {
      if (thisMainLoopId < MainLoop.currentlyRunningMainloop) {
#if RUNTIME_DEBUG
        dbg('main loop exiting');
#endif
        {{{ runtimeKeepalivePop() }}}
#if !MINIMAL_RUNTIME
        maybeExit();
#endif
        return false;
      }
      return true;
    }

    // We create the loop runner here but it is not actually running until
    // _emscripten_set_main_loop_timing is called (which might happen a
    // later time).  This member signifies that the current runner has not
    // yet been started so that we can call runtimeKeepalivePush when it
    // gets it timing set for the first time.
    MainLoop.running = false;
    MainLoop.runner = function MainLoop_runner() {
      if (ABORT) return;
      if (MainLoop.queue.length > 0) {
        var start = Date.now();
        var blocker = MainLoop.queue.shift();
        blocker.func(blocker.arg);
        if (MainLoop.remainingBlockers) {
          var remaining = MainLoop.remainingBlockers;
          var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
          if (blocker.counted) {
            MainLoop.remainingBlockers = next;
          } else {
            // not counted, but move the progress along a tiny bit
            next = next + 0.5; // do not steal all the next one's progress
            MainLoop.remainingBlockers = (8*remaining + next)/9;
          }
        }
#if RUNTIME_DEBUG
        dbg(`main loop blocker "${blocker.name}" took '${Date.now() - start} ms`); //, left: ' + MainLoop.remainingBlockers);
#endif
        MainLoop.updateStatus();

        // catches pause/resume main loop from blocker execution
        if (!checkIsRunning()) return;

        setTimeout(MainLoop.runner, 0);
        return;
      }

      // catch pauses from non-main loop sources
      if (!checkIsRunning()) return;

      // Implement very basic swap interval control
      MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
      if (MainLoop.timingMode == {{{ cDefs.EM_TIMING_RAF }}} && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
        // Not the scheduled time to render this frame - skip.
        MainLoop.scheduler();
        return;
      } else if (MainLoop.timingMode == {{{ cDefs.EM_TIMING_SETTIMEOUT }}}) {
        MainLoop.tickStartTime = _emscripten_get_now();
      }

#if ASSERTIONS
      if (MainLoop.method === 'timeout' && Module.ctx) {
        warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
        MainLoop.method = ''; // just warn once per call to set main loop
      }
#endif

      MainLoop.runIter(iterFunc);

      // catch pauses from the main loop itself
      if (!checkIsRunning()) return;

      MainLoop.scheduler();
    }

    if (!noSetTiming) {
      if (fps && fps > 0) {
        _emscripten_set_main_loop_timing({{{ cDefs.EM_TIMING_SETTIMEOUT }}}, 1000.0 / fps);
      } else {
        // Do rAF by rendering each frame (no decimating)
        _emscripten_set_main_loop_timing({{{ cDefs.EM_TIMING_RAF }}}, 1);
      }

      MainLoop.scheduler();
    }

    if (simulateInfiniteLoop) {
      throw 'unwind';
    }
  },

  emscripten_set_main_loop_arg__deps: ['$setMainLoop'],
  emscripten_set_main_loop_arg: (func, arg, fps, simulateInfiniteLoop) => {
    var iterFunc = () => {{{ makeDynCall('vp', 'func') }}}(arg);
    setMainLoop(iterFunc, fps, simulateInfiniteLoop, arg);
  },

  emscripten_cancel_main_loop__deps: ['$MainLoop'],
  emscripten_cancel_main_loop: () => {
    MainLoop.pause();
    MainLoop.func = null;
  },

  emscripten_pause_main_loop__deps: ['$MainLoop'],
  emscripten_pause_main_loop: () => {
    MainLoop.pause();
  },

  emscripten_resume_main_loop__deps: ['$MainLoop'],
  emscripten_resume_main_loop: () => {
    MainLoop.resume();
  },

  _emscripten_push_main_loop_blocker__deps: ['$MainLoop'],
  _emscripten_push_main_loop_blocker: (func, arg, name) => {
    MainLoop.queue.push({ func: () => {
      {{{ makeDynCall('vp', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: true });
    MainLoop.updateStatus();
  },

  _emscripten_push_uncounted_main_loop_blocker__deps: ['$MainLoop'],
  _emscripten_push_uncounted_main_loop_blocker: (func, arg, name) => {
    MainLoop.queue.push({ func: () => {
      {{{ makeDynCall('vp', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: false });
    MainLoop.updateStatus();
  },

  emscripten_set_main_loop_expected_blockers__deps: ['$MainLoop'],
  emscripten_set_main_loop_expected_blockers: (num) => {
    MainLoop.expectedBlockers = num;
    MainLoop.remainingBlockers = num;
    MainLoop.updateStatus();
  },

};

addToLibrary(LibraryJSEventLoop);
