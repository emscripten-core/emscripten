/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryStackTrace = {
  $jsStackTrace: function() {
    return new Error().stack.toString();
  },

  $stackTrace__deps: ['$jsStackTrace'],
  $stackTrace: function() {
    var js = jsStackTrace();
    if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
    return js;
  }
}

addToLibrary(LibraryStackTrace);
