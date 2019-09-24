/*
 * Copyright 2010 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * C++ exception handling support.
 */

var LibraryExceptions = {
  __exception_last: '0',
  __exception_caught: ' []',
  __exception_infos: '{}',

  __exception_deAdjust__deps: ['__exception_infos'],
  __exception_deAdjust: function(adjusted) {
    if (!adjusted || ___exception_infos[adjusted]) return adjusted;
    for (var key in ___exception_infos) {
      var ptr = +key; // the iteration key is a string, and if we throw this, it must be an integer as that is what we look for
      var adj = ___exception_infos[ptr].adjusted;
      var len = adj.length;
      for (var i = 0; i < len; i++) {
        if (adj[i] === adjusted) {
#if EXCEPTION_DEBUG
          err('de-adjusted exception ptr ' + adjusted + ' to ' + ptr);
#endif
          return ptr;
        }
      }
    }
#if EXCEPTION_DEBUG
    err('no de-adjustment for unknown exception ptr ' + adjusted);
#endif
    return adjusted;
  },

  __exception_addRef__deps: ['__exception_infos'],
  __exception_addRef: function(ptr) {
#if EXCEPTION_DEBUG
    err('addref ' + ptr);
#endif
    if (!ptr) return;
    var info = ___exception_infos[ptr];
    info.refcount++;
  },

  __exception_decRef__deps: ['__exception_infos', '__cxa_free_exception'
#if EXCEPTION_DEBUG
      , '__exception_last', '__exception_caught'
#endif
    ],
  __exception_decRef: function(ptr) {
#if EXCEPTION_DEBUG
    err('decref ' + ptr);
#endif
    if (!ptr) return;
    var info = ___exception_infos[ptr];
#if ASSERTIONS
    assert(info.refcount > 0);
#endif
    info.refcount--;
    // A rethrown exception can reach refcount 0; it must not be discarded
    // Its next handler will clear the rethrown flag and addRef it, prior to
    // final decRef and destruction here
    if (info.refcount === 0 && !info.rethrown) {
      if (info.destructor) {
#if WASM_BACKEND == 0
        Module['dynCall_vi'](info.destructor, ptr);
#else
        // In Wasm, destructors return 'this' as in ARM
        Module['dynCall_ii'](info.destructor, ptr);
#endif
      }
      delete ___exception_infos[ptr];
      ___cxa_free_exception(ptr);
#if EXCEPTION_DEBUG
      err('decref freeing exception ' + [ptr, ___exception_last, 'stack', ___exception_caught]);
#endif
    }
  },

  __exception_clearRef__deps: ['__exception_infos'],
  __exception_clearRef: function(ptr) {
    if (!ptr) return;
    var info = ___exception_infos[ptr];
    info.refcount = 0;
  },

  // Exceptions
  __cxa_allocate_exception: function(size) {
    return _malloc(size);
  },

  __cxa_free_exception: function(ptr) {
#if ABORTING_MALLOC || ASSERTIONS
    try {
#endif
      return _free(ptr);
#if ABORTING_MALLOC || ASSERTIONS
    } catch(e) {
#if ASSERTIONS
      err('exception during cxa_free_exception: ' + e);
#endif
    }
#endif
  },

  __cxa_increment_exception_refcount__deps: ['__exception_addRef', '__exception_deAdjust'],
  __cxa_increment_exception_refcount: function(ptr) {
    ___exception_addRef(___exception_deAdjust(ptr));
  },

  __cxa_decrement_exception_refcount__deps: ['__exception_decRef', '__exception_deAdjust'],
  __cxa_decrement_exception_refcount: function(ptr) {
    ___exception_decRef(___exception_deAdjust(ptr));
  },

  // Here, we throw an exception after recording a couple of values that we need to remember
  // We also remember that it was the last exception thrown as we need to know that later.
  __cxa_throw__sig: 'viii',
  __cxa_throw__deps: ['__exception_infos', '__exception_last', '_ZSt18uncaught_exceptionv'],
  __cxa_throw: function(ptr, type, destructor) {
#if EXCEPTION_DEBUG
    err('Compiled code throwing an exception, ' + [ptr,type,destructor]);
#endif
    ___exception_infos[ptr] = {
      ptr: ptr,
      adjusted: [ptr],
      type: type,
      destructor: destructor,
      refcount: 0,
      caught: false,
      rethrown: false
    };
    ___exception_last = ptr;
    if (!("uncaught_exception" in __ZSt18uncaught_exceptionv)) {
      __ZSt18uncaught_exceptionv.uncaught_exceptions = 1;
    } else {
      __ZSt18uncaught_exceptionv.uncaught_exceptions++;
    }
    {{{ makeThrow('ptr') }}}
  },

  // This exception will be caught twice, but while begin_catch runs twice,
  // we early-exit from end_catch when the exception has been rethrown, so
  // pop that here from the caught exceptions.
  __cxa_rethrow__deps: ['__exception_caught', '__exception_deAdjust', '__exception_infos', '__exception_last'],
  __cxa_rethrow: function() {
    var ptr = ___exception_caught.pop();
    ptr = ___exception_deAdjust(ptr);
    if (!___exception_infos[ptr].rethrown) {
      // Only pop if the corresponding push was through rethrow_primary_exception
      ___exception_caught.push(ptr);
      ___exception_infos[ptr].rethrown = true;
    }
#if EXCEPTION_DEBUG
    err('Compiled code RE-throwing an exception, popped ' + [ptr, ___exception_last, 'stack', ___exception_caught]);
#endif
    ___exception_last = ptr;
    {{{ makeThrow('ptr') }}}
  },

  llvm_eh_exception__deps: ['__exception_last'],
  llvm_eh_exception: function() {
    return ___exception_last;
  },

  llvm_eh_selector__jsargs: true,
  llvm_eh_selector__deps: ['__exception_last'],
  llvm_eh_selector: function(unused_exception_value, personality/*, varargs*/) {
    var type = ___exception_last;
    for (var i = 2; i < arguments.length; i++) {
      if (arguments[i] ==  type) return type;
    }
    return 0;
  },

  llvm_eh_typeid_for: function(type) {
    return type;
  },

  __cxa_begin_catch__deps: ['__exception_infos', '__exception_caught', '__exception_addRef', '__exception_deAdjust', '_ZSt18uncaught_exceptionv'],
  __cxa_begin_catch: function(ptr) {
    var info = ___exception_infos[ptr];
    if (info && !info.caught) {
      info.caught = true;
      __ZSt18uncaught_exceptionv.uncaught_exceptions--;
    }
    if (info) info.rethrown = false;
    ___exception_caught.push(ptr);
#if EXCEPTION_DEBUG
    err('cxa_begin_catch ' + [ptr, 'stack', ___exception_caught]);
#endif
    ___exception_addRef(___exception_deAdjust(ptr));
    return ptr;
  },

  // We're done with a catch. Now, we can run the destructor if there is one
  // and free the exception. Note that if the dynCall on the destructor fails
  // due to calling apply on undefined, that means that the destructor is
  // an invalid index into the FUNCTION_TABLE, so something has gone wrong.
  __cxa_end_catch__deps: ['__exception_caught', '__exception_last', '__exception_decRef', '__exception_deAdjust', 'setThrew'],
  __cxa_end_catch: function() {
    // Clear state flag.
    _setThrew(0);
    // Call destructor if one is registered then clear it.
    var ptr = ___exception_caught.pop();
#if EXCEPTION_DEBUG
    err('cxa_end_catch popped ' + [ptr, ___exception_last, 'stack', ___exception_caught]);
#endif
    if (ptr) {
      ___exception_decRef(___exception_deAdjust(ptr));
      ___exception_last = 0; // XXX in decRef?
    }
  },
  __cxa_get_exception_ptr: function(ptr) {
#if EXCEPTION_DEBUG
    err('cxa_get_exception_ptr ' + ptr);
#endif
    // TODO: use info.adjusted?
    return ptr;
  },

  _ZSt18uncaught_exceptionv: function() { // std::uncaught_exception()
    return __ZSt18uncaught_exceptionv.uncaught_exceptions > 0;
  },

  __cxa_uncaught_exceptions__deps: ['_ZSt18uncaught_exceptionv'],
  __cxa_uncaught_exceptions: function() {
    return __ZSt18uncaught_exceptionv.uncaught_exceptions;
  },

  __cxa_call_unexpected: function(exception) {
    err('Unexpected exception thrown, this is not properly supported - aborting');
#if !MINIMAL_RUNTIME
    ABORT = true;
#endif
    throw exception;
  },

  __cxa_current_primary_exception__deps: ['__exception_caught', '__exception_addRef', '__exception_deAdjust'],
  __cxa_current_primary_exception: function() {
    var ret = ___exception_caught[___exception_caught.length-1] || 0;
    if (ret) ___exception_addRef(___exception_deAdjust(ret));
    return ret;
  },

  __cxa_rethrow_primary_exception__deps: ['__exception_deAdjust', '__exception_caught', '__exception_infos', '__cxa_rethrow'],
  __cxa_rethrow_primary_exception: function(ptr) {
    if (!ptr) return;
    ptr = ___exception_deAdjust(ptr);
    ___exception_caught.push(ptr);
    ___exception_infos[ptr].rethrown = true;
    ___cxa_rethrow();
  },

  // Finds a suitable catch clause for when an exception is thrown.
  // In normal compilers, this functionality is handled by the C++
  // 'personality' routine. This is passed a fairly complex structure
  // relating to the context of the exception and makes judgements
  // about how to handle it. Some of it is about matching a suitable
  // catch clause, and some of it is about unwinding. We already handle
  // unwinding using 'if' blocks around each function, so the remaining
  // functionality boils down to picking a suitable 'catch' block.
  // We'll do that here, instead, to keep things simpler.

  __cxa_find_matching_catch__deps: ['__exception_last', '__exception_infos', '__resumeException'],
  __cxa_find_matching_catch: function() {
    var thrown = ___exception_last;
    if (!thrown) {
      // just pass through the null ptr
      {{{ makeStructuralReturn([0, 0]) }}};
    }
    var info = ___exception_infos[thrown];
    var throwntype = info.type;
    if (!throwntype) {
      // just pass through the thrown ptr
      {{{ makeStructuralReturn(['thrown', 0]) }}};
    }
    var typeArray = Array.prototype.slice.call(arguments);

    var pointer = {{{ exportedAsmFunc('___cxa_is_pointer_type') }}}(throwntype);
    // can_catch receives a **, add indirection
#if EXCEPTION_DEBUG
    out("can_catch on " + [thrown]);
#endif
#if DISABLE_EXCEPTION_CATCHING == 1
    var buffer = 0;
#else
    var buffer = {{{ makeStaticAlloc(4) }}};
#endif
    {{{ makeSetValue('buffer', '0', 'thrown', '*') }}};
    thrown = buffer;
    // The different catch blocks are denoted by different types.
    // Due to inheritance, those types may not precisely match the
    // type of the thrown object. Find one which matches, and
    // return the type of the catch block which should be called.
    for (var i = 0; i < typeArray.length; i++) {
      if (typeArray[i] && {{{ exportedAsmFunc('___cxa_can_catch') }}}(typeArray[i], throwntype, thrown)) {
        thrown = {{{ makeGetValue('thrown', '0', '*') }}}; // undo indirection
        info.adjusted.push(thrown);
#if EXCEPTION_DEBUG
        out("  can_catch found " + [thrown, typeArray[i]]);
#endif
        {{{ makeStructuralReturn(['thrown', 'typeArray[i]']) }}};
      }
    }
    // Shouldn't happen unless we have bogus data in typeArray
    // or encounter a type for which emscripten doesn't have suitable
    // typeinfo defined. Best-efforts match just in case.
    thrown = {{{ makeGetValue('thrown', '0', '*') }}}; // undo indirection
    {{{ makeStructuralReturn(['thrown', 'throwntype']) }}};
  },

  __resumeException__deps: [function() { '__exception_last', Functions.libraryFunctions['___resumeException'] = 1 }], // will be called directly from compiled code
  __resumeException: function(ptr) {
#if EXCEPTION_DEBUG
    out("Resuming exception " + [ptr, ___exception_last]);
#endif
    if (!___exception_last) { ___exception_last = ptr; }
    {{{ makeThrow('ptr') }}}
  },
};

// In LLVM, exceptions generate a set of functions of form __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.
// where the number specifies the number of arguments. In Emscripten, route all these to a single function '__cxa_find_matching_catch'
// that variadically processes all of these functions using JS 'arguments' object.
var maxExceptionArgs = 10; // arbitrary upper limit
for (var n = 0; n < maxExceptionArgs; ++n) {
  LibraryExceptions['__cxa_find_matching_catch_' + n] = '__cxa_find_matching_catch';
  LibraryExceptions['__cxa_find_matching_catch_' + n + '__sig'] = new Array(n + 2).join('i');
}

mergeInto(LibraryManager.library, LibraryExceptions);
