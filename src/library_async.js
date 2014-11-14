mergeInto(LibraryManager.library, {
#if ASYNCIFY
/*
 * When an async function exits, the normal JavaScript stack is unwound, so to
 * restore it later we save the execution context as a linked list of async
 * contexts, one for each function currently executing.  The async contexts
 * themselves are allocated on the stack before each (possibly) async call.
 * Each context is an instance of the following:
 *   struct async_ctx {
 *       struct async_ctx* previous;
 *       void* sp; // saved copy of STACKTOP
 *       int len; // total size of this async_ctx structure (the final field
 *                // has variable length)
 *       uint8_t invoked; // true if call is an invoke (inside try/catch)
 *       char padding[3];
 *       void (*async_cb)(struct async_ctx* ctx);
 *       char javascriptStackVariables[];
 *   };
 *
 * At the call site for an async function, the async_ctx is allocated on the
 * stack just before the call, then it makes the call. If it returns
 * synchronously, then the async_ctx is popped and isn't used. Otherwise, the
 * call site fills in the async_cb member with a pointer to a function that will
 * resume execution at the current line number, and saves the JavaScript local
 * variables.
 *
 * The linked list of async_ctx structures is maintained by
 * emscripten_alloc_async_context() and emscripten_async_resume(); the
 * __async_cur_frame pointer always points to the async_ctx at the top of the
 * stack.
 *
 * All the functions at the bottom of the stack are asynchronous (if any), then
 * come non-asynchronous functions (if any): all callers of async functions are
 * async.  The stack typed array looks like this:
 *
 * ---------------------  <-- bottom of stack
 * <structures and stack used by main()>
 * struct async_ctx; // main()'s async_ctx; "previous" is 0
 * ---------------------
 *     ...
 * <next function's stack frame> // All the async calls are at the bottom, and
 * struct async_ctx              // each one pushes an async_ctx before calling
 *                               // the next async function.
 *     ...
 * <last async function's stack frame>
 * struct async_ctx      <-- __async_cur_frame
 *     ...
 * ---------------------
 *     ...
 * <synchronous function stack frame> // All the synchronous functions increment
 *                                    // STACKTOP for their own local variables,
 *                                    // and don't touch the async state.
 *     ...
 * --------------------- <-- STACKTOP
 */

  __async: 0, // whether a truly async function has been called
  __async_unwind: 1, // whether to unwind the async stack frame
  __async_retval: 'allocate(2, "i32", ALLOC_STATIC)', // store the return value for async functions
  __async_cur_frame: 0, // address to the current frame, which stores previous frame, stack pointer and async context

  emscripten_async_resume__deps: ['__async', '__async_unwind', '__async_retval', '__async_cur_frame', 'emscripten_async_abort_with_exception', 'emscripten_async_exit'],
  emscripten_async_resume__sig: 'v',
  emscripten_async_resume__asm: true,
  emscripten_async_resume: function() {
    var activeException = 0, catchesExceptions = 0, currentFrame = 0;
    ___async = 0;
    ___async_unwind = 1;
    while (1) {
      if (!___async_cur_frame) {
        // print ugly integer in console, same as if an exception is thrown out of main()
        if (activeException) _emscripten_async_abort_with_exception();
        _emscripten_async_exit({{{ makeGetValueAsm('___async_retval', 0, 'i32') }}});
        return;
      }
      catchesExceptions = {{{ makeGetValueAsm('___async_cur_frame', 12, 'i8') }}};
      // When an async function throws an exception, it can't just bubble up as
      // normal (because there isn't a native JavaScript call stack, we're
      // manually reconstructing it here).  We keep rewinding the stack until we
      // find a function that can handle the exception.
      if (!!catchesExceptions | !activeException) {
        __THREW__ = activeException|0;
        currentFrame = ___async_cur_frame|0;
        invoke_vi({{{ makeGetValueAsm('___async_cur_frame', 16, 'i32') }}},
                  (___async_cur_frame + 16)|0);
        activeException = __THREW__|0; __THREW__ = 0;

        if (___async) return; // that was an async call

#if ASSERTIONS
        // If there isn't an exception, any async contexts created during the
        // callback function should have unwound themselves.  However, if
        // the callback was exited due to an exception, we may need to update
        // async_cur_frame.
        assert(activeException | (___async_cur_frame == currentFrame));
#endif
        if (activeException) {
          ___async_cur_frame = currentFrame|0;
        }

#if ASSERTIONS
        // If there's an exception we always unwind.
        assert(!activeException | !!___async_unwind);
#endif
        if (!___async_unwind) {
          // keep the async stack
          ___async_unwind = 1;
          continue;
        }
      }
      // unwind normal stack frame
      stackRestore({{{ makeGetValueAsm('___async_cur_frame', 4, 'i32') }}});
      // pop the last async stack frame
      ___async_cur_frame = {{{ makeGetValueAsm('___async_cur_frame', 0, 'i32') }}};
    }
  },

  emscripten_async_exit: function(status) {
    try {
      exit(status);
    } catch (e) {
      if (e instanceof ExitStatus) {
        // exit() throws this once it's done to make sure execution
        // has been stopped completely
        return;
      }
      throw e;
    }
  },

  emscripten_async_abort_with_exception: function() {
    {{{ makeThrow('EXCEPTIONS.last') }}};
  },

  emscripten_sleep__deps: ['emscripten_async_resume'],
  emscripten_sleep: function(ms) {
    asm.setAsync(); // tell the scheduler that we have a callback on hold
    Browser.safeSetTimeout(_emscripten_async_resume, ms);
  },

  emscripten_alloc_async_context__deps: ['__async_cur_frame'],
  emscripten_alloc_async_context__sig: 'iii',
  emscripten_alloc_async_context__asm: true,
  emscripten_alloc_async_context: function(len, invoked, sp) {
    len = len|0;
    invoked = invoked|0;
    sp = sp|0;
    var actual_len = 0, new_frame = 0;
    // len is the size of the call-specific data stored in the context; we also
    // need to store prev_frame, stack pointer, the actual length, and invoked.
    // Normally stackAlloc() must be used with caution since STACKTOP is reset
    // when exceptions are caught, so unless the "st" variable is also
    // incremented the allocation is in danger of being wiped. In this case
    // though we're OK, because the context is either free'd immediately after
    // the call, or else execution continues in an async_cb function which has
    // "st = STACKTOP" at the start.
    new_frame = stackAlloc((len + 16)|0)|0;
    actual_len = ((stackSave()|0) - (new_frame|0))|0;
    // save invoked
    {{{ makeSetValueAsm('new_frame', 12, 'invoked', 'i8') }}};
    // save len
    {{{ makeSetValueAsm('new_frame', 8, 'actual_len', 'i32') }}};
    // save sp
    {{{ makeSetValueAsm('new_frame', 4, 'sp', 'i32') }}};
    // link the frame with previous one
    {{{ makeSetValueAsm('new_frame', 0, '___async_cur_frame', 'i32') }}};
    ___async_cur_frame = new_frame;
    return (new_frame + 16)|0;
  },

  emscripten_realloc_async_context__deps: ['__async_cur_frame'],
  emscripten_realloc_async_context__sig: 'ii',
  emscripten_realloc_async_context__asm: true,
  emscripten_realloc_async_context: function(len, invoked) {
    len = len|0;
    invoked = invoked|0;
    var new_frame = 0, actual_len = 0;

#if ASSERTIONS
    var old_len = 0;
    old_len = {{{ makeGetValueAsm('___async_cur_frame', 8, 'i32') }}};

    // Assert that we haven't wiped out the async ctx from the stack - this is
    // the assertion we will keep if the one below is fixed.
    //assert(stackSave() >= ((___async_cur_frame + old_len)|0));

    // XXX Assert that the async structure is still on the top of the stack,
    // otherwise we can't realloc! If an async_cb uses alloca() then calls
    // another async function, we're in trouble; asyncify doesn't currently
    // support this.
    assert(((stackSave()|0) == ((___async_cur_frame + old_len)|0))|0,
           'alloca not supported inside async callback');
#endif

    stackRestore(___async_cur_frame);
    new_frame = stackAlloc((len + 16)|0)|0;
    actual_len = ((stackSave()|0) - (new_frame|0))|0;
    // save the new length and 'invoked' flag
    {{{ makeSetValueAsm('new_frame', 12, 'invoked', 'i8') }}};
    {{{ makeSetValueAsm('new_frame', 8, 'actual_len', 'i32') }}};
    return (new_frame + 16)|0;
  },

  emscripten_free_async_context__deps: ['__async_cur_frame'],
  emscripten_free_async_context__sig: 'vi',
  emscripten_free_async_context__asm: true,
  emscripten_free_async_context: function(ctx, st) {
    //  this function is called when a possibly async function turned out to be sync
    //  just undo a recent emscripten_alloc_async_context
    ctx = ctx|0;
    st = st|0;
    var ctx_len = 0;

    // When emscripten_free_async_context() is called after an invoke(), there
    // may be some async contexts on the stack which weren't cleaned up, but
    // otherwise we expect the ctx passed in to be the current one.
#if ASSERTIONS
    assert(!!__THREW__ | (((___async_cur_frame + 16)|0) == (ctx|0))|0);
#endif
    ___async_cur_frame = (ctx - 16)|0;

    ctx_len = {{{ makeGetValueAsm('___async_cur_frame', 8, 'i32') }}};

#if ASSERTIONS
    // The only thing that can happen in between alloc_async_context and
    // free_async_context is the call itself, and there are two cases:
    if (st >= 0) {
      // If the call returned via invoke then the landingpad is about to restore
      // STACKTOP to st (if the exception's type matches a landingpad!).
      assert(st == ___async_cur_frame);
    } else {
      // In the case of a normal (non-invoke) call, st = -1 and the function
      // prologue should have restored the STACKTOP already.
      assert((stackSave()|0) == ((___async_cur_frame + ctx_len)|0));
    }
#endif
    // The above checks make sure it's safe to reset the stack globally here:
    stackRestore(___async_cur_frame);

    ___async_cur_frame = {{{ makeGetValueAsm('___async_cur_frame', 0, 'i32') }}};
  },

  emscripten_check_async: true,
  emscripten_do_not_unwind: true,
  emscripten_do_not_unwind_async: true,

  emscripten_get_async_return_value_addr__deps: ['__async_retval'],
  emscripten_get_async_return_value_addr: true,

/*
 * Layout of a coroutine structure
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
  emscripten_coroutine_create__sig: 'iii',
  emscripten_coroutine_create__asm: true,
  emscripten_coroutine_create__deps: ['malloc'],
  emscripten_coroutine_create: function(f, arg, stack_size) {
    f = f|0;
    arg = arg|0;
    stack_size = stack_size|0;
    var coroutine = 0;

    if ((stack_size|0) <= 0) stack_size = 4096;

    coroutine = _malloc(stack_size)|0;
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
    var coroutine_not_finished = 0;
    // switch context
    {{{ makeSetValueAsm('coroutine', 0, '___async_cur_frame', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 4, 'stackSave()|0', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 8, 'STACK_MAX', 'i32') }}};
    ___async_cur_frame = {{{ makeGetValueAsm('coroutine', 12, 'i32') }}};
    stackRestore({{{ makeGetValueAsm('coroutine', 16, 'i32') }}});
    STACK_MAX = coroutine + 32 + {{{ makeGetValueAsm('coroutine', 20, 'i32') }}} | 0;

    if (!___async_cur_frame) {
      // first run
      dynCall_vi(
        {{{ makeGetValueAsm('coroutine', 24, 'i32') }}},
        {{{ makeGetValueAsm('coroutine', 28, 'i32') }}}
      );
    } else {
      _emscripten_async_resume();
    }

    // switch context
    {{{ makeSetValueAsm('coroutine', 12, '___async_cur_frame', 'i32') }}};
    {{{ makeSetValueAsm('coroutine', 16, 'stackSave()|0', 'i32') }}};
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
  }
#else // ASYNCIFY
  emscripten_sleep: function() {
    throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_sleep';
  },
  emscripten_coroutine_create: function() {
    throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_coroutine_create';
  },
  emscripten_coroutine_next: function() {
    throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_coroutine_next';
  },
  emscripten_yield: function() {
    throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_yield';
  }
#endif
});

