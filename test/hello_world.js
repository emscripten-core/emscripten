/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Simple test JS program used by `test/jsrun.py` to verify that a JS engine
 * is working.
 */

'use strict';

if (typeof print === 'undefined') {
  global.print = console.log
}

print('hello, world!');
