/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryExceptions = {
#if !WASM_EXCEPTIONS
  $uncaughtExceptionCount: '0',
  $exceptionLast: '0',
  $exceptionCaught: ' []',

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
#if EXCEPTION_DEBUG
  $ExceptionInfo__deps: ['$ptrToString'],
#endif
  $ExceptionInfo: class {
    // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
    constructor(excPtr) {
      this.excPtr = excPtr;
      this.ptr = excPtr - {{{ C_STRUCTS.__cxa_exception.__size__ }}};
    }

    set_type(type) {
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionType, 'type', '*') }}};
    }

    get_type() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionType, '*') }}};
    }

    set_destructor(destructor) {
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionDestructor, 'destructor', '*') }}};
    }

    get_destructor() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.exceptionDestructor, '*') }}};
    }

    set_caught(caught) {
      caught = caught ? 1 : 0;
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.caught, 'caught', 'i8') }}};
    }

    get_caught() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.caught, 'i8') }}} != 0;
    }

    set_rethrown(rethrown) {
      rethrown = rethrown ? 1 : 0;
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.rethrown, 'rethrown', 'i8') }}};
    }

    get_rethrown() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.rethrown, 'i8') }}} != 0;
    }

    // Initialize native structure fields. Should be called once after allocated.
    init(type, destructor) {
#if EXCEPTION_DEBUG
      dbg('ExceptionInfo init: ' + [type, destructor]);
#endif
      this.set_adjusted_ptr(0);
      this.set_type(type);
      this.set_destructor(destructor);
    }

    set_adjusted_ptr(adjustedPtr) {
      {{{ makeSetValue('this.ptr', C_STRUCTS.__cxa_exception.adjustedPtr, 'adjustedPtr', '*') }}};
    }

    get_adjusted_ptr() {
      return {{{ makeGetValue('this.ptr', C_STRUCTS.__cxa_exception.adjustedPtr, '*') }}};
    }
  },

  // Here, we throw an exception after recording a couple of values that we need to remember
  // We also remember that it was the last exception thrown as we need to know that later.
  __cxa_throw__deps: ['$ExceptionInfo', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_throw: (ptr, type, destructor) => {
#if EXCEPTION_DEBUG
    dbg('__cxa_throw: ' + [ptrToString(ptr), type, ptrToString(destructor)]);
#endif
    var info = new ExceptionInfo(ptr);
    // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
    info.init(type, destructor);
    {{{ storeException('exceptionLast', 'ptr') }}}
    uncaughtExceptionCount++;
    {{{ makeThrow('exceptionLast') }}}
  },

  // This exception will be caught twice, but while begin_catch runs twice,
  // we early-exit from end_catch when the exception has been rethrown, so
  // pop that here from the caught exceptions.
  __cxa_rethrow__deps: ['$exceptionCaught', '$exceptionLast', '$uncaughtExceptionCount'],
  __cxa_rethrow: () => {
    var info = exceptionCaught.pop();
    if (!info) {
      abort('no exception to throw');
    }
    var ptr = info.excPtr;
    if (!info.get_rethrown()) {
      // Only pop if the corresponding push was through rethrow_primary_exception
      exceptionCaught.push(info);
      info.set_rethrown(true);
      info.set_caught(false);
      uncaughtExceptionCount++;
    }
#if EXCEPTION_DEBUG
    dbg('__cxa_rethrow, popped ' +
      [ptrToString(ptr), exceptionLast, 'stack', exceptionCaught]);
#endif
    {{{ storeException('exceptionLast', 'ptr') }}}
    {{{ makeThrow('exceptionLast') }}}
  },

  llvm_eh_typeid_for: (type) => type,

  __cxa_begin_catch__deps: ['$exceptionCaught', '__cxa_increment_exception_refcount',
                            '__cxa_get_exception_ptr',
                            '$uncaughtExceptionCount'],
  __cxa_begin_catch: (ptr) => {
    var info = new ExceptionInfo(ptr);
    if (!info.get_caught()) {
      info.set_caught(true);
      uncaughtExceptionCount--;
    }
    info.set_rethrown(false);
    exceptionCaught.push(info);
#if EXCEPTION_DEBUG
    dbg('__cxa_begin_catch ' + [ptrToString(ptr), 'stack', exceptionCaught]);
#endif
    ___cxa_increment_exception_refcount(ptr);
    return ___cxa_get_exception_ptr(ptr);
  },

  // We're done with a catch. Now, we can run the destructor if there is one
  // and free the exception. Note that if the dynCall on the destructor fails
  // due to calling apply on undefined, that means that the destructor is
  // an invalid index into the FUNCTION_TABLE, so something has gone wrong.
  __cxa_end_catch__deps: ['$exceptionCaught', '$exceptionLast', '__cxa_decrement_exception_refcount', 'setThrew'],
  __cxa_end_catch: () => {
    // Clear state flag.
    _setThrew(0, 0);
#if ASSERTIONS
    assert(exceptionCaught.length > 0);
#endif
    // Call destructor if one is registered then clear it.
    var info = exceptionCaught.pop();

#if EXCEPTION_DEBUG
    dbg('__cxa_end_catch popped ' + [info, exceptionLast, 'stack', exceptionCaught]);
#endif
    ___cxa_decrement_exception_refcount(info.excPtr);
    exceptionLast = 0; // XXX in decRef?
  },

  __cxa_uncaught_exceptions__deps: ['$uncaughtExceptionCount'],
  __cxa_uncaught_exceptions: () => uncaughtExceptionCount,

  __cxa_call_unexpected: (exception) => abort('Unexpected exception thrown, this is not properly supported - aborting'),

  __cxa_current_primary_exception__deps: ['$exceptionCaught', '__cxa_increment_exception_refcount'],
  __cxa_current_primary_exception: () => {
    if (!exceptionCaught.length) {
      return 0;
    }
    var info = exceptionCaught[exceptionCaught.length - 1];
    ___cxa_increment_exception_refcount(info.excPtr);
    return info.excPtr;
  },

  __cxa_rethrow_primary_exception__deps: ['$ExceptionInfo', '$exceptionCaught', '__cxa_rethrow'],
  __cxa_rethrow_primary_exception: (ptr) => {
    if (!ptr) return;
    var info = new ExceptionInfo(ptr);
    exceptionCaught.push(info);
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
  $findMatchingCatch__deps: ['$exceptionLast', '$ExceptionInfo', '__resumeException', '__cxa_can_catch', '$setTempRet0'],
  $findMatchingCatch: (args) => {
    var thrown =
#if EXCEPTION_STACK_TRACES
      exceptionLast?.excPtr;
#else
      exceptionLast;
#endif
    if (!thrown) {
      // just pass through the null ptr
      setTempRet0(0);
      return 0;
    }
    var info = new ExceptionInfo(thrown);
    info.set_adjusted_ptr(thrown);
    var thrownType = info.get_type();
    if (!thrownType) {
      // just pass through the thrown ptr
      setTempRet0(0);
      return thrown;
    }

    // can_catch receives a **, add indirection
#if EXCEPTION_DEBUG
    dbg("findMatchingCatch on " + ptrToString(thrown));
#endif
    // The different catch blocks are denoted by different types.
    // Due to inheritance, those types may not precisely match the
    // type of the thrown object. Find one which matches, and
    // return the type of the catch block which should be called.
    for (var caughtType of args) {
      if (caughtType === 0 || caughtType === thrownType) {
        // Catch all clause matched or exactly the same type is caught
        break;
      }
      var adjusted_ptr_addr = info.ptr + {{{ C_STRUCTS.__cxa_exception.adjustedPtr }}};
      if (___cxa_can_catch(caughtType, thrownType, adjusted_ptr_addr)) {
#if EXCEPTION_DEBUG
        dbg("  findMatchingCatch found " + [ptrToString(info.get_adjusted_ptr()), caughtType]);
#endif
        setTempRet0(caughtType);
        return thrown;
      }
    }
    setTempRet0(thrownType);
    return thrown;
  },

  __resumeException__deps: ['$exceptionLast'],
  __resumeException: (ptr) => {
#if EXCEPTION_DEBUG
    dbg("__resumeException " + [ptrToString(ptr), exceptionLast]);
#endif
    if (!exceptionLast) {
      {{{ storeException('exceptionLast', 'ptr') }}}
    }
    {{{ makeThrow('exceptionLast') }}}
  },

#endif
#if WASM_EXCEPTIONS || !DISABLE_EXCEPTION_CATCHING
  $getExceptionMessageCommon__deps: ['__get_exception_message', 'free', '$stackSave', '$stackRestore', '$stackAlloc'],
  $getExceptionMessageCommon: (ptr) => {
    var sp = stackSave();
    var type_addr_addr = stackAlloc({{{ POINTER_SIZE }}});
    var message_addr_addr = stackAlloc({{{ POINTER_SIZE }}});
    ___get_exception_message(ptr, type_addr_addr, message_addr_addr);
    var type_addr = {{{ makeGetValue('type_addr_addr', 0, '*') }}};
    var message_addr = {{{ makeGetValue('message_addr_addr', 0, '*') }}};
    var type = UTF8ToString(type_addr);
    _free(type_addr);
    var message;
    if (message_addr) {
      message = UTF8ToString(message_addr);
      _free(message_addr);
    }
    stackRestore(sp);
    return [type, message];
  },
#endif
#if WASM_EXCEPTIONS
  $getCppExceptionTag: () =>
    // In static linking, tags are defined within the wasm module and are
    // exported, whereas in dynamic linking, tags are defined in library.js in
    // JS code and wasm modules import them.
#if RELOCATABLE
    ___cpp_exception // defined in library.js
#else
    wasmExports['__cpp_exception']
#endif
  ,

#if EXCEPTION_STACK_TRACES
  // Throw a WebAssembly.Exception object with the C++ tag with a stack trace
  // embedded. WebAssembly.Exception is a JS object representing a Wasm
  // exception, provided by Wasm JS API:
  // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/Exception
  // In release builds, this function is not needed and the native
  // _Unwind_RaiseException in libunwind is used instead.
  __throw_exception_with_stack_trace__deps: ['$getCppExceptionTag', '$getExceptionMessage'],
  __throw_exception_with_stack_trace: (ex) => {
    var e = new WebAssembly.Exception(getCppExceptionTag(), [ex], {traceStack: true});
    e.message = getExceptionMessage(e);
    throw e;
  },
#endif

  // Given an WebAssembly.Exception object, returns the actual user-thrown
  // C++ object address in the Wasm memory.
  $getCppExceptionThrownObjectFromWebAssemblyException__deps: ['$getCppExceptionTag', '__thrown_object_from_unwind_exception'],
  $getCppExceptionThrownObjectFromWebAssemblyException: (ex) => {
    // In Wasm EH, the value extracted from WebAssembly.Exception is a pointer
    // to the unwind header. Convert it to the actual thrown value.
    var unwind_header = ex.getArg(getCppExceptionTag(), 0);
    return ___thrown_object_from_unwind_exception(unwind_header);
  },

  $incrementExceptionRefcount__deps: ['__cxa_increment_exception_refcount', '$getCppExceptionThrownObjectFromWebAssemblyException'],
  $incrementExceptionRefcount: (ex) => {
    var ptr = getCppExceptionThrownObjectFromWebAssemblyException(ex);
    ___cxa_increment_exception_refcount(ptr);
  },

  $decrementExceptionRefcount__deps: ['__cxa_decrement_exception_refcount', '$getCppExceptionThrownObjectFromWebAssemblyException'],
  $decrementExceptionRefcount: (ex) => {
    var ptr = getCppExceptionThrownObjectFromWebAssemblyException(ex);
    ___cxa_decrement_exception_refcount(ptr);
  },

  $getExceptionMessage__deps: ['$getCppExceptionThrownObjectFromWebAssemblyException', '$getExceptionMessageCommon'],
  $getExceptionMessage: (ex) => {
    var ptr = getCppExceptionThrownObjectFromWebAssemblyException(ex);
    return getExceptionMessageCommon(ptr);
  },

#elif !DISABLE_EXCEPTION_CATCHING
  $incrementExceptionRefcount__deps: ['__cxa_increment_exception_refcount'],
  $incrementExceptionRefcount: (ptr) => ___cxa_increment_exception_refcount(ptr),

  $decrementExceptionRefcount__deps: ['__cxa_decrement_exception_refcount'],
  $decrementExceptionRefcount: (ptr) => ___cxa_decrement_exception_refcount(ptr),

  $getExceptionMessage__deps: ['$getExceptionMessageCommon'],
  $getExceptionMessage: (ptr) => getExceptionMessageCommon(ptr),

#endif
};

#if !WASM_EXCEPTIONS
// In LLVM, exceptions generate a set of functions of form
// __cxa_find_matching_catch_2(), __cxa_find_matching_catch_3(), etc.  where the
// number specifies the number of arguments.  In Emscripten, route all these to
// a single function '__cxa_find_matching_catch' that variadically processes all
// of these functions using JS 'arguments' object.
addCxaCatch = (n) => {
  const args = [];
  // Confusingly, the actual number of asrgument is n - 2. According to the llvm
  // code in WebAssemblyLowerEmscriptenEHSjLj.cpp:
  // This is because a landingpad instruction contains two more arguments, a
  // personality function and a cleanup bit, and __cxa_find_matching_catch_N
  // functions are named after the number of arguments in the original landingpad
  // instruction.
  let sig = 'p';
  for (let i = 0; i < n - 2; i++) {
    args.push(`arg${i}`);
    sig += 'p';
  }
  const argString = args.join(',');
  LibraryManager.library[`__cxa_find_matching_catch_${n}__sig`] = sig;
  LibraryManager.library[`__cxa_find_matching_catch_${n}__deps`] = ['$findMatchingCatch'];
  LibraryManager.library[`__cxa_find_matching_catch_${n}`] = eval(`(${args}) => findMatchingCatch([${argString}])`);
};

// Add the first 2-5 catch handlers preemptively.  Others get added on demand in
// jsifier.  This is done here primarily so that these symbols end up with the
// correct deps in the stub library that we pass to wasm-ld.
// Note: __cxa_find_matching_catch_N function uses N = NumClauses + 2 so
// __cxa_find_matching_catch_2 is the first such function with zero clauses.
// See WebAssemblyLowerEmscriptenEHSjLj.cpp.
for (let i = 2; i < 5; i++) {
  addCxaCatch(i)
}
#endif

addToLibrary(LibraryExceptions);
