// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Async support
//
// Two experiments in async support: ASYNCIFY, and EMTERPRETIFY_ASYNC

mergeInto(LibraryManager.library, {
  // error handling

  $runAndAbortIfError: function(func) {
    try {
      return func();
    } catch (e) {
      abort(e);
    }
  },

#if !WASM_BACKEND && ASYNCIFY
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

#else // !WASM_BACKEND && ASYNCIFY

#if EMTERPRETIFY_ASYNC

  // Emterpreter sync=>async support
  //
  // The idea here is that almost all interpreter frames are already on the stack (the
  // emterpreter stack), so it's easy to save a callstack and reload it, we just need
  // to also save the pc.
  // Saving state keeps the pc right before the current call. That means when we reload,
  // we are going to re-call in the exact same way as before - including the final call
  // to the async function here! Therefore sleep etc. detect the state, so they know
  // if they are the first call or the second. The second typically does nothing, but
  // if there is a return value it could return it, etc.
  $EmterpreterAsync__deps: ['$Browser'],
  $EmterpreterAsync: {
    initted: false,
    state: 0, // 0 - Nothing/normal.
              // 1 - Sleeping: This is set when we start to save the stack, and continues through the
              //     sleep, until we start to restore the stack.
              //     If we enter a function while in this state - that is, before we
              //     start to restore the stack, or in other words if we call a function while
              //     sleeping - then we switch to state 3, "definitely sleeping". That lets us know
              //     we are no longer saving the stack. How this works is that while we save the stack
              //     we don't hit any function entries, so they are valid places to switch to state 3,
              //     and then when we reach code later that checks if we need to save the stack, we
              //     know we don't need to.
              // 2 - Restoring the stack: On the way to resume normal execution.
              // 3 - Definitely sleeping, that is, sleeping and after saving the stack.
    saveStack: '',
    yieldCallbacks: [],
    postAsync: null,
    restartFunc: null, // During an async call started with ccall, this contains the function generated
                       // by the compiler that calls emterpret and returns the return value. If we call
                       // emterpret directly, we don't get the return value back (and we can't just read
                       // it from the stack because we don't know the correct type). Note that only
                       // a single one (and not a stack) is required because only one async ccall can be
                       // in flight at once.
    asyncFinalizers: [], // functions to run when *all* asynchronicity is done

    ensureInit: function() {
      if (this.initted) return;
      this.initted = true;
#if ASSERTIONS
      abortDecorators.push(function(output, what) {
        if (EmterpreterAsync.state === 1 || EmterpreterAsync.state === 2) {
          return output + '\nThis error happened during an emterpreter-async operation. Was there non-emterpreted code on the stack during save (which is unallowed)? If so, you may want to adjust EMTERPRETIFY_BLACKLIST, EMTERPRETIFY_WHITELIST. For reference, this is what the stack looked like when we tried to save it: ' + [EmterpreterAsync.state, EmterpreterAsync.saveStack];
        }
        return output;
      });
#endif
    },
    setState: function(s) {
      this.ensureInit();
      this.state = s;
      Module['setAsyncState'](s);
    },
    handle: function(doAsyncOp, yieldDuring) {
      noExitRuntime = true;
      if (EmterpreterAsync.state === 0) {
        // save the stack we want to resume. this lets other code run in between
        // XXX this assumes that this stack top never ever leak! exceptions might violate that
        var stack = new Int32Array(HEAP32.subarray(EMTSTACKTOP>>2, Module['emtStackSave']()>>2));
#if ASSERTIONS
        var stacktop = Module['stackSave']();
#endif

        var resumedCallbacksForYield = false;
        function resumeCallbacksForYield() {
          if (resumedCallbacksForYield) return;
          resumedCallbacksForYield = true;
          // allow async callbacks, and also make sure to call the specified yield callbacks. we must
          // do this when nothing is on the stack, i.e. after it unwound
          EmterpreterAsync.yieldCallbacks.forEach(function(func) {
            func();
          });
          Browser.resumeAsyncCallbacks(); // if we were paused (e.g. we are after a sleep), then since we are now yielding, it is safe to call callbacks
        }

        var callingDoAsyncOp = 1; // if resume is called synchronously - during the doAsyncOp - we must make it truly async, for consistency

        doAsyncOp(function resume(post) {
          if (ABORT) {
            return;
          }
          if (callingDoAsyncOp) {
            assert(callingDoAsyncOp === 1); // avoid infinite recursion
            callingDoAsyncOp++;
            setTimeout(function() {
              resume(post);
            }, 0);
            return;
          }

          assert(EmterpreterAsync.state === 1 || EmterpreterAsync.state === 3);
          EmterpreterAsync.setState(3);
          if (yieldDuring) {
            resumeCallbacksForYield();
          }
          // copy the stack back in and resume
          HEAP32.set(stack, EMTSTACKTOP>>2);
#if ASSERTIONS
          assert(stacktop === Module['stackSave']()); // nothing should have modified the stack meanwhile
#endif
          // we are now starting to restore the stack
          EmterpreterAsync.setState(2);
          // Resume the main loop
          if (Browser.mainLoop.func) {
            Browser.mainLoop.resume();
          }
          assert(!EmterpreterAsync.postAsync);
          EmterpreterAsync.postAsync = post || null;
          var asyncReturnValue;
          if (!EmterpreterAsync.restartFunc) {
            // pc of the first function, from which we can reconstruct the rest, is at position 0 on the stack
            Module['emterpret'](stack[0]);
          } else {
            // the restartFunc knows how to emterpret the proper function, and also returns the return value
            asyncReturnValue = EmterpreterAsync.restartFunc();
          }
          if (!yieldDuring && EmterpreterAsync.state === 0) {
            // if we did *not* do another async operation, then we know that nothing is conceptually on the stack now, and we can re-allow async callbacks as well as run the queued ones right now
            Browser.resumeAsyncCallbacks();
          }
          if (EmterpreterAsync.state === 0) {
            // All async operations have concluded.
            // In particular, if we were in an async ccall, we have
            // consumed the restartFunc and can reset it to null.
            EmterpreterAsync.restartFunc = null;
            // The async finalizers can run now, after all async operations.
            var asyncFinalizers = EmterpreterAsync.asyncFinalizers;
            EmterpreterAsync.asyncFinalizers = [];
            asyncFinalizers.forEach(function(func) {
              func(asyncReturnValue);
            });
          }
        });

        callingDoAsyncOp = 0;

        EmterpreterAsync.setState(1);
#if ASSERTIONS
        EmterpreterAsync.saveStack = new Error().stack; // we can't call  stackTrace()  as it calls compiled code
#endif
        // Pause the main loop, until we resume
        if (Browser.mainLoop.func) {
          Browser.mainLoop.pause();
        }
        if (yieldDuring) {
          // do this when we are not on the stack, i.e., the stack unwound. we might be too late, in which case we do it in resume()
          setTimeout(function() {
            resumeCallbacksForYield();
          }, 0);
        } else {
          Browser.pauseAsyncCallbacks();
        }
      } else {
        // nothing to do here, the stack was just recreated. reset the state.
        assert(EmterpreterAsync.state === 2);
        EmterpreterAsync.setState(0);

        if (EmterpreterAsync.postAsync) {
          var ret = EmterpreterAsync.postAsync();
          EmterpreterAsync.postAsync = null;
          return ret;
        }
      }
    }
  },

  emscripten_sleep__deps: ['$EmterpreterAsync'],
  emscripten_sleep: function(ms) {
    EmterpreterAsync.handle(function(resume) {
      setTimeout(function() {
        // do this manually; we can't call into Browser.safeSetTimeout, because that is paused/resumed!
        resume();
      }, ms);
    });
  },

  emscripten_sleep_with_yield__deps: ['$EmterpreterAsync'],
  emscripten_sleep_with_yield: function(ms) {
    EmterpreterAsync.handle(function(resume) {
      Browser.safeSetTimeout(resume, ms);
    }, true);
  },

  emscripten_wget__deps: ['$EmterpreterAsync', '$PATH_FS', '$FS', '$Browser'],
  emscripten_wget: function(url, file) {
    EmterpreterAsync.handle(function(resume) {
      var _url = UTF8ToString(url);
      var _file = UTF8ToString(file);
      _file = PATH_FS.resolve(FS.cwd(), _file);
      var destinationDirectory = PATH.dirname(_file);
      FS.createPreloadedFile(
        destinationDirectory,
        PATH.basename(_file),
        _url, true, true,
        resume,
        resume,
        undefined, // dontCreateFile
        undefined, // canOwn
        function() { // preFinish
          // if the destination directory does not yet exist, create it
          FS.mkdirTree(destinationDirectory);
        }
      );
    });
  },

  emscripten_wget_data__deps: ['$EmterpreterAsync', '$Browser'],
  emscripten_wget_data: function(url, pbuffer, pnum, perror) {
    EmterpreterAsync.handle(function(resume) {
      Browser.asyncLoad(UTF8ToString(url), function(byteArray) {
        resume(function() {
          // can only allocate the buffer after the resume, not during an asyncing
          var buffer = _malloc(byteArray.length); // must be freed by caller!
          HEAPU8.set(byteArray, buffer);
          {{{ makeSetValueAsm('pbuffer', 0, 'buffer', 'i32') }}};
          {{{ makeSetValueAsm('pnum',  0, 'byteArray.length', 'i32') }}};
          {{{ makeSetValueAsm('perror',  0, '0', 'i32') }}};
        });
      }, function() {
        {{{ makeSetValueAsm('perror',  0, '1', 'i32') }}};
        resume();
      }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
    });
  },

  /*
   * Layout of an EMTERPRETIFY_ASYNC coroutine structure:
   *
   *  0 callee's EMTSTACKTOP
   *  4 callee's EMTSTACKTOP from the compiled code
   *  8 callee's EMT_STACK_MAX
   * 12 my EMTSTACKTOP
   * 16 my EMTSTACKTOP from the compiled code
   * 20 my EMT_STACK_MAX
   * 24 coroutine function (0 if already started)
   * 28 coroutine arg
   * 32 my stack:
   *    ...
   */
  emscripten_coroutine_create__sig: 'iiii',
  emscripten_coroutine_create__asm: true,
  emscripten_coroutine_create__deps: ['malloc'],
  emscripten_coroutine_create: function(f, arg, stack_size) {
    f = f|0;
    arg = arg|0;
    stack_size = stack_size|0;
    var coroutine = 0;

    if ((stack_size|0) <= 0) stack_size = 4096;

    coroutine = _malloc(stack_size + 32 | 0) | 0;
    {{{ makeSetValueAsm('coroutine', 12, '(coroutine+32)', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 16, '(coroutine+32)', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 20, '(coroutine+32+stack_size)', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 24, 'f', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 28, 'arg', 'i32') }}};
    return coroutine|0;
  },

  emscripten_coroutine_next__sig: 'ii',
  emscripten_coroutine_next__deps: ['$EmterpreterAsync', 'free'],
  emscripten_coroutine_next: function(coroutine) {
    // this is a rewritten emscripten_coroutine_next function from ASYNCIFY
    coroutine = coroutine|0;
    var temp = 0, func = 0, funcArg = 0, coroutine_not_finished = 0;

    // switch context
    // TODO Save EMTSTACKTOP to EMTSTACK_BASE during startup and use it instead
    {{{ makeSetValueAsm('coroutine', 0, 'EMTSTACKTOP', 'i32') }}};
    temp = Module['emtStackSave']();
    {{{ makeSetValueAsm('coroutine', 4, 'temp', 'i32') }}};
    temp = Module['getEmtStackMax']();
    {{{ makeSetValueAsm('coroutine', 8, 'temp', 'i32') }}};

    EMTSTACKTOP = {{{ makeGetValueAsm('coroutine', 12, 'i32') }}};
    Module['emtStackRestore']({{{ makeGetValueAsm('coroutine', 16, 'i32') }}});
    Module['setEmtStackMax']({{{ makeGetValueAsm('coroutine', 20, 'i32') }}});

    func = {{{ makeGetValueAsm('coroutine', 24, 'i32') }}};
    if (func !== 0) {
      // unset func
      {{{ makeSetValueAsm('coroutine', 24, 0, 'i32') }}};
      // first run
      funcArg = {{{ makeGetValueAsm('coroutine', 28, 'i32') }}};
      {{{ makeDynCall('vi') }}}(func, funcArg);
    } else {
      EmterpreterAsync.setState(2);
      Module['emterpret']({{{ makeGetValue('EMTSTACKTOP', 0, 'i32')}}});
    }
    coroutine_not_finished = EmterpreterAsync.state !== 0;
    EmterpreterAsync.setState(0);

    // switch context
    {{{ makeSetValueAsm('coroutine', 12, 'EMTSTACKTOP', 'i32') }}}; // cannot change?
    temp = Module['emtStackSave']();
    {{{ makeSetValueAsm('coroutine', 16, 'temp', 'i32') }}};
    temp = Module['getEmtStackMax']();
    {{{ makeSetValueAsm('coroutine', 20, 'temp', 'i32') }}}; // cannot change?

    EMTSTACKTOP = {{{ makeGetValueAsm('coroutine', 0, 'i32') }}};
    Module['emtStackRestore']({{{ makeGetValueAsm('coroutine', 4, 'i32') }}});
    Module['setEmtStackMax']({{{ makeGetValueAsm('coroutine', 8, 'i32') }}});

    if (!coroutine_not_finished) {
      _free(coroutine);
    }

    return coroutine_not_finished|0;
  },

  emscripten_yield__sig: 'v',
  emscripten_yield__deps: ['$EmterpreterAsync'],
  emscripten_yield: function() {
    if (EmterpreterAsync.state === 2) {
      // re-entering after yield
      EmterpreterAsync.setState(0);
    } else {
      EmterpreterAsync.setState(1);
    }
  },

  emscripten_fiber_init: function() {
    throw 'emscripten_fiber_init is not implemented for EMTERPRETIFY_ASYNC';
  },
  emscripten_fiber_init_from_current_context: function() {
    throw 'emscripten_fiber_init_from_current_context is not implemented for EMTERPRETIFY_ASYNC';
  },
  emscripten_fiber_swap: function() {
    throw 'emscripten_fiber_swap is not implemented for EMTERPRETIFY_ASYNC';
  },

#else // EMTERPRETIFY_ASYNC

#if WASM_BACKEND && ASYNCIFY
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
#endif // EMTERPRETIFY_ASYNC
#endif // ASYNCIFY
});

if (EMTERPRETIFY_ASYNC && !EMTERPRETIFY) {
  error('You must enable EMTERPRETIFY to use EMTERPRETIFY_ASYNC');
}
if (WASM_BACKEND && ASYNCIFY) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$Asyncify');
}

