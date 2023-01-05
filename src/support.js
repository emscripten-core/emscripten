/**
 * @license
 * Copyright 2017 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var STACK_ALIGN = {{{ STACK_ALIGN }}};
var POINTER_SIZE = {{{ MEMORY64 ? 8 : 4 }}};

{{{ getNativeTypeSize }}}

#include "runtime_debug.js"

#if RETAIN_COMPILER_SETTINGS
var compilerSettings = {{{ JSON.stringify(makeRetainedCompilerSettings()) }}} ;

function getCompilerSetting(name) {
  if (!(name in compilerSettings)) return 'invalid compiler setting: ' + name;
  return compilerSettings[name];
}
#endif // RETAIN_COMPILER_SETTINGS
