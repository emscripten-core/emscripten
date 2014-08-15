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
  __async_retval: 'allocate(2, "i32", ALLOC_STATIC)', // store the return value for async functions
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
      dynCall_vi(callback, (___async_cur_frame + 8)|0);
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

  emscripten_sleep__deps: ['emscripten_async_resume'],
  emscripten_sleep: function(ms) {
    asm.setAsync(); // tell the scheduler that we have a callback on hold
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
    stackRestore(___async_cur_frame);
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

