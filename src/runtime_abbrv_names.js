/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Create short names for builtin methods that we call.  This allows closure
 * compiler to minify them and means the full names will never appear in the
 * generated code more than once.
 */
var objAssign = Object.assign;
var arraySlice = Array.prototype.slice;
