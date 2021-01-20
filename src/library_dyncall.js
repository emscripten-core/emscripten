mergeInto(LibraryManager.library, {
  {{{ (function() { global.wbind = function() { return SHRINK_LEVEL == 0 ? 'wbind' : 'wasmTable.get'; }; return null; })(); }}}
  {{{ (function() { global.getDynCaller = function(sig) { return MINIMAL_RUNTIME ? `dynCalls[${sig}]` : `Module["dynCall_"+${sig}]`; }; return null; })(); }}}

#if SHRINK_LEVEL == 0
  // A mirror copy of contents of wasmTable in JS side, to avoid relatively
  // slow wasmTable.get() call. Only used when not compiling with -Os or -Oz.
  _wasmTableMirror: [],

  $wbind__deps: ['_wasmTableMirror'],
  $wbind: function(funcPtr) {
    var func = __wasmTableMirror[funcPtr];
    if (!func) {
      if (funcPtr >= __wasmTableMirror.length) __wasmTableMirror.length = funcPtr + 1;
      __wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
    }
    return func;
  },

#if WASM_DYNCALLS
  $dynCall__deps: ['$wbind'],
  $bindDynCall__deps: ['$wbind'],
#endif
  $wbindArray__deps: ['$wbind'],
#else
  $wbind: function(funcPtr) {
    // In -Os and -Oz builds, do not implement a JS side wasm table mirror for small
    // code size, but directly access wasmTable, which is a bit slower.
    return wasmTable.get(funcPtr);
  },
#endif

  // A helper that binds a wasm function into a form that can be called by passing all
  // the parameters in an array, e.g. wbindArray(func)([param1, param2, ..., paramN]).
  $wbindArray: function(funcPtr) {
    var func = {{{ wbind() }}}(funcPtr);
    return func.length
      ? function(args) { return func.apply(null, args); }
      : function() { return func(); }
  },

#if WASM_DYNCALLS
  // A helper that returns a function that can be used to invoke function pointers, i.e.
  // getDynCaller('vi')(funcPtr, myInt);
  $getDynCaller: function(sig, funcPtr) {
    return {{{ getDynCaller('sig') }}};
  },

  $bindDynCall: function(sig, funcPtr) {
    // For int64 signatures, use the dynCall_sig dispatch mechanism.
    if (sig.includes('j')) return function(args) {
      return {{{ getDynCaller('sig') }}}.apply(null, [funcPtr].concat(args));
    }
    // For non-int64 signatures, invoke via the wasm table.
    var func = {{{ wbind() }}}(funcPtr);
    return func.length
      ? function(args) { return func.apply(null, args); }
      : function() { return func(); }
  },

#if SHRINK_LEVEL
  $dynCall__deps: ['$bindDynCall'],
#endif
  $dynCall: function(sig, funcPtr, args) {
#if SHRINK_LEVEL
    return bindDynCall(sig, funcPtr)(args);
#else
    // For int64 signatures, use the dynCall_sig dispatch mechanism.
    if (sig.includes('j')) {
      return {{{ getDynCaller('sig') }}}.apply(null, [funcPtr].concat(args));
    }

    // For non-int64 signatures, invoke via the wasm table.
    return {{{ wbind() }}}(funcPtr).apply(null, args);
#endif
  },
#endif
});
