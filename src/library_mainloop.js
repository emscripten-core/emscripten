/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// MainLoop system
var LibraryMainLoop = {
  $MainLoop__deps: ['$setMainLoop', 'emscripten_set_main_loop_timing'],
  $MainLoop__postset: 'Module["pauseMainLoop"] = function Module_pauseMainLoop() { MainLoop.pause() };\n' +
                      'Module["resumeMainLoop"] = function Module_resumeMainLoop() { MainLoop.resume() };\n',

  $MainLoop: {
    scheduler: null,
    method: '',
    // Each main loop is numbered with a ID in sequence order. Only one main loop can run at a time. This variable stores the ordinal number of the main loop that is currently
    // allowed to run. All previous main loops will quit themselves. This is incremented whenever a new main loop is created.
    /** @type{number} */
    currentlyRunningMainloop: 0,
    func: null, // The main loop tick function that will be called at each iteration.
    arg: 0, // The argument that will be passed to the main loop. (of type void*)
    timingMode: 0,
    timingValue: 0,
    currentFrameNumber: 0,
    queue: [],
    pause: function() {
      MainLoop.scheduler = null;
      MainLoop.currentlyRunningMainloop++; // Incrementing this signals the previous main loop that it's now become old, and it must return.
    },
    resume: function() {
      MainLoop.currentlyRunningMainloop++;
      var timingMode = MainLoop.timingMode;
      var timingValue = MainLoop.timingValue;
      var func = MainLoop.func;
      MainLoop.func = null;
      setMainLoop(func, 0, false, MainLoop.arg, true /* do not set timing and call scheduler, we will do it on the next lines */);
      _emscripten_set_main_loop_timing(timingMode, timingValue);
      MainLoop.scheduler();
    },
    updateStatus: function() {
      if (Module['setStatus']) {
        var message = Module['statusMessage'] || 'Please wait...';
        var remaining = MainLoop.remainingBlockers;
        var expected = MainLoop.expectedBlockers;
        if (remaining) {
          if (remaining < expected) {
            Module['setStatus'](message + ' (' + (expected - remaining) + '/' + expected + ')');
          } else {
            Module['setStatus'](message);
          }
        } else {
          Module['setStatus']('');
        }
      }
    },
    runIter: function(func) {
      if (ABORT) return;
      if (Module['preMainLoop']) {
        var preRet = Module['preMainLoop']();
        if (preRet === false) {
          return; // |return false| skips a frame
        }
      }
      try {
        func();
      } catch (e) {
        if (e instanceof ExitStatus) {
          return;
        } else if (e == 'unwind') {
          return;
        } else {
          if (e && typeof e === 'object' && e.stack) err('exception thrown: ' + [e, e.stack]);
          throw e;
        }
      }
      if (Module['postMainLoop']) Module['postMainLoop']();
    },
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_get_main_loop_timing: function(mode, value) {
    if (mode) {{{ makeSetValue('mode', 0, 'MainLoop.timingMode', 'i32') }}};
    if (value) {{{ makeSetValue('value', 0, 'MainLoop.timingValue', 'i32') }}};
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_timing__deps: ['$SafeTimers'],
  emscripten_set_main_loop_timing: function(mode, value) {
    MainLoop.timingMode = mode;
    MainLoop.timingValue = value;

    if (!MainLoop.func) {
#if ASSERTIONS
      console.error('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call   emscripten_set_main_loop first to set one up.');
#endif
      return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
    }

    if (mode == 0 /*EM_TIMING_SETTIMEOUT*/) {
      MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
        var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now())|0;
        setTimeout(MainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
      };
      MainLoop.method = 'timeout';
    } else if (mode == 1 /*EM_TIMING_RAF*/) {
      MainLoop.scheduler = function MainLoop_scheduler_rAF() {
        SafeTimers.requestAnimationFrame(MainLoop.runner);
      };
      MainLoop.method = 'rAF';
    } else if (mode == 2 /*EM_TIMING_SETIMMEDIATE*/) {
      if (typeof setImmediate === 'undefined') {
        // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
        var setImmediates = [];
        var emscriptenMainLoopMessageId = 'setimmediate';
        var MainLoop_setImmediate_messageHandler = function(event) {
          // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
          // so check for both cases.
          if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
            event.stopPropagation();
            setImmediates.shift()();
          }
        }
        addEventListener("message", MainLoop_setImmediate_messageHandler, true);
        setImmediate = /** @type{function(function(): ?, ...?): number} */(function MainLoop_emulated_setImmediate(func) {
          setImmediates.push(func);
          if (ENVIRONMENT_IS_WORKER) {
            if (Module['setImmediates'] === undefined) Module['setImmediates'] = [];
            Module['setImmediates'].push(func);
            postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
          } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
        })
      }
      MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
        setImmediate(MainLoop.runner);
      };
      MainLoop.method = 'immediate';
    }
    return 0;
  },

  emscripten_set_main_loop__deps: ['$setMainLoop'],
  emscripten_set_main_loop__docs: '/** @param {number|boolean=} noSetTiming */',
  emscripten_set_main_loop: function(func, fps, simulateInfiniteLoop, arg, noSetTiming) {
    var MainLoopIterationFunc = function() { {{{ makeDynCall('v', 'func') }}}(); };
    setMainLoop(MainLoopIterationFunc, fps, simulateInfiniteLoop, arg, noSetTiming);
  },

  // Runs natively in pthread, no __proxy needed.
#if OFFSCREEN_FRAMEBUFFER
  $setMainLoop__deps: ['emscripten_set_main_loop_timing', 'emscripten_get_now', 'emscripten_webgl_commit_frame'],
#else
  $setMainLoop__deps: ['emscripten_set_main_loop_timing', 'emscripten_get_now'],
#endif
  $setMainLoop: function(MainLoopIterationFunc, fps, simulateInfiniteLoop, arg, noSetTiming) {
    noExitRuntime = true;

    assert(!MainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');

    MainLoop.func = MainLoopIterationFunc;
    MainLoop.arg = arg;

#if USE_CLOSURE_COMPILER
    // Closure compiler bug(?): Closure does not see that the assignment
    //   var thisMainLoopId = MainLoop.currentlyRunningMainloop
    // is a value copy of a number (even with the JSDoc @type annotation)
    // but optimizeis the code as if the assignment was a reference assignment,
    // which results in MainLoop.pause() not working. Hence use a
    // workaround to make Closure believe this is a value copy that should occur:
    // (TODO: Minimize this down to a small test case and report - was unable
    // to reproduce in a small written test case)
    /** @type{number} */
    var thisMainLoopId = (function(){return MainLoop.currentlyRunningMainloop; })();
#else
    var thisMainLoopId = MainLoop.currentlyRunningMainloop;
#endif

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
        console.log('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + ' ms'); //, left: ' + MainLoop.remainingBlockers);
        MainLoop.updateStatus();

        // catches pause/resume main loop from blocker execution
        if (thisMainLoopId < MainLoop.currentlyRunningMainloop) return;

        setTimeout(MainLoop.runner, 0);
        return;
      }

      // catch pauses from non-main loop sources
      if (thisMainLoopId < MainLoop.currentlyRunningMainloop) return;

      // Implement very basic swap interval control
      MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
      if (MainLoop.timingMode == 1/*EM_TIMING_RAF*/ && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
        // Not the scheduled time to render this frame - skip.
        MainLoop.scheduler();
        return;
      } else if (MainLoop.timingMode == 0/*EM_TIMING_SETTIMEOUT*/) {
        MainLoop.tickStartTime = _emscripten_get_now();
      }

      // Signal GL rendering layer that processing of a new frame is about to start. This helps it optimize
      // VBO double-buffering and reduce GPU stalls.
#if FULL_ES2 || LEGACY_GL_EMULATION
      GL.newRenderingFrameStarted();
#endif

#if USE_PTHREADS && OFFSCREEN_FRAMEBUFFER && GL_SUPPORT_EXPLICIT_SWAP_CONTROL
      // If the current GL context is a proxied regular WebGL context, and was initialized with implicit swap mode on the main thread, and we are on the parent thread,
      // perform the swap on behalf of the user.
      if (typeof GL !== 'undefined' && GL.currentContext && GL.currentContextIsProxied) {
        var explicitSwapControl = {{{ makeGetValue('GL.currentContext', 0, 'i32') }}};
        if (!explicitSwapControl) _emscripten_webgl_commit_frame();
      }
#endif

#if OFFSCREENCANVAS_SUPPORT
      // If the current GL context is an OffscreenCanvas, but it was initialized with implicit swap mode, perform the swap on behalf of the user.
      if (typeof GL !== 'undefined' && GL.currentContext && !GL.currentContextIsProxied && !GL.currentContext.attributes.explicitSwapControl && GL.currentContext.GLctx.commit) {
        GL.currentContext.GLctx.commit();
      }
#endif

#if ASSERTIONS
      if (MainLoop.method === 'timeout' && Module.ctx) {
        warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
        MainLoop.method = ''; // just warn once per call to set main loop
      }
#endif

      MainLoop.runIter(MainLoopIterationFunc);

#if STACK_OVERFLOW_CHECK
      checkStackCookie();
#endif

      // catch pauses from the main loop itself
      if (thisMainLoopId < MainLoop.currentlyRunningMainloop) return;

      // Queue new audio data. This is important to be right after the main loop invocation, so that we will immediately be able
      // to queue the newest produced audio samples.
      // TODO: Consider adding pre- and post- rAF callbacks so that GL.newRenderingFrameStarted() and SDL.audio.queueNewAudioData()
      //       do not need to be hardcoded into this function, but can be more generic.
      if (typeof SDL === 'object' && SDL.audio && SDL.audio.queueNewAudioData) SDL.audio.queueNewAudioData();

      MainLoop.scheduler();
    }

    if (!noSetTiming) {
      if (fps && fps > 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 1000.0 / fps);
      else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, 1); // Do rAF by rendering each frame (no decimating)

      MainLoop.scheduler();
    }

    if (simulateInfiniteLoop) {
      throw 'unwind';
    }
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_arg__deps: ['$setMainLoop'],
  emscripten_set_main_loop_arg: function(func, arg, fps, simulateInfiniteLoop) {
    var MainLoopIterationFunc = function() { {{{ makeDynCall('vi', 'func') }}}(arg); };
    setMainLoop(MainLoopIterationFunc, fps, simulateInfiniteLoop, arg);
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_cancel_main_loop: function() {
    MainLoop.pause();
    MainLoop.func = null;
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_pause_main_loop: function() {
    MainLoop.pause();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_resume_main_loop: function() {
    MainLoop.resume();
  },

  // Runs natively in pthread, no __proxy needed.
  _emscripten_push_main_loop_blocker: function(func, arg, name) {
    MainLoop.queue.push({ func: function() {
      {{{ makeDynCall('vi', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: true });
    MainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  _emscripten_push_uncounted_main_loop_blocker: function(func, arg, name) {
    MainLoop.queue.push({ func: function() {
      {{{ makeDynCall('vi', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: false });
    MainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_expected_blockers: function(num) {
    MainLoop.expectedBlockers = num;
    MainLoop.remainingBlockers = num;
    MainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_async_call__deps: ['$SafeTimers'],
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

autoAddDeps(LibraryMainLoop, '$MainLoop');

mergeInto(LibraryManager.library, LibraryMainLoop);