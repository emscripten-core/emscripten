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
#if !WASM_BACKEND
/*
 * The layout of normal and async stack frames
 *
 * ---------------------  <-- saved sp for the current function
 * <last normal stack frame>
 * ---------------------
 * pointer to the previous frame <-- __async_cur_frame
 * saved sp
 * callback function   <-- ctx, returned by alloc/reallloc, used by the program
 * saved local variable1
 * saved local variable2
 * ...
 * --------------------- <-- STACKTOP
 *
 */
  __async: 0, // whether a truly async function has been called
  __async_unwind: 1, // whether to unwind the async stack frame
  __async_retval: '{{{ makeStaticAlloc(2) }}}', // store the return value for async functions
  __async_cur_frame: 0, // address to the current frame, which stores previous frame, stack pointer and async context

  // __async_retval is not actually required in emscripten_async_resume
  // but we want it included when ASYNCIFY is enabled
  emscripten_async_resume__deps: ['__async', '__async_unwind', '__async_retval', '__async_cur_frame'],
  emscripten_async_resume__sig: 'v',
  emscripten_async_resume__asm: true,
  emscripten_async_resume: function() {
    var callback = 0;
    ___async = 0;
    ___async_unwind = 1;
    while (1) {
      if (!___async_cur_frame) return;
      callback = {{{ makeGetValueAsm('___async_cur_frame', 8, 'i32') }}};
      // the signature of callback is always vi
      // the only argument is ctx
      {{{ makeDynCall('vi') }}}(callback | 0, (___async_cur_frame + 8)|0);
      if (___async) return; // that was an async call
      if (!___async_unwind) {
        // keep the async stack
        ___async_unwind = 1;
        continue;
      }
      // unwind normal stack frame
      stackRestore({{{ makeGetValueAsm('___async_cur_frame', 4, 'i32') }}});
      // pop the last async stack frame
      ___async_cur_frame = {{{ makeGetValueAsm('___async_cur_frame', 0, 'i32') }}};
    }
  },

  emscripten_sleep__deps: ['emscripten_async_resume', '$Browser'],
  emscripten_sleep: function(ms) {
    Module['setAsync'](); // tell the scheduler that we have a callback on hold
    Browser.safeSetTimeout(_emscripten_async_resume, ms);
  },

  emscripten_alloc_async_context__deps: ['__async_cur_frame'],
  emscripten_alloc_async_context__sig: 'iii',
  emscripten_alloc_async_context__asm: true,
  emscripten_alloc_async_context: function(len, sp) {
    len = len|0;
    sp = sp|0;
    // len is the size of ctx
    // we also need to store prev_frame, stack pointer before ctx
    var new_frame = 0; new_frame = stackAlloc((len + 8)|0)|0;
    // save sp
    {{{ makeSetValueAsm('new_frame', 4, 'sp', 'i32') }}};
    // link the frame with previous one
    {{{ makeSetValueAsm('new_frame', 0, '___async_cur_frame', 'i32') }}};
    ___async_cur_frame = new_frame;
    return (___async_cur_frame + 8)|0;
  },

  emscripten_realloc_async_context__deps: ['__async_cur_frame'],
  emscripten_realloc_async_context__sig: 'ii',
  emscripten_realloc_async_context__asm: true,
  emscripten_realloc_async_context: function(len) {
    len = len|0;
    // assuming that we have on the stacktop
    stackRestore(___async_cur_frame | 0);
    return ((stackAlloc((len + 8)|0)|0) + 8)|0;
  },

  emscripten_free_async_context__deps: ['__async_cur_frame'],
  emscripten_free_async_context__sig: 'vi',
  emscripten_free_async_context__asm: true,
  emscripten_free_async_context: function(ctx) {
    //  this function is called when a possibly async function turned out to be sync
    //  just undo a recent emscripten_alloc_async_context
    ctx = ctx|0;
#if ASSERTIONS
    if ((((___async_cur_frame + 8)|0) != (ctx|0))|0) abort();
#endif
    stackRestore(___async_cur_frame | 0);
    ___async_cur_frame = {{{ makeGetValueAsm('___async_cur_frame', 0, 'i32') }}};
  },

  emscripten_check_async: true,
  emscripten_do_not_unwind: true,
  emscripten_do_not_unwind_async: true,

  emscripten_get_async_return_value_addr__deps: ['__async_retval'],
  emscripten_get_async_return_value_addr: true,

/*
 * Layout of an ASYNCIFY coroutine structure
 *
 *  0 callee's async ctx
 *  4 callee's STACKTOP
 *  8 callee's STACK_MAX
 * 12 my async ctx
 * 16 my STACKTOP
 * 20 my stack size
 * 24 coroutine function
 * 28 coroutine arg
 * 32 my stack:
 *    ...
 */
  emscripten_coroutine_create__sig: 'iiii',
  emscripten_coroutine_create__asm: true,
  emscripten_coroutine_create__deps: ['malloc', 'emscripten_alloc_async_context'],
  emscripten_coroutine_create: function(f, arg, stack_size) {
    f = f|0;
    arg = arg|0;
    stack_size = stack_size|0;
    var coroutine = 0;

    if ((stack_size|0) <= 0) stack_size = 4096;

    coroutine = _malloc(stack_size + 32 | 0) | 0;
    {{{ makeSetValueAsm('coroutine', 12, 0, 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 16, '(coroutine+32)', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 20, 'stack_size', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 24, 'f', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 28, 'arg', 'i32') }}};
    return coroutine|0;
  },
  emscripten_coroutine_next__sig: 'ii',
  emscripten_coroutine_next__asm: true,
  emscripten_coroutine_next__deps: ['__async_cur_frame', '__async', 'emscripten_async_resume', 'free'],
  emscripten_coroutine_next: function(coroutine) {
    coroutine = coroutine|0;
    var coroutine_not_finished = 0, temp = 0;
    // switch context
    {{{ makeSetValueAsm('coroutine', 0, '___async_cur_frame', 'i32') }}};
    temp = stackSave() | 0;
    {{{ makeSetValueAsm('coroutine', 4, 'temp', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 8, 'STACK_MAX', 'i32') }}};
    ___async_cur_frame = {{{ makeGetValueAsm('coroutine', 12, 'i32') }}};
    stackRestore({{{ makeGetValueAsm('coroutine', 16, 'i32') }}});
    STACK_MAX = coroutine + 32 + {{{ makeGetValueAsm('coroutine', 20, 'i32') }}} | 0;

    if (!___async_cur_frame) {
      // first run
      {{{ makeDynCall('vi') }}}(
        {{{ makeGetValueAsm('coroutine', 24, 'i32') }}},
        {{{ makeGetValueAsm('coroutine', 28, 'i32') }}}
      );
    } else {
      _emscripten_async_resume();
    }

    // switch context
    {{{ makeSetValueAsm('coroutine', 12, '___async_cur_frame', 'i32') }}};
    temp = stackSave() | 0;
    {{{ makeSetValueAsm('coroutine', 16, 'temp', 'i32') }}};
    ___async_cur_frame = {{{ makeGetValueAsm('coroutine', 0, 'i32') }}};
    stackRestore({{{ makeGetValueAsm('coroutine', 4, 'i32') }}});
    STACK_MAX = {{{ makeGetValueAsm('coroutine', 8, 'i32') }}};

    coroutine_not_finished = ___async;
    if (!coroutine_not_finished) {
      // coroutine has finished
      _free(coroutine);
    }
    // coroutine may be created during an async function
    // we do not want to affect the original async ctx
    // strictly we should backup and restore ___async, ___async_retval and ___async_unwind
    // but ___async=0 seems enough
    ___async = 0;

    return coroutine_not_finished|0;
  },
  emscripten_yield__sig: 'v',
  emscripten_yield__asm: true,
  emscripten_yield: function() {
    ___async = 1;
  },

  emscripten_wget__deps: ['emscripten_async_resume', '$PATH_FS', '$Browser'],
  emscripten_wget: function(url, file) {
    var _url = UTF8ToString(url);
    var _file = UTF8ToString(file);
    _file = PATH_FS.resolve(FS.cwd(), _file);
    Module['setAsync']();
    noExitRuntime = true;
    var destinationDirectory = PATH.dirname(_file);
    FS.createPreloadedFile(
      destinationDirectory,
      PATH.basename(_file),
      _url, true, true,
      _emscripten_async_resume,
      _emscripten_async_resume,
      undefined, // dontCreateFile
      undefined, // canOwn
      function() { // preFinish
        // if the destination directory does not yet exist, create it
        FS.mkdirTree(destinationDirectory);
      }
    );
  },

  emscripten_fiber_init: function() {
    throw 'emscripten_fiber_init is not implemented for fastcomp ASYNCIFY';
  },
  emscripten_fiber_init_from_current_context: function() {
    throw 'emscripten_fiber_init_from_current_context is not implemented for fastcomp ASYNCIFY';
  },
  emscripten_fiber_swap: function() {
    throw 'emscripten_fiber_swap is not implemented for fastcomp ASYNCIFY';
  },

#else // !WASM_BACKEND
  $Asyncify__deps: ['$Browser', '$runAndAbortIfError'],
  $Asyncify: {
    State: {
      Normal: 0,
      Unwinding: 1,
      Rewinding: 2
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
    callStackIdToFunc: {},
#if ASYNCIFY_LAZY_LOAD_CODE
    callStackIdToName: {},
#endif
    callStackId: 0,
    afterUnwind: null,
    asyncFinalizers: [], // functions to run when *all* asynchronicity is done
    sleepCallbacks: [], // functions to call every time we sleep

    getCallStackId: function(funcName) {
      var id = Asyncify.callStackNameToId[funcName];
      if (id === undefined) {
        id = Asyncify.callStackId++;
        Asyncify.callStackNameToId[funcName] = id;
#if ASYNCIFY_LAZY_LOAD_CODE
        Asyncify.callStackIdToName[id] = funcName;
#else
        Asyncify.callStackIdToFunc[id] = Module['asm'][funcName];
#endif
      }
      return id;
    },

#if ASSERTIONS
    instrumentWasmImports: function(imports) {
      var ASYNCIFY_IMPORTS = {{{ JSON.stringify(ASYNCIFY_IMPORTS) }}}.map(function(x) {
        return x.split('.')[1];
      });
      for (var x in imports) {
        (function(x) {
          var original = imports[x];
          if (typeof original === 'function') {
            imports[x] = function() {
              var originalAsyncifyState = Asyncify.state;
              try {
                return original.apply(null, arguments);
              } finally {
                // Only functions in the list of known relevant imports are allowed to change the state.
                // Note that invoke_* functions are allowed to change the state if we do not ignore
                // indirect calls.
                if (Asyncify.state !== originalAsyncifyState &&
                    ASYNCIFY_IMPORTS.indexOf(x) < 0 &&
                    !(x.startsWith('invoke_') && {{{ !ASYNCIFY_IGNORE_INDIRECT }}})) {
                  throw 'import ' + x + ' was not in ASYNCIFY_IMPORTS, but changed the state';
                }
              }
            }
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
              Asyncify.exportCallStack.push(x);
#if ASYNCIFY_DEBUG >= 2
              err('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length) + ' try', x);
#endif
              try {
                return original.apply(null, arguments);
              } finally {
                if (ABORT) return;
                var y = Asyncify.exportCallStack.pop(x);
                assert(y === x);
#if ASYNCIFY_DEBUG >= 2
                err('ASYNCIFY: ' + '  '.repeat(Asyncify.exportCallStack.length + 1) + ' finally', x);
#endif
                if (Asyncify.currData &&
                    Asyncify.state === Asyncify.State.Unwinding &&
                    Asyncify.exportCallStack.length === 0) {
                  // We just finished unwinding.
#if ASYNCIFY_DEBUG
                  err('ASYNCIFY: stop unwind');
#endif
                  Asyncify.state = Asyncify.State.Normal;
                  runAndAbortIfError(Module['_asyncify_stop_unwind']);
                  if (typeof Fibers !== 'undefined') {
                    Fibers.trampoline();
                  }
                  if (Asyncify.afterUnwind) {
                    Asyncify.afterUnwind();
                    Asyncify.afterUnwind = null;
                  }
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

    allocateData: function() {
      // An asyncify data structure has three fields:
      //  0  current stack pos
      //  4  max stack pos
      //  8  id of function at bottom of the call stack (callStackIdToFunc[id] == js function)
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
      var func = Asyncify.callStackIdToFunc[id];

#if ASYNCIFY_LAZY_LOAD_CODE
      if (func === undefined) {
        func = Module['asm'][Asyncify.callStackIdToName[id]];
        Asyncify.callStackIdToFunc[id] = func;
      }
#endif

      return func;
    },

    handleSleep: function(startAsync) {
      if (ABORT) return;
      noExitRuntime = true;
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
        startAsync(function(handleSleepReturnValue) {
#if ASSERTIONS
          assert(!handleSleepReturnValue || typeof handleSleepReturnValue === 'number'); // old emterpretify API supported other stuff
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
          runAndAbortIfError(function() { Module['_asyncify_start_rewind'](Asyncify.currData) });
          if (Browser.mainLoop.func) {
            Browser.mainLoop.resume();
          }
          var start = Asyncify.getDataRewindFunc(Asyncify.currData);
#if ASYNCIFY_DEBUG
          err('ASYNCIFY: start:', start);
#endif
          var asyncWasmReturnValue = start();
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
            var asyncFinalizers = Asyncify.asyncFinalizers;
            Asyncify.asyncFinalizers = [];
            asyncFinalizers.forEach(function(func) {
              func(asyncWasmReturnValue);
            });
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
          runAndAbortIfError(function() { Module['_asyncify_start_unwind'](Asyncify.currData) });
          if (Browser.mainLoop.func) {
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
        Asyncify.sleepCallbacks.forEach(function(func) {
          func();
        });
      } else {
        abort('invalid state: ' + Asyncify.state);
      }
      return Asyncify.handleSleepReturnValue;
    }
  },

  emscripten_sleep: function(ms) {
    Asyncify.handleSleep(function(wakeUp) {
      Browser.safeSetTimeout(wakeUp, ms);
    });
  },

  emscripten_wget__deps: ['$PATH_FS', '$FS'],
  emscripten_wget: function(url, file) {
    Asyncify.handleSleep(function(wakeUp) {
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
        undefined, // dontCreateFile
        undefined, // canOwn
        function() { // preFinish
          // if the destination directory does not yet exist, create it
          FS.mkdirTree(destinationDirectory);
        }
      );
    });
  },

  emscripten_wget_data: function(url, pbuffer, pnum, perror) {
    Asyncify.handleSleep(function(wakeUp) {
      Browser.asyncLoad(UTF8ToString(url), function(byteArray) {
        // can only allocate the buffer after the wakeUp, not during an asyncing
        var buffer = _malloc(byteArray.length); // must be freed by caller!
        HEAPU8.set(byteArray, buffer);
        {{{ makeSetValueAsm('pbuffer', 0, 'buffer', 'i32') }}};
        {{{ makeSetValueAsm('pnum',  0, 'byteArray.length', 'i32') }}};
        {{{ makeSetValueAsm('perror',  0, '0', 'i32') }}};
        wakeUp();
      }, function() {
        {{{ makeSetValueAsm('perror',  0, '1', 'i32') }}};
        wakeUp();
      }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
    });
  },

  emscripten_scan_registers: function(func) {
    Asyncify.handleSleep(function(wakeUp) {
      // We must first unwind, so things are spilled to the stack. We
      // can resume right after unwinding, no need for a timeout.
      Asyncify.afterUnwind = function() {
        {{{ makeDynCall('vii') }}}(func, Asyncify.currData + 8, HEAP32[Asyncify.currData >> 2]);
        wakeUp();
      };
    });
  },

  emscripten_lazy_load_code: function() {
    Asyncify.handleSleep(function(wakeUp) {
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
      STACK_BASE = {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_base,  'i32') }}};
      STACK_MAX =  {{{ makeGetValue('newFiber', C_STRUCTS.emscripten_fiber_s.stack_limit, 'i32') }}};

#if WASM_BACKEND && STACK_OVERFLOW_CHECK >= 2
      Module['___set_stack_limit'](STACK_MAX);
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
        {{{ makeDynCall('vi') }}}(entryPoint, userData);
      } else {
        var asyncifyData = newFiber + 20;
        Asyncify.currData = asyncifyData;

#if ASYNCIFY_DEBUG
        err('ASYNCIFY/FIBER: start rewind', asyncifyData, '(resuming fiber', newFiber, ')');
#endif
        Asyncify.state = Asyncify.State.Rewinding;
        Module['_asyncify_start_rewind'](asyncifyData);
        var start = Asyncify.getDataRewindFunc(asyncifyData);
#if ASYNCIFY_DEBUG
        err('ASYNCIFY/FIBER: start: ' + start);
#endif
        start();
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
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_base,  'STACK_BASE', 'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.stack_limit, 'STACK_MAX',  'i32') }}};
    {{{ makeSetValue('fiber', C_STRUCTS.emscripten_fiber_s.entry,       0,            'i32') }}};

    var asyncifyData = fiber + {{{ C_STRUCTS.emscripten_fiber_s.asyncify_data }}};
    Asyncify.setDataHeader(asyncifyData, asyncStack, asyncStackSize);
  },

  emscripten_fiber_swap__sig: 'vii',
  emscripten_fiber_swap__deps: ["$Asyncify", "$Fibers"],
  emscripten_fiber_swap: function(oldFiber, newFiber) {
    if (ABORT) return;
    noExitRuntime = true;
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

  emscripten_coroutine_create: function() {
    throw 'emscripten_coroutine_create has been removed. Please use the Fibers API';
  },
  emscripten_coroutine_next: function() {
    throw 'emscripten_coroutine_next has been removed. Please use the Fibers API';
  },
  emscripten_yield: function() {
    throw 'emscripten_yield has been removed. Please use the Fibers API';
  },
#endif
#else // ASYNCIFY
  emscripten_sleep: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_sleep';
  },
  emscripten_coroutine_create: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_coroutine_create';
  },
  emscripten_coroutine_next: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_coroutine_next';
  },
  emscripten_yield: function() {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_yield';
  },
  emscripten_wget: function(url, file) {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget';
  },
  emscripten_wget_data: function(url, file) {
    throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_wget_data';
  },
  emscripten_scan_registers: function(url, file) {
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

if (WASM_BACKEND && ASYNCIFY) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$Asyncify');
}

