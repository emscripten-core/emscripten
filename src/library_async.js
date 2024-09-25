/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//
// Async support via ASYNCIFY
//

addToLibrary({
  // error handling

  $runAndAbortIfError: (func) => {
    try {
      return func();
    } catch (e) {
      abort(e);
    }
  },

#if ASYNCIFY
  $Asyncify__deps: ['$runAndAbortIfError', '$callUserCallback', '$sigToWasmTypes',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
#if ASYNCIFY == 1
    // Needed by allocateData and handleSleep respectively
    'malloc', 'free',
#endif
  ],

  $Asyncify: {
    //
    // Asyncify code that is shared between mode 1 (original) and mode 2 (JSPI).
    //
#if ASYNCIFY == 1 && MEMORY64
    rewindArguments: {},
#endif
    instrumentWasmImports(imports) {
#if ASYNCIFY_DEBUG
      dbg('asyncify instrumenting imports');
#endif
#if ASSERTIONS && ASYNCIFY == 2
      assert('Suspending' in WebAssembly, 'JSPI not supported by current environment. Perhaps it needs to be enabled via flags?');
#endif
      var importPattern = {{{ new RegExp(`^(${ASYNCIFY_IMPORTS_EXCEPT_JS_LIBS.map(x => x.split('.')[1]).join('|').replace(/\*/g, '.*')})$`) }}};

      for (let [x, original] of Object.entries(imports)) {
        if (typeof original == 'function') {
          let isAsyncifyImport = original.isAsync || importPattern.test(x);
#if ASYNCIFY == 2
          // Wrap async imports with a suspending WebAssembly function.
          if (isAsyncifyImport) {
#if ASYNCIFY_DEBUG
            dbg('asyncify: suspendOnReturnedPromise for', x, original);
#endif
            imports[x] = original = new WebAssembly.Suspending(original);
          }
#endif
#if ASSERTIONS && ASYNCIFY != 2 // We cannot apply assertions with stack switching, as the imports must not be modified from suspender.suspendOnReturnedPromise TODO find a way
          imports[x] = (...args) => {
            var originalAsyncifyState = Asyncify.state;
            try {
              return original(...args);
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
                throw new Error(`import ${x} was not in ASYNCIFY_IMPORTS, but changed the state`);
              }
            }
          };
#if MAIN_MODULE
          // The dynamic library loader needs to be able to read .sig
          // properties, so that it knows function signatures when it adds
          // them to the table.
          imports[x].sig = original.sig;
#endif // MAIN_MODULE
#endif // ASSERTIONS
        }
      }
    },
#if ASYNCIFY == 1 && MEMORY64
    saveRewindArguments(funcName, passedArguments) {
      return Asyncify.rewindArguments[funcName] = Array.from(passedArguments)
    },
    restoreRewindArguments(funcName) {
      return Asyncify.rewindArguments[funcName] || []
    },
#endif
    instrumentWasmExports(exports) {
#if ASYNCIFY_DEBUG
      dbg('asyncify instrumenting exports');
#endif
#if ASYNCIFY == 2
      var exportPattern = {{{ new RegExp(`^(${ASYNCIFY_EXPORTS.join('|').replace(/\*/g, '.*')})$`) }}};
      Asyncify.asyncExports = new Set();
#endif
      var ret = {};
      for (let [x, original] of Object.entries(exports)) {
        if (typeof original == 'function') {
#if ASYNCIFY == 2
          // Wrap all exports with a promising WebAssembly function.
          let isAsyncifyExport = exportPattern.test(x);
          if (isAsyncifyExport) {
            Asyncify.asyncExports.add(original);
            original = Asyncify.makeAsyncFunction(original);
          }
#endif
          ret[x] = (...args) => {
#if ASYNCIFY_DEBUG >= 2
            dbg(`ASYNCIFY: ${'  '.repeat(Asyncify.exportCallStack.length} try ${x}`);
#endif
#if ASYNCIFY == 1
            Asyncify.exportCallStack.push(x);
            try {
#endif
#if ASYNCIFY == 1 && MEMORY64
              Asyncify.saveRewindArguments(x, args);
#endif
              return original(...args);
#if ASYNCIFY == 1
            } finally {
              if (!ABORT) {
                var y = Asyncify.exportCallStack.pop();
#if ASSERTIONS
                assert(y === x);
#endif
#if ASYNCIFY_DEBUG >= 2
                dbg(`ASYNCIFY: ${'  '.repeat(Asyncify.exportCallStack.length)} finally ${x}`);
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
    // Asyncify.handleSleep((wakeUp) => {...}) is stored here,
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

    getCallStackId(funcName) {
      var id = Asyncify.callStackNameToId[funcName];
      if (id === undefined) {
        id = Asyncify.callStackId++;
        Asyncify.callStackNameToId[funcName] = id;
        Asyncify.callStackIdToName[id] = funcName;
      }
      return id;
    },

    maybeStopUnwind() {
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

    whenDone() {
#if ASSERTIONS
      assert(Asyncify.currData, 'Tried to wait for an async operation when none is in progress.');
      assert(!Asyncify.asyncPromiseHandlers, 'Cannot have multiple async operations in flight at once');
#endif
      return new Promise((resolve, reject) => {
        Asyncify.asyncPromiseHandlers = { resolve, reject };
      });
    },

    allocateData() {
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

    setDataHeader(ptr, stack, stackSize) {
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.stack_ptr, 'stack', '*') }}};
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.stack_limit, 'stack + stackSize', '*') }}};
    },

    setDataRewindFunc(ptr) {
      var bottomOfCallStack = Asyncify.exportCallStack[0];
#if ASYNCIFY_DEBUG >= 2
      dbg('ASYNCIFY: setDataRewindFunc('+ptr+'), bottomOfCallStack is', bottomOfCallStack, new Error().stack);
#endif
      var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
      {{{ makeSetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'rewindId', 'i32') }}};
    },

    getDataRewindFuncName(ptr) {
      var id = {{{ makeGetValue('ptr', C_STRUCTS.asyncify_data_s.rewind_id, 'i32') }}};
      var name = Asyncify.callStackIdToName[id];
      return name;
    },

#if RELOCATABLE
    getDataRewindFunc__deps: [ '$resolveGlobalSymbol' ],
#endif
    getDataRewindFunc(name) {
      var func = wasmExports[name];
#if RELOCATABLE
      // Exported functions in side modules are not listed in `wasmExports`,
      // So we should use `resolveGlobalSymbol` helper function, which is defined in `library_dylink.js`.
      if (!func) {
        func = resolveGlobalSymbol(name, false).sym;
      }
#endif
      return func;
    },

    doRewind(ptr) {
      var name = Asyncify.getDataRewindFuncName(ptr);
      var func = Asyncify.getDataRewindFunc(name);
#if ASYNCIFY_DEBUG
      dbg('ASYNCIFY: doRewind:', name);
#endif
      // Once we have rewound and the stack we no longer need to artificially
      // keep the runtime alive.
      {{{ runtimeKeepalivePop(); }}}
#if MEMORY64
      // When re-winding, the arguments to a function are ignored.  For i32 arguments we
      // can just call the function with no args at all since and the engine will produce zeros
      // for all arguments.  However, for i64 arguments we get `undefined cannot be converted to
      // BigInt`.
      return func(...Asyncify.restoreRewindArguments(name));
#else
      return func();
#endif
    },

    // This receives a function to call to start the async operation, and
    // handles everything else for the user of this API. See emscripten_sleep()
    // and other async methods for simple examples of usage.
    handleSleep(startAsync) {
#if ASSERTIONS
      assert(Asyncify.state !== Asyncify.State.Disabled, 'Asyncify cannot be done during or after the runtime exits');
#endif
      if (ABORT) return;
#if ASYNCIFY_DEBUG
      dbg(`ASYNCIFY: handleSleep ${Asyncify.state}`);
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
          dbg(`ASYNCIFY: start rewind ${Asyncify.currData}`);
#endif
          Asyncify.state = Asyncify.State.Rewinding;
          runAndAbortIfError(() => _asyncify_start_rewind(Asyncify.currData));
          if (typeof MainLoop != 'undefined' && MainLoop.func) {
            MainLoop.resume();
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
          dbg(`ASYNCIFY: start unwind ${Asyncify.currData}`);
#endif
          if (typeof MainLoop != 'undefined' && MainLoop.func) {
            MainLoop.pause();
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
        Asyncify.sleepCallbacks.forEach(callUserCallback);
      } else {
        abort(`invalid state: ${Asyncify.state}`);
      }
      return Asyncify.handleSleepReturnValue;
    },

    // Unlike `handleSleep`, accepts a function returning a `Promise`
    // and uses the fulfilled value instead of passing in a separate callback.
    //
    // This is particularly useful for native JS `async` functions where the
    // returned value will "just work" and be passed back to C++.
    handleAsync(startAsync) {
      return Asyncify.handleSleep((wakeUp) => {
        // TODO: add error handling as a second param when handleSleep implements it.
        startAsync().then(wakeUp);
      });
    },

#elif ASYNCIFY == 2
    //
    // JSPI implementation of Asyncify.
    //

    // Stores all the exported raw Wasm functions that are wrapped with async
    // WebAssembly.Functions.
    asyncExports: null,
    isAsyncExport(func) {
      return Asyncify.asyncExports?.has(func);
    },
    handleAsync: async (startAsync) => {
      {{{ runtimeKeepalivePush(); }}}
      try {
        return await startAsync();
      } finally {
        {{{ runtimeKeepalivePop(); }}}
      }
    },
    handleSleep(startAsync) {
      return Asyncify.handleAsync(() => new Promise(startAsync));
    },
    makeAsyncFunction(original) {
#if ASYNCIFY_DEBUG
      dbg('asyncify: makeAsyncFunction for', original);
#endif
      return WebAssembly.promising(original);
    },
#endif
  },

  emscripten_sleep__deps: ['$safeSetTimeout'],
  emscripten_sleep__async: true,
  emscripten_sleep: (ms) => {
    // emscripten_sleep() does not return a value, but we still need a |return|
    // here for stack switching support (ASYNCIFY=2). In that mode this function
    // returns a Promise instead of nothing, and that Promise is what tells the
    // wasm VM to pause the stack.
    return Asyncify.handleSleep((wakeUp) => safeSetTimeout(wakeUp, ms));
  },

  emscripten_wget_data__deps: ['$asyncLoad', 'malloc'],
  emscripten_wget_data__async: true,
  emscripten_wget_data: (url, pbuffer, pnum, perror) => {
    return Asyncify.handleSleep((wakeUp) => {
      asyncLoad(UTF8ToString(url), (byteArray) => {
        // can only allocate the buffer after the wakeUp, not during an asyncing
        var buffer = _malloc(byteArray.length); // must be freed by caller!
        HEAPU8.set(byteArray, buffer);
        {{{ makeSetValue('pbuffer', 0, 'buffer', '*') }}};
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
  emscripten_scan_registers__async: true,
  emscripten_scan_registers: (func) => {
    return Asyncify.handleSleep((wakeUp) => {
      // We must first unwind, so things are spilled to the stack. Then while
      // we are pausing we do the actual scan. After that we can resume. Note
      // how using a timeout here avoids unbounded call stack growth, which
      // could happen if we tried to scan the stack immediately after unwinding.
      safeSetTimeout(() => {
        var stackBegin = Asyncify.currData + {{{ C_STRUCTS.asyncify_data_s.__size__ }}};
        var stackEnd = {{{ makeGetValue('Asyncify.currData', 0, '*') }}};
        {{{ makeDynCall('vpp', 'func') }}}(stackBegin, stackEnd);
        wakeUp();
      }, 0);
    });
  },

  emscripten_lazy_load_code__async: true,
  emscripten_lazy_load_code: () => Asyncify.handleSleep((wakeUp) => {
    // Update the expected wasm binary file to be the lazy one.
    wasmBinaryFile += '.lazy.wasm';
    // Add a callback for when all run dependencies are fulfilled, which happens when async wasm loading is done.
    dependenciesFulfilled = wakeUp;
    // Load the new wasm.
    createWasm();
  }),

  _load_secondary_module__sig: 'v',
  _load_secondary_module: async function() {
    // Mark the module as loading for the wasm module (so it doesn't try to load it again).
    wasmExports['load_secondary_module_status'].value = 1;
    var imports = {'primary': wasmExports};
    // Replace '.wasm' suffix with '.deferred.wasm'.
    var deferred = wasmBinaryFile.slice(0, -5) + '.deferred.wasm';
    await new Promise((resolve) => {
      instantiateAsync(null, deferred, imports, resolve);
    });
  },

  $Fibers__deps: ['$Asyncify', 'emscripten_stack_set_limits', '$stackRestore'],
  $Fibers: {
    nextFiber: 0,
    trampolineRunning: false,
    trampoline() {
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
    finishContextSwitch(newFiber) {
      var stack_base = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_base,  '*') }}};
      var stack_max =  {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_limit, '*') }}};
      _emscripten_stack_set_limits(stack_base, stack_max);

#if STACK_OVERFLOW_CHECK >= 2
      ___set_stack_limits(stack_base, stack_max);
#endif

      stackRestore({{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr,   '*') }}});

      var entryPoint = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, '*') }}};

      if (entryPoint !== 0) {
#if STACK_OVERFLOW_CHECK
        writeStackCookie();
#endif
#if ASYNCIFY_DEBUG
        dbg('ASYNCIFY/FIBER: entering fiber', newFiber, 'for the first time');
#endif
        Asyncify.currData = null;
        {{{ makeSetValue('newFiber', C_STRUCTS.emscripten_fiber_s.entry, 0, '*') }}};

        var userData = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.user_data, '*') }}};
        {{{ makeDynCall('vp', 'entryPoint') }}}(userData);
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

  emscripten_fiber_swap__deps: ["$Asyncify", "$Fibers", '$stackSave'],
  emscripten_fiber_swap__async: true,
  emscripten_fiber_swap: (oldFiber, newFiber) => {
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
      {{{ makeSetValue('oldFiber', C_STRUCTS.emscripten_fiber_s.stack_ptr, 'stackTop', '*') }}};

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
  emscripten_sleep: () => {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_sleep';
  },
  emscripten_wget: (url, file) => {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget';
  },
  emscripten_wget_data: (url, pbuffer, pnum, perror) => {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget_data';
  },
  emscripten_scan_registers: (func) => {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_scan_registers';
  },
  emscripten_fiber_swap: function(oldFiber, newFiber) {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_fiber_swap';
  },
#endif // ASYNCIFY
});

if (ASYNCIFY) {
  extraLibraryFuncs.push('$Asyncify');
}
