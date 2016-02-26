// Capture the output of this into a variable, if you want
(function(fb, parentModule) {
  var Module = {};
  var args = [];
  Module.arguments = [];
  Module.print = parentModule.print;
  Module.printErr = parentModule.printErr;

  Module.cleanups = [];

  var gb = 0;
  // Each module has its own stack
  // XXX without dynamic allocations, we try to allocate these on the parent's stack. But if our stacks are the same size, that can't work! Need to specify smaller sizes for side module stacks.
  var STACKTOP = getMemory(TOTAL_STACK);
  assert(STACKTOP % 8 == 0);
  var STACK_MAX = STACKTOP + TOTAL_STACK;
  Module.cleanups.push(function() {
    parentModule['_free'](STACKTOP); // XXX ensure exported
    parentModule['_free'](gb);
  });

  {{BODY}}

  // {{MODULE_ADDITIONS}}

  return Module;
});

