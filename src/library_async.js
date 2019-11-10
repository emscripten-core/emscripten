// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Async support
//
// Two experiments in async support: ASYNCIFY, and EMTERPRETIFY_ASYNC

mergeInto(LibraryManager.library, {
#if ASYNCIFY
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
    assert((((___async_cur_frame + 8)|0) == (ctx|0))|0);
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

  emscripten_wget__deps: ['emscripten_async_resume', '$PATH', '$Browser'],
  emscripten_wget: function(url, file) {
    var _url = Pointer_stringify(url);
    var _file = Pointer_stringify(file);
    _file = PATH.resolve(FS.cwd(), _file);
    Module['setAsync']();
    Module['noExitRuntime'] = true;
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

#else // ASYNCIFY

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
      Module['noExitRuntime'] = true;
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

  emscripten_wget__deps: ['$EmterpreterAsync', '$PATH', '$FS', '$Browser'],
  emscripten_wget: function(url, file) {
    EmterpreterAsync.handle(function(resume) {
      var _url = Pointer_stringify(url);
      var _file = Pointer_stringify(file);
      _file = PATH.resolve(FS.cwd(), _file);
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
      Browser.asyncLoad(Pointer_stringify(url), function(byteArray) {
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

#else // EMTERPRETIFY_ASYNC

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
#endif // EMTERPRETIFY_ASYNC
#endif // ASYNCIFY
});

if (EMTERPRETIFY_ASYNC && !EMTERPRETIFY) {
  error('You must enable EMTERPRETIFY to use EMTERPRETIFY_ASYNC');
}
