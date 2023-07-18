/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryStackTrace = {
#if DEMANGLE_SUPPORT
  $demangle__deps: ['$withStackSave', '__cxa_demangle', 'free', '$stringToUTF8OnStack'],
#endif
  $demangle: function(func) {
#if DEMANGLE_SUPPORT
    // If demangle has failed before, stop demangling any further function names
    // This avoids an infinite recursion with malloc()->abort()->stackTrace()->demangle()->malloc()->...
    demangle.recursionGuard = (demangle.recursionGuard|0)+1;
    if (demangle.recursionGuard > 1) return func;
    return withStackSave(function() {
      try {
        var s = func;
        if (s.startsWith('__Z'))
          s = s.substr(1);
        var buf = stringToUTF8OnStack(s);
        var status = stackAlloc(4);
        var ret = ___cxa_demangle(buf, 0, 0, status);
        if ({{{ makeGetValue('status', '0', 'i32') }}} === 0 && ret) {
          return UTF8ToString(ret);
        }
        // otherwise, libcxxabi failed
      } catch(e) {
      } finally {
        _free(ret);
        if (demangle.recursionGuard < 2) --demangle.recursionGuard;
      }
      // failure when using libcxxabi, don't demangle
      return func;
    });
#else // DEMANGLE_SUPPORT
#if ASSERTIONS
    warnOnce('warning: build with -sDEMANGLE_SUPPORT to link in libcxxabi demangling');
#endif // ASSERTIONS
    return func;
#endif // DEMANGLE_SUPPORT
  },

  $demangleAll__deps: ['$demangle'],
  $demangleAll: function(text) {
    var regex =
      /\b_Z[\w\d_]+/g;
    return text.replace(regex,
      function(x) {
        var y = demangle(x);
        return x === y ? x : (y + ' [' + x + ']');
      });
  },

  $jsStackTrace: function() {
    var error = new Error();
    if (!error.stack) {
      // IE10+ special cases: It does have callstack info, but it is only
      // populated if an Error object is thrown, so try that as a special-case.
      try {
        throw new Error();
      } catch(e) {
        error = e;
      }
      if (!error.stack) {
        return '(no stack trace available)';
      }
    }
    return error.stack.toString();
  },

  $stackTrace__deps: ['$jsStackTrace', '$demangleAll'],
  $stackTrace: function() {
    var js = jsStackTrace();
    if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
    return demangleAll(js);
  }
}

mergeInto(LibraryManager.library, LibraryStackTrace);
