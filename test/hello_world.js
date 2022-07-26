/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

'use strict';

var ENVIRONMENT_IS_NODE = typeof process === 'object';
if (ENVIRONMENT_IS_NODE) {
  global.print = function(x) {
    process['stdout'].write(x + '\n');
  }
}

print('hello, world!');
