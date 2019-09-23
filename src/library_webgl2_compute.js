// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryWebGL2Compute = {
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
var webgl2computeFuncs = [[0, ''],
 [1, ''],
 [2, ''],
 [3, ''],
 [4, ''],
 [5, ''],
 [6, ''],
 [7, ''],
 [8, ''],
 [9, ''],
 [10, '']];

#if GL_MAX_FEATURE_LEVEL >= 30

createGLPassthroughFunctions(LibraryWebGL2Compute, webgl2computeFuncs);

recordGLProcAddressGet(LibraryWebGL2Compute);

mergeInto(LibraryManager.library, LibraryWebGL2Compute);

#endif

#if USE_WEBGL2_COMPUTE
assert(GL_MAX_FEATURE_LEVEL == 30, 'cannot resolve conflicting USE_WEBGL* settings');
#endif
