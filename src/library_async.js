/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//
// Async support via ASYNCIFY
//

mergeInto(LibraryManager.library, {
  // error handling

  $runAndAbortIfError: function(func) {
    try {
      return func();
    } catch (e) {
      abort(e);
    }
  },

#if ASYNCIFY
  $Asyncify__deps: ['$runAndAbortIfError', '$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop'
#endif
  ],
  $Asyncify: {
    State: {
      Normal: 0,
      Unwinding: 1,
      Rewinding: 2,
      Disabled: 3,
    },
    state: 0,
    StackSize: {{{ ASYNCIFY_STACK_SIZE }}},
    currData: null,
    // The return value passed to wakeUp() in
    // Asyncify.handleSleep(function(wakeUp){...}) is stored here,
    // so we can return it later from the C function that called
    // Asyncify.handleSleep() after rewinding finishes.
    handleSleepReturnValue: 0,
    // We must track which wasm exports are called into and
    // exited, so that we know where the call stack began,
    // which is where we must call to rewind it.
    exportCallStack: [],
    callStackNameToId: {},
    callStackIdToName: {},
    callStackId: 0,
    asyncPromiseHandlers: null, // { resolve, reject } pair for when *all* asynchronicity is done
    sleepCallbacks: [], // functions to call every time we sleep

    getCallStackId: function(funcName) {
      var id = Asyncify.callStackNameToId[funcName];
      if (id === undefined) {
        id = Asyncify.callStackId++;
        Asyncify.callStackNameToId[funcName] = id;
        Asyncify.callStackIdToName[id] = funcName;
      }
      return id;
    },

#if ASSERTIONS
    instrumentWasmImports: function(imports) {
      var ASYNCIFY_IMPORTS = {{{ JSON.stringify(ASYNCIFY_IMPORTS) }}}.map((x) => x.split('.')[1]);
      for (var x in imports) {
        (function(x) {
          var original = imports[x];
          if (typeof original === 'function') {
            imports[x] = function() {
              var originalAsyncifyState = Asyncify.state;
              try {
                return original.apply(null, arguments);
              } finally {
                // Only asyncify-declared imports are allowed to change the
                // state.
                var isAsyncifyImport = ASYNCIFY_IMPORTS.indexOf(x) >= 0 ||
                                       x.startsWith('__asyncjs__');
                // Changing the state from normal to disabled is allowed (in any
                // function) as that is what shutdown does (and we don't have an
                // explicit list of shutdown imports).
                var changedToDisabled =
                      originalAsyncifyState === Asyncify.State.Normal &&
                      Asyncify.state        === Asyncify.State.Disabled;
                // invoke_* functions are allowed to change the state if we do
                // not ignore indirect calls.
                var ignoredInvoke = x.startsWith('invoke_') &&
                                    {{{ !ASYNCIFY_IGNORE_INDIRECT }}};
                if (Asyncify.state !== originalAsyncifyState &&
                    !isAsyncifyImport &&
                    !changedToDisabled &&
                    !ignoredInvoke) {
                  throw new Error('import ' + x + ' was not in ASYNCIFY_IMPORTS, but changed the state');
                }
              }
            }
#if MAIN_MODULE
            // The dynamic library loader needs to be able to read .sig
            // properties, so that it knows function signatures when it adds
            // them to the table.
            imports[x].sig = original.sig;
#endif
          }
        })(x);
      }
    },
#endif

    instrumentWasmExports: function(exports) {
      var ret = {};
      for (var x in exports) {
        (function(x) {
          var original = exports[x];
          if (typeof original === 'function') {
            ret[x] = function() {
#if ASYNCIFY_DEBUG >= 2
              err('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length) + ' try ' + x);
#endif
              Asyncify.exportCallStack.push(x);
              try {
                return original.apply(null, arguments);
              } finally {
                if (!ABORT) {
                  var y = Asyncify.exportCallStack.pop();
                  assert(y === x);
#if ASYNCIFY_DEBUG >= 2
                  err('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length) + ' finally ' + x);
#endif
                  Asyncify.maybeStopUnwind();
                }
              }
            };
          } else {
            ret[x] = original;
          }
        })(x);
      }
      return ret;
    },

    maybeStopUnwind: function() {
#if ASYNCIFY_DEBUG
      err('ASYNCIFY: maybe stop unwind', Asyncify.exportCallStack);
#endif
      if (Asyncify.currData &&
          Asyncify.state === Asyncify.State.Unwinding &&
          Asyncify.exportCallStack.length === 0) {
        // We just finished unwinding.
#if ASYNCIFY_DEBUG
        err('ASYNCIFY: stop unwind');
#endif
        {{{ runtimeKeepalivePush(); }}}
        Asyncify.state = Asyncify.State.Normal;
        // Keep the runtime alive so that a re-wind can be done later.
        runAndAbortIfError(Module['_asyncify_stop_unwind']);
        if (typeof Fibers !== 'undefined') {
          Fibers.trampoline();
        }
      }
    },

    whenDone: function() {
#if ASSERTIONS
      assert(Asyncify.currData, 'Tried to wait for an async operation when none is in progress.');
      assert(!Asyncify.asyncPromiseHandlers, 'Cannot have multiple async operations in flight at once');
#endif
      return new Promise((resolve, reject) => {
        Asyncify.asyncPromiseHandlers = {
          resolve: resolve,
          reject: reject
        };
      });
    },

    allocateData: function() {
      // An asyncify data structure has three fields:
      //  0  current stack pos
      //  4  max stack pos
      //  8  id of function at bottom of the call stack (callStackIdToName[id] == name of js function)
      //
      // The Asyncify ABI only interprets the first two fields, the rest is for the runtime.
      // We also embed a stack in the same memory region here, right next to the structure.
      // This struct is also defined as asyncify_data_t in emscripten/fiber.h
      var ptr = _malloc({{{ C_STRUCTS.asyncify_data_s.__size__ }}} + Asyncify.StackSize);
      Asyncify.setDataHeader(ptr, ptr + {{{ C_STRUCTS.asyncify_data_s.__size__ }}}, Asyncify.StackSize);
      Asyncify.setDataRewindFunc(ptr);
      return ptr;
    },

    setDataHeader: function(ptr, stack, stackSize) {
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.stack_ptr, 'stack', 'i32') }}};
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.stack_limit, 'stack + stackSize', 'i32') }}};
    },

    setDataRewindFunc: function(ptr) {
      var bottomOfCallStack = Asyncify.exportCallStack[0];
#if ASYNCIFY_DEBUG >= 2
      err('ASYNCIFY: setDataRewindFunc('+ptr+'), bottomOfCallStack is', bottomOfCallStack, new Error().stack);
#endif
      var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'rewindId', 'i32') }}};
    },

    getDataRewindFunc: function(ptr) {
      var id = {{{ makeGetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'i32') }}};
      var name = Asyncify.callStackIdToName[id];
      var func = Module['asm'][name];
      return func;
    },

    doRewind: function(ptr) {
      var start = Asyncify.getDataRewindFunc(ptr);
#if ASYNCIFY_DEBUG
      err('ASYNCIFY: start:', start);
#endif
      // Once we have rewound and the stack we no longer need to artificially keep
      // the runtime alive.
      {{{ runtimeKeepalivePop(); }}}
      return start();
    },

    handleSleep: function(startAsync) {
#if ASSERTIONS
      assert(Asyncify.state !== Asyncify.State.Disabled, 'Asyncify cannot be done during or after the runtime exits');
#endif
      if (ABORT) return;
#if ASYNCIFY_DEBUG
      err('ASYNCIFY: handleSleep ' + Asyncify.state);
#endif
      if (Asyncify.state === Asyncify.State.Normal) {
        // Prepare to sleep. Call startAsync, and see what happens:
        // if the code decided to call our callback synchronously,
        // then no async operation was in fact begun, and we don't
        // need to do anything.
        var reachedCallback = false;
        var reachedAfterCallback = false;
        startAsync((handleSleepReturnValue) => {
#if ASSERTIONS
          assert(!handleSleepReturnValue || typeof handleSleepReturnValue === 'number' || typeof handleSleepReturnValue === 'boolean'); // old emterpretify API supported other stuff
#endif
          if (ABORT) return;
          Asyncify.handleSleepReturnValue = handleSleepReturnValue || 0;
          reachedCallback = true;
          if (!reachedAfterCallback) {
            // We are happening synchronously, so no need for async.
            return;
          }
#if ASSERTIONS
          // This async operation did not happen synchronously, so we did
          // unwind. In that case there can be no compiled code on the stack,
          // as it might break later operations (we can rewind ok now, but if
          // we unwind again, we would unwind through the extra compiled code
          // too).
          assert(!Asyncify.exportCallStack.length, 'Waking up (starting to rewind) must be done from JS, without compiled code on the stack.');
#endif
#if ASYNCIFY_DEBUG
          err('ASYNCIFY: start rewind ' + Asyncify.currData);
#endif
          Asyncify.state = Asyncify.State.Rewinding;
          runAndAbortIfError(() => Module['_asyncify_start_rewind'](Asyncify.currData));
          if (typeof Browser !== 'undefined' && Browser.mainLoop.func) {
            Browser.mainLoop.resume();
          }
          var asyncWasmReturnValue, isError = false;
          try {
            asyncWasmReturnValue = Asyncify.doRewind(Asyncify.currData);
          } catch (err) {
            asyncWasmReturnValue = err;
            isError = true;
          }
          // Track whether the return value was handled by any promise handlers.
          var handled = false;
          if (!Asyncify.currData) {
            // All asynchronous execution has finished.
            // `asyncWasmReturnValue` now contains the final
            // return value of the exported async WASM function.
            //
            // Note: `asyncWasmReturnValue` is distinct from
            // `Asyncify.handleSleepReturnValue`.
            // `Asyncify.handleSleepReturnValue` contains the return
            // value of the last C function to have executed
            // `Asyncify.handleSleep()`, where as `asyncWasmReturnValue`
            // contains the return value of the exported WASM function
            // that may have called C functions that
            // call `Asyncify.handleSleep()`.
            var asyncPromiseHandlers = Asyncify.asyncPromiseHandlers;
            if (asyncPromiseHandlers) {
              Asyncify.asyncPromiseHandlers = null;
              (isError ? asyncPromiseHandlers.reject : asyncPromiseHandlers.resolve)(asyncWasmReturnValue);
              handled = true;
            }
          }
          if (isError && !handled) {
            // If there was an error and it was not handled by now, we have no choice but to
            // rethrow that error into the global scope where it can be caught only by
            // `onerror` or `onunhandledpromiserejection`.
            throw asyncWasmReturnValue;
          }
        });
        reachedAfterCallback = true;
        if (!reachedCallback) {
          // A true async operation was begun; start a sleep.
          Asyncify.state = Asyncify.State.Unwinding;
          // TODO: reuse, don't alloc/free every sleep
          Asyncify.currData = Asyncify.allocateData();
#if ASYNCIFY_DEBUG
          err('ASYNCIFY: start unwind ' + Asyncify.currData);
#endif
          runAndAbortIfError(() => Module['_asyncify_start_unwind'](Asyncify.currData));
          if (typeof Browser !== 'undefined' && Browser.mainLoop.func) {
            Browser.mainLoop.pause();
          }
        }
      } else if (Asyncify.state === Asyncify.State.Rewinding) {
        // Stop a resume.
#if ASYNCIFY_DEBUG
        err('ASYNCIFY: stop rewind');
#endif
        Asyncify.state = Asyncify.State.Normal;
        runAndAbortIfError(Module['_asyncify_stop_rewind']);
        _free(Asyncify.currData);
        Asyncify.currData = null;
        // Call all sleep callbacks now that the sleep-resume is all done.
        Asyncify.sleepCallbacks.forEach((func) => callUserCallback(func));
      } else {
        abort('invalid state: ' + Asyncify.state);
      }
      return Asyncify.handleSleepReturnValue;
    },

    // Unlike `handleSleep`, accepts a function returning a `Promise`
    // and uses the fulfilled value instead of passing in a separate callback.
    //
    // This is particularly useful for native JS `async` functions where the
    // returned value will "just work" and be passed back to C++.
    handleAsync: function(startAsync) {
      return Asyncify.handleSleep((wakeUp) => {
        // TODO: add error handling as a second param when handleSleep implements it.
        startAsync().then(wakeUp);
      });
    },
  },

  emscripten_sleep__deps: ['$safeSetTimeout'],
  emscripten_sleep: function(ms) {
    Asyncify.handleSleep((wakeUp) => safeSetTimeout(wakeUp, ms));
  },

  emscripten_wget__deps: ['$Browser', '$PATH_FS', '$FS'],
  emscripten_wget: function(url, file) {
    Asyncify.handleSleep((wakeUp) => {
      var _url = UTF8ToString(url);
      var _file = UTF8ToString(file);
      _file = PATH_FS.resolve(FS.cwd(), _file);
      var destinationDirectory = PATH.dirname(_file);
      FS.createPreloadedFile(
        destinationDirectory,
        PATH.basename(_file),
        _url, true, true,
        wakeUp,
        wakeUp,
        false, // dontCreateFile
        false, // canOwn
        // preFinish: if the destination directory does not yet exist, create it
        () => FS.mkdirTree(destinationDirectory)
      );
    });
  },

  emscripten_wget_data__deps: ['$asyncLoad', 'malloc'],
  emscripten_wget_data: function(url, pbuffer, pnum, perror) {
    Asyncify.handleSleep((wakeUp) => {
      asyncLoad(UTF8ToString(url), (byteArray) => {
        // can only allocate the buffer after the wakeUp, not during an asyncing
        var buffer = _malloc(byteArray.length); // must be freed by caller!
        HEAPU8.set(byteArray, buffer);
        {{{ makeSetValue('pbuffer', 0, 'buffer', 'i32') }}};
        {{{ makeSetValue('pnum',  0, 'byteArray.length', 'i32') }}};
        {{{ makeSetValue('perror',  0, '0', 'i32') }}};
        wakeUp();
      }, () => {
        {{{ makeSetValue('perror',  0, '1', 'i32') }}};
        wakeUp();
      }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
    });
  },

  emscripten_scan_registers__deps: ['$safeSetTimeout'],
  emscripten_scan_registers: function(func) {
    Asyncify.handleSleep((wakeUp) => {
      // We must first unwind, so things are spilled to the stack. Then while
      // we are pausing we do the actual scan. After that we can resume. Note
      // how using a timeout here avoids unbounded call stack growth, which
      // could happen if we tried to scan the stack immediately after unwinding.
      safeSetTimeout(() => {
        var stackBegin = Asyncify.currData + {{{ C_STRUCTS.asyncify_data_s.__size__ }}};
        var stackEnd = HEAP32[Asyncify.currData >> 2];
        {{{ makeDynCall('vii', 'func') }}}(stackBegin, stackEnd);
        wakeUp();
      }, 0);
    });
  },

  emscripten_lazy_load_code: function() {
    Asyncify.handleSleep((wakeUp) => {
      // Update the expected wasm binary file to be the lazy one.
      wasmBinaryFile += '.lazy.wasm';
      // Add a callback for when all run dependencies are fulfilled, which happens when async wasm loading is done.
      dependenciesFulfilled = wakeUp;
      // Load the new wasm.
      asm = createWasm();
    });
  },

  $Fibers__deps: ['$Asyncify'],
  $Fibers: {
    nextFiber: 0,
    trampolineRunning: false,
    trampoline: function() {
      if (!Fibers.trampolineRunning && Fibers.nextFiber) {
        Fibers.trampolineRunning = true;
        do {
          var fiber = Fibers.nextFiber;
          Fibers.nextFiber = 0;
#if ASYNCIFY_DEBUG >= 2
          err("ASYNCIFY/FIBER: trampoline jump into fiber", fiber, new Error().stack);
#endif
          Fibers.finishContextSwitch(fiber);
        } while (Fibers.nextFiber);
        Fibers.trampolineRunning = false;
      }
    },
    /*
     * NOTE: This function is the asynchronous part of emscripten_fiber_swap.
     */
    finishContextSwitch: function(newFiber) {
      var stack_base = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_base,  'i32') }}};
      var stack_max =  {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_limit, 'i32') }}};
      _emscripten_stack_set_limits(stack_base, stack_max);

#if STACK_OVERFLOW_CHECK >= 2
      Module['___set_stack_limits'](stack_base, stack_max);
#endif

      stackRestore({{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr,   'i32') }}});

      var entryPoint = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, 'i32') }}};

      if (entryPoint !== 0) {
#if STACK_OVERFLOW_CHECK
        writeStackCookie();
#endif
#if ASYNCIFY_DEBUG
        err('ASYNCIFY/FIBER: entering fiber', newFiber, 'for the first time');
#endif
        Asyncify.currData = null;
        {{{ makeSetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, 0, 'i32') }}};

        var userData = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.user_data, 'i32') }}};
        {{{ makeDynCall('vi', 'entryPoint') }}}(userData);
      } else {
        var asyncifyData = newFiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
        Asyncify.currData = asyncifyData;

#if ASYNCIFY_DEBUG
        err('ASYNCIFY/FIBER: start rewind', asyncifyData, '(resuming fiber', newFiber, ')');
#endif
        Asyncify.state = Asyncify.State.Rewinding;
        Module['_asyncify_start_rewind'](asyncifyData);
        Asyncify.doRewind(asyncifyData);
      }
    },
  },

  emscripten_fiber_init__sig: 'viiiiiii',
  emscripten_fiber_init__deps: ['$Asyncify'],
  emscripten_fiber_init: function(fiber, entryPoint, userData, cStack, cStackSize, asyncStack, asyncStackSize) {
    var cStackBase = cStack + cStackSize;

    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_base,  'cStackBase',  'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_limit, 'cStack',      'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_ptr,   'cStackBase',  'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.entry,       'entryPoint', 'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.user_data,   'userData',   'i32') }}};

    var asyncifyData = fiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
    Asyncify.setDataHeader(asyncifyData, asyncStack, asyncStackSize);
  },

  emscripten_fiber_init_from_current_context__sig: 'vii',
  emscripten_fiber_init_from_current_context__deps: ['$Asyncify'],
  emscripten_fiber_init_from_current_context: function(fiber, asyncStack, asyncStackSize) {
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_base,  '_emscripten_stack_get_base()', 'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_limit, '_emscripten_stack_get_end()',  'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.entry,       0,            'i32') }}};

    var asyncifyData = fiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
    Asyncify.setDataHeader(asyncifyData, asyncStack, asyncStackSize);
  },

  emscripten_fiber_swap__sig: 'vii',
  emscripten_fiber_swap__deps: ["$Asyncify", "$Fibers"],
  emscripten_fiber_swap: function(oldFiber, newFiber) {
    if (ABORT) return;
#if ASYNCIFY_DEBUG
    err('ASYNCIFY/FIBER: swap', oldFiber, '->', newFiber, 'state:', Asyncify.state);
#endif
    if (Asyncify.state === Asyncify.State.Normal) {
      Asyncify.state = Asyncify.State.Unwinding;

      var asyncifyData = oldFiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
      Asyncify.setDataRewindFunc(asyncifyData);
      Asyncify.currData = asyncifyData;

#if ASYNCIFY_DEBUG
      err('ASYNCIFY/FIBER: start unwind', asyncifyData);
#endif
      Module['_asyncify_start_unwind'](asyncifyData);

      var stackTop = stackSave();
      {{{ makeSetValue('oldFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr, 'stackTop', 'i32') }}};

      Fibers.nextFiber = newFiber;
    } else {
#if ASSERTIONS
      assert(Asyncify.state === Asyncify.State.Rewinding);
#endif
#if ASYNCIFY_DEBUG
      err('ASYNCIFY/FIBER: stop rewind');
#endif
      Asyncify.state = Asyncify.State.Normal;
      Module['_asyncify_stop_rewind']();
      Asyncify.currData = null;
    }
  },
#else // ASYNCIFY
  emscripten_sleep: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_sleep';
  },
  emscripten_wget: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget';
  },
  emscripten_wget_data: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget_data';
  },
  emscripten_scan_registers: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_scan_registers';
  },
  emscripten_fiber_init: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_fiber_init';
  },
  emscripten_fiber_init_from_current_context: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_fiber_init_from_current_context';
  },
  emscripten_fiber_swap: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_fiber_swap';
  },
#endif // ASYNCIFY
});

if (ASYNCIFY) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$Asyncify');
}
