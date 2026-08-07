/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if WASM_EXCEPTIONS
#error "Internal error! WASM_EXCEPTIONS should not be enabled when including libunwind.js."
#endif

var LibraryUnwind = {
  $uncaughtExceptionCount: '0',
#if !DISABLE_EXCEPTION_CATCHING
  $exceptionLast: null,
#endif

  _Unwind_Backtrace__deps: ['$getCallstack'],
  _Unwind_Backtrace: (func, arg) => {
    var trace = getCallstack();
    var parts = trace.split('\n');
    for (var i = 0; i < parts.length; i++) {
      var ret = {{{ makeDynCall('iii', 'func') }}}(0, arg);
      if (ret) return;
    }
  },

  _Unwind_GetIPInfo: (context, ipBefore) => abort('Unwind_GetIPInfo'),

  _Unwind_FindEnclosingFunction: (ip) => 0, // we cannot succeed

  _Unwind_RaiseException__deps: ['$uncaughtExceptionCount',
#if !DISABLE_EXCEPTION_CATCHING
    '$exceptionLast',
#endif
  ],
  _Unwind_RaiseException: (ex) => {
#if !DISABLE_EXCEPTION_CATCHING
    exceptionLast = ex;
    uncaughtExceptionCount++;
#endif
    {{{ makeThrow('ex') }}}
  },

#if !DISABLE_EXCEPTION_CATCHING
  _Unwind_Resume__deps: ['$exceptionLast'],
#endif
  _Unwind_Resume: (ex) => {
#if !DISABLE_EXCEPTION_CATCHING
    exceptionLast = ex;
#endif
    {{{ makeThrow('ex') }}}
  },

  _Unwind_DeleteException: (ex) => err('TODO: Unwind_DeleteException'),
};

addToLibrary(LibraryUnwind);
