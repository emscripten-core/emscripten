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
  $Asyncify__deps: ['$runAndAbortIfError', '$callUserCallback', '$sigToWasmTypes',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop'
#endif
  ],

  $Asyncify: {
    //
    // Asyncify code that is shared between mode 1 (original) and mode 2 (JSPI).
    //
    instrumentWasmImports: function(imports) {
#if ASYNCIFY_DEBUG
      dbg('asyncify instrumenting imports');
#endif
      var ASYNCIFY_IMPORTS = {{{ JSON.stringify(ASYNCIFY_IMPORTS.map((x) => x.split('.')[1])) }}};
      for (var x in imports) {
        (function(x) {
          var original = imports[x];
          var sig = original.sig;
          if (typeof original == 'function') {
            var isAsyncifyImport = ASYNCIFY_IMPORTS.indexOf(x) >= 0 ||
                                   x.startsWith('__asyncjs__');
#if ASYNCIFY == 2
            // Wrap async imports with a suspending WebAssembly function.
            if (isAsyncifyImport) {
#if ASSERTIONS
              assert(sig, 'Missing __sig for ' + x);
#endif
              var type = sigToWasmTypes(sig);
#if ASYNCIFY_DEBUG
              dbg('asyncify: suspendOnReturnedPromise for', x, original);
#endif
              // Add space for the suspender promise that will be used in the
              // Wasm wrapper function.
              type.parameters.unshift('externref');
              imports[x] = original = new WebAssembly.Function(
                type,
                original,
                { suspending: 'first' }
              );
            }
#endif
#if ASSERTIONS && ASYNCIFY != 2 // We cannot apply assertions with stack switching, as the imports must not be modified from suspender.suspendOnReturnedPromise TODO find a way
            imports[x] = function() {
              var originalAsyncifyState = Asyncify.state;
              try {
                return original.apply(null, arguments);
              } finally {
                // Only asyncify-declared imports are allowed to change the
                // state.
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
            };
#if MAIN_MODULE
            // The dynamic library loader needs to be able to read .sig
            // properties, so that it knows function signatures when it adds
            // them to the table.
            imports[x].sig = sig;
#endif // MAIN_MODULE
#endif // ASSERTIONS
          }
        })(x);
      }
    },

    instrumentWasmExports: function(exports) {
#if ASYNCIFY_DEBUG
      dbg('asyncify instrumenting exports');
#endif
#if ASYNCIFY == 2
      var ASYNCIFY_EXPORTS = {{{ JSON.stringify(ASYNCIFY_EXPORTS) }}};
#endif
      var ret = {};
      for (var x in exports) {
        (function(x) {
          var original = exports[x];
          if (typeof original == 'function') {
#if ASYNCIFY == 2
            // Wrap all exports with a promising WebAssembly function.
            var isAsyncifyExport = ASYNCIFY_EXPORTS.indexOf(x) >= 0;
            if (isAsyncifyExport) {
#if ASYNCIFY_DEBUG
              dbg('asyncify: returnPromiseOnSuspend for', x, original);
#endif
              var type = WebAssembly.Function.type(original);
              var parameters = type.parameters;
              var results = type.results;
#if ASSERTIONS
              assert(results.length !== 0, 'There must be a return result')
              assert(parameters[0] === 'externref', 'First param must be externref.');
#endif
              // Remove the extern ref.
              parameters.shift();
              original = new WebAssembly.Function(
                { parameters , results: ['externref'] },
                original,
                { promising : 'first' });
            }
#endif
            ret[x] = function() {
#if ASYNCIFY_DEBUG >= 2
              dbg('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length) + ' try ' + x);
#endif
#if ASYNCIFY == 1
              Asyncify.exportCallStack.push(x);
              try {
#endif
                return original.apply(null, arguments);
#if ASYNCIFY == 1
              } finally {
                if (!ABORT) {
                  var y = Asyncify.exportCallStack.pop();
                  assert(y === x);
#if ASYNCIFY_DEBUG >= 2
                  dbg('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length) + ' finally ' + x);
#endif
                  Asyncify.maybeStopUnwind();
                }
              }
#endif
            };
#if MAIN_MODULE
            ret[x].orig = original;
#endif
          } else {
            ret[x] = original;
          }
        })(x);
      }
      return ret;
    },

#if ASYNCIFY == 1
    //
    // Original implementation of Asyncify.
    //
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

    maybeStopUnwind: function() {
#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY: maybe stop unwind', Asyncify.exportCallStack);
#endif
      if (Asyncify.currData &&
          Asyncify.state === Asyncify.State.Unwinding &&
          Asyncify.exportCallStack.length === 0) {
        // We just finished unwinding.
        // Be sure to set the state before calling any other functions to avoid
        // possible infinite recursion here (For example in debug pthread builds
        // the dbg() function itself can call back into WebAssembly to get the
        // current pthread_self() pointer).
        Asyncify.state = Asyncify.State.Normal;
#if ASYNCIFY_DEBUG
        dbg('ASYNCIFY: stop unwind');
#endif
        {{{ runtimeKeepalivePush(); }}}
        // Keep the runtime alive so that a re-wind can be done later.
        runAndAbortIfError(_asyncify_stop_unwind);
        if (typeof Fibers != 'undefined') {
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
      dbg('ASYNCIFY: setDataRewindFunc('+ptr+'), bottomOfCallStack is', bottomOfCallStack, new Error().stack);
#endif
      var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'rewindId', 'i32') }}};
    },

#if RELOCATABLE
    getDataRewindFunc__deps: [ '$resolveGlobalSymbol' ],
#endif
    getDataRewindFunc: function(ptr) {
      var id = {{{ makeGetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'i32') }}};
      var name = Asyncify.callStackIdToName[id];
      var func = Module['asm'][name];
#if RELOCATABLE
      // Exported functions in side modules are not listed in `Module["asm"]`,
      // So we should use `resolveGlobalSymbol` helper function, which is defined in `library_dylink.js`.
      if (!func) {
        func = resolveGlobalSymbol(name, false).sym;
      }
#endif
      return func;
    },

    doRewind: function(ptr) {
      var start = Asyncify.getDataRewindFunc(ptr);
#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY: start:', start);
#endif
      // Once we have rewound and the stack we no longer need to artificially
      // keep the runtime alive.
      {{{ runtimeKeepalivePop(); }}}
      return start();
    },

    // This receives a function to call to start the async operation, and
    // handles everything else for the user of this API. See emscripten_sleep()
    // and other async methods for simple examples of usage.
    handleSleep: function(startAsync) {
#if ASSERTIONS
      assert(Asyncify.state !== Asyncify.State.Disabled, 'Asyncify cannot be done during or after the runtime exits');
#endif
      if (ABORT) return;
#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY: handleSleep ' + Asyncify.state);
#endif
      if (Asyncify.state === Asyncify.State.Normal) {
        // Prepare to sleep. Call startAsync, and see what happens:
        // if the code decided to call our callback synchronously,
        // then no async operation was in fact begun, and we don't
        // need to do anything.
        var reachedCallback = false;
        var reachedAfterCallback = false;
        startAsync((handleSleepReturnValue = 0) => {
#if ASSERTIONS
          assert(!handleSleepReturnValue || typeof handleSleepReturnValue == 'number' || typeof handleSleepReturnValue == 'boolean'); // old emterpretify API supported other stuff
#endif
          if (ABORT) return;
          Asyncify.handleSleepReturnValue = handleSleepReturnValue;
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
          dbg('ASYNCIFY: start rewind ' + Asyncify.currData);
#endif
          Asyncify.state = Asyncify.State.Rewinding;
          runAndAbortIfError(() => _asyncify_start_rewind(Asyncify.currData));
          if (typeof Browser != 'undefined' && Browser.mainLoop.func) {
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
          dbg('ASYNCIFY: start unwind ' + Asyncify.currData);
#endif
          if (typeof Browser != 'undefined' && Browser.mainLoop.func) {
            Browser.mainLoop.pause();
          }
          runAndAbortIfError(() => _asyncify_start_unwind(Asyncify.currData));
        }
      } else if (Asyncify.state === Asyncify.State.Rewinding) {
        // Stop a resume.
#if ASYNCIFY_DEBUG
        dbg('ASYNCIFY: stop rewind');
#endif
        Asyncify.state = Asyncify.State.Normal;
        runAndAbortIfError(_asyncify_stop_rewind);
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

#elif ASYNCIFY == 2
    //
    // JSPI implementation of Asyncify.
    //
    handleSleep: function(startAsync) {
      {{{ runtimeKeepalivePush(); }}}
      var promise = new Promise((resolve) => {
        startAsync(resolve);
      });
      promise.finally(() => {
        {{{ runtimeKeepalivePop(); }}}
      });
      return promise;
    },
    handleAsync: function(startAsync) {
      return Asyncify.handleSleep((wakeUp) => {
        // TODO: add error handling as a second param when handleSleep implements it.
        startAsync().then(wakeUp);
      });
    },
#endif
  },

  emscripten_sleep__sig: 'vi',
  emscripten_sleep__deps: ['$safeSetTimeout'],
  emscripten_sleep: function(ms) {
    // emscripten_sleep() does not return a value, but we still need a |return|
    // here for stack switching support (ASYNCIFY=2). In that mode this function
    // returns a Promise instead of nothing, and that Promise is what tells the
    // wasm VM to pause the stack.
    return Asyncify.handleSleep((wakeUp) => safeSetTimeout(wakeUp, ms));
  },

  emscripten_wget__sig: 'vpp',
  emscripten_wget__deps: ['$Browser', '$PATH_FS', '$FS'],
  emscripten_wget: function(url, file) {
    return Asyncify.handleSleep((wakeUp) => {
      var _url = UTF8ToString(url);
      var _file = UTF8ToString(file);
      _file = PATH_FS.resolve(_file);
      var destinationDirectory = PATH.dirname(_file);
      FS.createPreloadedFile(
        destinationDirectory,
        PATH.basename(_file),
        _url, true, true,
        wakeUp,
        wakeUp,
        false, // dontCreateFile
        false, // canOwn
        function() { // preFinish
          // if a file exists there, we overwrite it
          try {
            FS.unlink(_file);
          } catch (e) {}
          // if the destination directory does not yet exist, create it
          FS.mkdirTree(destinationDirectory);
        }
      );
    });
  },

  emscripten_wget_data__sig: 'vpppp',
  emscripten_wget_data__deps: ['$asyncLoad', 'malloc'],
  emscripten_wget_data: function(url, pbuffer, pnum, perror) {
    return Asyncify.handleSleep((wakeUp) => {
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

  emscripten_scan_registers__sig: 'vp',
  emscripten_scan_registers__deps: ['$safeSetTimeout'],
  emscripten_scan_registers: function(func) {
    return Asyncify.handleSleep((wakeUp) => {
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

  emscripten_lazy_load_code__sig: 'v',
  emscripten_lazy_load_code: function() {
    return Asyncify.handleSleep((wakeUp) => {
      // Update the expected wasm binary file to be the lazy one.
      wasmBinaryFile += '.lazy.wasm';
      // Add a callback for when all run dependencies are fulfilled, which happens when async wasm loading is done.
      dependenciesFulfilled = wakeUp;
      // Load the new wasm.
      asm = createWasm();
    });
  },

  _load_secondary_module__sig: 'v',
  _load_secondary_module: async function() {
    // Mark the module as loading for the wasm module (so it doesn't try to load it again).
    Module['asm']['load_secondary_module_status'].value = 1;
    var imports = {'primary': Module['asm']};
    // Replace '.wasm' suffix with '.deferred.wasm'.
    var deferred = wasmBinaryFile.slice(0, -5) + '.deferred.wasm';
    await new Promise((resolve) => {
      instantiateAsync(null, deferred, imports, resolve);
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
          dbg("ASYNCIFY/FIBER: trampoline jump into fiber", fiber, new Error().stack);
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
      ___set_stack_limits(stack_base, stack_max);
#endif

      stackRestore({{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr,   'i32') }}});

      var entryPoint = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, 'i32') }}};

      if (entryPoint !== 0) {
#if STACK_OVERFLOW_CHECK
        writeStackCookie();
#endif
#if ASYNCIFY_DEBUG
        dbg('ASYNCIFY/FIBER: entering fiber', newFiber, 'for the first time');
#endif
        Asyncify.currData = null;
        {{{ makeSetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, 0, 'i32') }}};

        var userData = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.user_data, 'i32') }}};
        {{{ makeDynCall('vi', 'entryPoint') }}}(userData);
      } else {
        var asyncifyData = newFiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
        Asyncify.currData = asyncifyData;

#if ASYNCIFY_DEBUG
        dbg('ASYNCIFY/FIBER: start rewind', asyncifyData, '(resuming fiber', newFiber, ')');
#endif
        Asyncify.state = Asyncify.State.Rewinding;
        _asyncify_start_rewind(asyncifyData);
        Asyncify.doRewind(asyncifyData);
      }
    },
  },

  emscripten_fiber_swap__sig: 'vii',
  emscripten_fiber_swap__deps: ["$Asyncify", "$Fibers"],
  emscripten_fiber_swap: function(oldFiber, newFiber) {
    if (ABORT) return;
#if ASYNCIFY_DEBUG
    dbg('ASYNCIFY/FIBER: swap', oldFiber, '->', newFiber, 'state:', Asyncify.state);
#endif
    if (Asyncify.state === Asyncify.State.Normal) {
      Asyncify.state = Asyncify.State.Unwinding;

      var asyncifyData = oldFiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
      Asyncify.setDataRewindFunc(asyncifyData);
      Asyncify.currData = asyncifyData;

#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY/FIBER: start unwind', asyncifyData);
#endif
      _asyncify_start_unwind(asyncifyData);

      var stackTop = stackSave();
      {{{ makeSetValue('oldFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr, 'stackTop', 'i32') }}};

      Fibers.nextFiber = newFiber;
    } else {
#if ASSERTIONS
      assert(Asyncify.state === Asyncify.State.Rewinding);
#endif
#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY/FIBER: stop rewind');
#endif
      Asyncify.state = Asyncify.State.Normal;
      _asyncify_stop_rewind();
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
