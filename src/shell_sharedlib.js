/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Capture the output of this into a variable, if you want
(function(fb, parentModule) {
  var Module = {};
  var args = [];
  Module.arguments = [];
  Module.cleanups = [];

  var gb = 0;
  // Each module has its own stack
  var STACKTOP = getMemory(parentModule['TOTAL_STACK']);
  assert(STACKTOP % 8 == 0);
  var STACK_MAX = STACKTOP + parentModule['TOTAL_STACK'];
  Module.cleanups.push(function() {
    parentModule['_free'](STACKTOP); // XXX ensure exported, and that it was actually malloc'ed and not static memory FIXME
    parentModule['_free'](gb);
  });

  {{BODY}}

  // {{MODULE_ADDITIONS}}

  return Module;
});

