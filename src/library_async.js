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

mergeInto(LibraryManager.library, {
#if ASYNCIFY
  __async: 0, // whether a truly async function has been called
  __async_unwind: 1, // whether to unwind the async stack frame
  __async_retval: 'allocate(2, "i32", ALLOC_STATIC)', // store the return value for async functions
  __async_cur_frame: 0, // address to the current frame, which stores previous frame, stack pointer and async context

  emscripten_async_resume__deps: ['__async', '__async_unwind', '__async_cur_frame'],
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
    assert(___async_cur_frame + 8 == ctx);
#endif
    stackRestore(___async_cur_frame);
    ___async_cur_frame = {{{ makeGetValueAsm('___async_cur_frame', 0, 'i32') }}};
  },

  emscripten_check_async: true,
  emscripten_do_not_unwind: true,
  emscripten_do_not_unwind_async: true,

  emscripten_get_async_return_value_addr__deps: ['__async_retval'],
  emscripten_get_async_return_value_addr: true
#else // ASYNCIFY
  emscripten_sleep: function() {
    throw 'Please compile your program with -s ASYNCIFY=1 in order to use asynchronous operations like emscripten_sleep';
  }
#endif
});

