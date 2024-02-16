/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryStackTrace = {
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

  $stackTrace__deps: ['$jsStackTrace'],
  $stackTrace: function() {
    var js = jsStackTrace();
    if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
    return js;
  }
}

addToLibrary(LibraryStackTrace);
