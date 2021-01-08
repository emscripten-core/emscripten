/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryExceptions = {
  $uncaughtExceptionCount: '0',
  $exceptionLast: '0',
  $exceptionCaught: ' []',

  // Static fields for ExceptionInfo class.
  $ExceptionInfoAttrs: {
    // ExceptionInfo native structure layout.
    DESTRUCTOR_OFFSET: 0,
    REFCOUNT_OFFSET: Runtime.POINTER_SIZE,
    TYPE_OFFSET: Runtime.POINTER_SIZE + 4,
    CAUGHT_OFFSET: Runtime.POINTER_SIZE + 8,
    RETHROWN_OFFSET: Runtime.POINTER_SIZE + 9,

    // Total structure size with padding, should be multiple of allocation alignment.
    SIZE: alignMemory(Runtime.POINTER_SIZE + 10)
  },

  $ExceptionInfo__deps: ['$ExceptionInfoAttrs'],
  // This class is the exception metadata which is prepended to each thrown object (in WASM memory).
  // It is allocated in one block among with a thrown object in __cxa_allocate_exception and freed
  // in ___cxa_free_exception. It roughly corresponds to __cxa_exception structure in libcxxabi. The
  // class itself is just a native pointer wrapper, and contains all the necessary accessors for the
  // fields in the native structure.
  // TODO: Unfortunately this approach still cannot be considered thread-safe because single
  // exception object can be simultaneously thrown in several threads and its state (except
  // reference counter) is not protected from that. Also protection is not enough, separate state
  // should be allocated. libcxxabi has concept of dependent exception which is used for that
  // purpose, it references the primary exception.
  //
  // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
  $ExceptionInfo: function(excPtr) {
    this.excPtr = excPtr;
    this.ptr = excPtr - ExceptionInfoAttrs.SIZE;

    this.set_type = function(type) {
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.TYPE_OFFSET', 'type', '*') }}};
    };

    this.get_type = function() {
      return {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.TYPE_OFFSET', '*') }}};
    };

    this.set_destructor = function(destructor) {
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.DESTRUCTOR_OFFSET', 'destructor', '*') }}};
    };

    this.get_destructor = function() {
      return {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.DESTRUCTOR_OFFSET', '*') }}};
    };

    this.set_refcount = function(refcount) {
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.REFCOUNT_OFFSET', 'refcount', 'i32') }}};
    };

    this.set_caught = function (caught) {
      caught = caught ? 1 : 0;
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.CAUGHT_OFFSET', 'caught', 'i8') }}};
    };

    this.get_caught = function () {
      return {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.CAUGHT_OFFSET', 'i8') }}} != 0;
    };

    this.set_rethrown = function (rethrown) {
      rethrown = rethrown ? 1 : 0;
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.RETHROWN_OFFSET', 'rethrown', 'i8') }}};
    };

    this.get_rethrown = function () {
      return {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.RETHROWN_OFFSET', 'i8') }}} != 0;
    };

    // Initialize native structure fields. Should be called once after allocated.
    this.init = function(type, destructor) {
      this.set_type(type);
      this.set_destructor(destructor);
      this.set_refcount(0);
      this.set_caught(false);
      this.set_rethrown(false);
    }

    this.add_ref = function() {
#if USE_PTHREADS
      Atomics.add(HEAP32, (this.ptr + ExceptionInfoAttrs.REFCOUNT_OFFSET) >> 2, 1);
#else
      var value = {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.REFCOUNT_OFFSET', 'i32') }}};
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.REFCOUNT_OFFSET', 'value + 1', 'i32') }}};
#endif
    };

    // Returns true if last reference released.
    this.release_ref = function() {
#if USE_PTHREADS
      var prev = Atomics.sub(HEAP32, (this.ptr + ExceptionInfoAttrs.REFCOUNT_OFFSET) >> 2, 1);
#else
      var prev = {{{ makeGetValue('this.ptr', 'ExceptionInfoAttrs.REFCOUNT_OFFSET', 'i32') }}};
      {{{ makeSetValue('this.ptr', 'ExceptionInfoAttrs.REFCOUNT_OFFSET', 'prev - 1', 'i32') }}};
#endif
#if ASSERTIONS
      assert(prev > 0);
#endif
      return prev === 1;
    };
  },

  $CatchInfo__deps: ['$ExceptionInfo'],
  // This native structure is returned from __cxa_find_matching_catch, and serves as catching
  // context, i.e. stores information required to proceed with a specific selected catch. It stores
  // base and adjusted pointers of a thrown object. It is allocated dynamically and should be freed
  // when it is done with a specific catch (i.e. either in __cxa_end_catch when caught or in
  // __resumeException when no catch clause matched). The class itself is just a native pointer
  // wrapper, and contains all the necessary accessors for the fields in the native structure.
  // ptr - Native structure pointer to wrap, the structure is allocated when not specified.
  $CatchInfo: function(ptr) {

    this.free = function() {
      _free(this.ptr);
      this.ptr = 0;
    };

    this.set_base_ptr = function(basePtr) {
      {{{ makeSetValue('this.ptr', '0', 'basePtr', '*') }}};
    };

    this.get_base_ptr = function() {
      return {{{ makeGetValue('this.ptr', '0', '*') }}};
    };

    this.set_adjusted_ptr = function(adjustedPtr) {
      var ptrSize = {{{ Runtime.POINTER_SIZE }}};
      {{{ makeSetValue('this.ptr', 'ptrSize', 'adjustedPtr', '*') }}};
    };

    this.get_adjusted_ptr = function() {
      var ptrSize = {{{ Runtime.POINTER_SIZE }}};
      return {{{ makeGetValue('this.ptr', 'ptrSize', '*') }}};
    };

    // Get pointer which is expected to be received by catch clause in C++ code. It may be adjusted
    // when the pointer is casted to some of the exception object base classes (e.g. when virtual
    // inheritance is used). When a pointer is thrown this method should return the thrown pointer
    // itself.
    this.get_exception_ptr = function() {
      // Work around a fastcomp bug, this code is still included for some reason in a build without
      // exceptions support.
      var isPointer = {{{ exportedAsmFunc('___cxa_is_pointer_type') }}}(
        this.get_exception_info().get_type());
      if (isPointer) {
        return {{{ makeGetValue('this.get_base_ptr()', '0', '*') }}};
      }
      var adjusted = this.get_adjusted_ptr();
      if (adjusted !== 0) return adjusted;
      return this.get_base_ptr();
    };

    this.get_exception_info = function() {
      return new ExceptionInfo(this.get_base_ptr());
    };

    if (ptr === undefined) {
      this.ptr = _malloc({{{ Runtime.POINTER_SIZE * 2 }}});
      this.set_adjusted_ptr(0);
    } else {
      this.ptr = ptr;
    }
  },

  $exception_addRef: function (info) {
#if EXCEPTION_DEBUG
    err('addref ' + info.excPtr);
#endif
    info.add_ref();
  },

  $exception_decRef__deps: ['__cxa_free_exception'
#if EXCEPTION_DEBUG
    , '$exceptionLast', '$exceptionCaught'
#endif
  ],
  $exception_decRef: function(info) {
#if EXCEPTION_DEBUG
    err('decref ' + info.excPtr);
#endif
    // A rethrown exception can reach refcount 0; it must not be discarded
    // Its next handler will clear the rethrown flag and addRef it, prior to
    // final decRef and destruction here
    if (info.release_ref() && !info.get_rethrown()) {
      var destructor = info.get_destructor();
      if (destructor) {
        // In Wasm, destructors return 'this' as in ARM
        {{{ makeDynCall('ii', 'destructor') }}}(info.excPtr);
      }
      ___cxa_free_exception(info.excPtr);
#if EXCEPTION_DEBUG
      err('decref freeing exception ' + [info.excPtr, exceptionLast, 'stack', exceptionCaught]);
#endif
    }
  },

  // Exceptions
  __cxa_allocate_exception__deps: ['$ExceptionInfoAttrs'],
  __cxa_allocate_exception: function(size) {
    // Thrown object is prepended by exception metadata block
    return _malloc(size + ExceptionInfoAttrs.SIZE) + ExceptionInfoAttrs.SIZE;
  },

  __cxa_free_exception__deps: ['$ExceptionInfo'],
  __cxa_free_exception: function(ptr) {
#if ABORTING_MALLOC || ASSERTIONS
    try {
#endif
      return _free(new ExceptionInfo(ptr).ptr);
#if ABORTING_MALLOC || ASSERTIONS
    } catch(e) {
#if ASSERTIONS
      err('exception during cxa_free_exception: ' + e);
#endif
    }
#endif
  },

  __cxa_increment_exception_refcount__deps: ['$exception_addRef', '$ExceptionInfo'],
  __cxa_increment_exception_refcount: function(ptr) {
    if (!ptr) return;
    exception_addRef(new ExceptionInfo(ptr));
  },

  __cxa_decrement_exception_refcount__deps: ['$exception_decRef', '$ExceptionInfo'],
  __cxa_decrement_exception_refcount: function(ptr) {
    if (!ptr) return;
    exception_decRef(new ExceptionInfo(ptr));
  },

  // Here, we throw an exception after recording a couple of values that we need to remember
  // We also remember that it was the last exception thrown as we need to know that later.
  __cxa_throw__sig: 'viii',
  __cxa_throw__deps: ['$ExceptionInfo', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_throw: function(ptr, type, destructor) {
#if EXCEPTION_DEBUG
    err('Compiled code throwing an exception, ' + [ptr,type,destructor]);
#endif
    var info = new ExceptionInfo(ptr);
    // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
    info.init(type, destructor);
    exceptionLast = ptr;
    uncaughtExceptionCount++;
    {{{ makeThrow('ptr') }}}
  },

  // This exception will be caught twice, but while begin_catch runs twice,
  // we early-exit from end_catch when the exception has been rethrown, so
  // pop that here from the caught exceptions.
  __cxa_rethrow__deps: ['$exceptionCaught', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_rethrow__sig: 'v',
  __cxa_rethrow: function() {
    var catchInfo = exceptionCaught.pop();
    if (!catchInfo) {
      abort('no exception to throw');
    }
    var info = catchInfo.get_exception_info();
    var ptr = catchInfo.get_base_ptr();
    if (!info.get_rethrown()) {
      // Only pop if the corresponding push was through rethrow_primary_exception
      exceptionCaught.push(catchInfo);
      info.set_rethrown(true);
      info.set_caught(false);
      uncaughtExceptionCount++;
    } else {
      catchInfo.free();
    }
#if EXCEPTION_DEBUG
    err('Compiled code RE-throwing an exception, popped ' +
      [ptr, exceptionLast, 'stack', exceptionCaught]);
#endif
    exceptionLast = ptr;
    {{{ makeThrow('ptr') }}}
  },

  llvm_eh_typeid_for: function(type) {
    return type;
  },

  __cxa_begin_catch__deps: ['$CatchInfo', '$exceptionCaught', '$exception_addRef',
                            '$uncaughtExceptionCount'],
  __cxa_begin_catch: function(ptr) {
    var catchInfo = new CatchInfo(ptr);
    var info = catchInfo.get_exception_info();
    if (!info.get_caught()) {
      info.set_caught(true);
      uncaughtExceptionCount--;
    }
    info.set_rethrown(false);
    exceptionCaught.push(catchInfo);
#if EXCEPTION_DEBUG
    err('cxa_begin_catch ' + [ptr, 'stack', exceptionCaught]);
#endif
    exception_addRef(info);
    return catchInfo.get_exception_ptr();
  },

  // We're done with a catch. Now, we can run the destructor if there is one
  // and free the exception. Note that if the dynCall on the destructor fails
  // due to calling apply on undefined, that means that the destructor is
  // an invalid index into the FUNCTION_TABLE, so something has gone wrong.
  __cxa_end_catch__deps: ['$exceptionCaught', '$exceptionLast', '$exception_decRef',
                          '$CatchInfo'],
  __cxa_end_catch__sig: 'v',
  __cxa_end_catch: function() {
    // Clear state flag.
    _setThrew(0);
#if ASSERTIONS
    assert(exceptionCaught.length > 0);
#endif
    // Call destructor if one is registered then clear it.
    var catchInfo = exceptionCaught.pop();

#if EXCEPTION_DEBUG
    err('cxa_end_catch popped ' + [catchInfo, exceptionLast, 'stack', exceptionCaught]);
#endif
    exception_decRef(catchInfo.get_exception_info());
    catchInfo.free();
    exceptionLast = 0; // XXX in decRef?
  },

  __cxa_get_exception_ptr__deps: ['$CatchInfo'],
  __cxa_get_exception_ptr: function(ptr) {
#if EXCEPTION_DEBUG
    err('cxa_get_exception_ptr ' + ptr);
#endif
    return new CatchInfo(ptr).get_exception_ptr();
  },

  __cxa_uncaught_exceptions__deps: ['$uncaughtExceptionCount'],
  __cxa_uncaught_exceptions: function() {
    return uncaughtExceptionCount;
  },

  __cxa_call_unexpected: function(exception) {
    err('Unexpected exception thrown, this is not properly supported - aborting');
#if !MINIMAL_RUNTIME
    ABORT = true;
#endif
    throw exception;
  },

  __cxa_current_primary_exception__deps: ['$exceptionCaught', '$exception_addRef', '$CatchInfo'],
  __cxa_current_primary_exception: function() {
    if (!exceptionCaught.length) {
      return 0;
    }
    var catchInfo = exceptionCaught[exceptionCaught.length - 1];
    exception_addRef(catchInfo.get_exception_info());
    return catchInfo.get_base_ptr();
  },

  __cxa_rethrow_primary_exception__deps: ['$CatchInfo', '$exceptionCaught', '__cxa_rethrow'],
  __cxa_rethrow_primary_exception: function(ptr) {
    if (!ptr) return;
    var catchInfo = new CatchInfo();
    catchInfo.set_base_ptr(ptr);
    var info = catchInfo.get_exception_info();
    exceptionCaught.push(catchInfo);
    info.set_rethrown(true);
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
  __cxa_find_matching_catch__deps: ['$exceptionLast', '$ExceptionInfo', '$CatchInfo', '__resumeException'],
  __cxa_find_matching_catch: function() {
    var thrown = exceptionLast;
    if (!thrown) {
      // just pass through the null ptr
      {{{ makeStructuralReturn([0, 0]) }}};
    }
    var info = new ExceptionInfo(thrown);
    var thrownType = info.get_type();
    var catchInfo = new CatchInfo();
    catchInfo.set_base_ptr(thrown);
    if (!thrownType) {
      // just pass through the thrown ptr
      {{{ makeStructuralReturn(['catchInfo.ptr', 0]) }}};
    }
    var typeArray = Array.prototype.slice.call(arguments);

    // can_catch receives a **, add indirection
#if EXCEPTION_DEBUG
    out("can_catch on " + [thrown]);
#endif
    var stackTop = stackSave();
    var exceptionThrowBuf = stackAlloc(4);
    {{{ makeSetValue('exceptionThrowBuf', '0', 'thrown', '*') }}};
    // The different catch blocks are denoted by different types.
    // Due to inheritance, those types may not precisely match the
    // type of the thrown object. Find one which matches, and
    // return the type of the catch block which should be called.
    for (var i = 0; i < typeArray.length; i++) {
      var caughtType = typeArray[i];
      if (caughtType === 0 || caughtType === thrownType) {
        // Catch all clause matched or exactly the same type is caught
        break;
      }
      if ({{{ exportedAsmFunc('___cxa_can_catch') }}}(caughtType, thrownType, exceptionThrowBuf)) {
        var adjusted = {{{ makeGetValue('exceptionThrowBuf', '0', '*') }}};
        if (thrown !== adjusted) {
          catchInfo.set_adjusted_ptr(adjusted);
        }
#if EXCEPTION_DEBUG
        out("  can_catch found " + [adjusted, caughtType]);
#endif
        {{{ makeStructuralReturn(['catchInfo.ptr', 'caughtType']) }}};
      }
    }
    stackRestore(stackTop);
    {{{ makeStructuralReturn(['catchInfo.ptr', 'thrownType']) }}};
  },

  __resumeException__deps: [function() { '$exceptionLast', '$CatchInfo',
                                        Functions.libraryFunctions['___resumeException'] = 1 }], // will be called directly from compiled code
  __resumeException: function(catchInfoPtr) {
    var catchInfo = new CatchInfo(catchInfoPtr);
    var ptr = catchInfo.get_base_ptr();
#if EXCEPTION_DEBUG
    out("Resuming exception " + [ptr, exceptionLast]);
#endif
    if (!exceptionLast) { exceptionLast = ptr; }
    catchInfo.free();
    {{{ makeThrow('ptr') }}}
  },
};

// In LLVM, exceptions generate a set of functions of form __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.
// where the number specifies the number of arguments. In Emscripten, route all these to a single function '__cxa_find_matching_catch'
// that variadically processes all of these functions using JS 'arguments' object.
addCxaCatch = function(n) {
  LibraryManager.library['__cxa_find_matching_catch_' + n] = LibraryExceptions['__cxa_find_matching_catch'];
  LibraryManager.library['__cxa_find_matching_catch_' + n + '__sig'] = new Array(n + 2).join('i');
  LibraryManager.library['__cxa_find_matching_catch_' + n + '__deps'] = LibraryExceptions['__cxa_find_matching_catch__deps'];
};

mergeInto(LibraryManager.library, LibraryExceptions);
