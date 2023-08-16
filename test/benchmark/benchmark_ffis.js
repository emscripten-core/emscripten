// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

addToLibrary({
  $foreignCounter: 0,
  foreignFunctionThatTakesThreeParameters__deps: ['$foreignCounter'],
  foreignFunctionThatTakesThreeParameters: function(a, b, c) {
    foreignCounter += a + b + c;
    return foreignCounter;
  }
});
