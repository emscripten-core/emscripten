/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Takes a pair of return values, stashes one in tempRet0 and returns the other.
// Should probably be renamed to `makeReturn64` but keeping this old name in
// case external JS library code uses this name.
function makeStructuralReturn(values) {
  assert(values.length == 2);
  return 'setTempRet0(' + values[1] + '); return ' + asmCoercion(values[0], 'i32');
}
